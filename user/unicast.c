#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// define the format of a msg
#define MAX_NUM_RECEIVERS 10
#define MAX_MSG_SIZE 256
struct msg_t
{
  int flags[MAX_NUM_RECEIVERS];
  char content[MAX_MSG_SIZE];
};

void panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

// create a new process
int fork1(void)
{
  int pid;
  pid = fork();
  if (pid == -1)
    panic("fork");
  return pid;
}

// create a pipe
void pipe1(int fd[2])
{
  int rc = pipe(fd);
  if (rc < 0)
  {
    panic("Fail to create a pipe.");
  }
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    // unitcast 5 3 Hello!
    panic("Usage: broadcast <num_of_children> <id_of_receiver> <msg_to_broadcast>");
  }

  // Placing values into variables
  int numReceiver = atoi(argv[1]);
  int id_of_receiver = atoi(argv[2]);

  // create a pair of pipes as communication channels
  int channelToReceivers[2], channelFromReceivers[2];
  pipe(channelToReceivers);
  pipe(channelFromReceivers);

  for (int i = 0; i < numReceiver; i++)
  {

    // create child process as receiver
    int retFork = fork1();
    if (retFork == 0)
    {

      /*following is the code for child process i*/

      // announce start of the child process
      int myId = i;
      printf("Child %d: start!\n", myId);

      // read pipe to get the message
      struct msg_t msg;
      read(channelToReceivers[0],
           (void *)&msg, sizeof(struct msg_t));
      printf("Child %d: get msg (%s) to Child %d\n",
             myId, msg.content, id_of_receiver);

      // check if all receivers have already received this message
      msg.flags[i] = 0;
      int sum = 0;
      for (int j = 0; j < MAX_NUM_RECEIVERS; j++)
        sum += msg.flags[j];
      // while iterating thorugh all the children, check to see if it matches the desired ID
      if (id_of_receiver == myId)
      {
        printf("Child %d: the msg is for me.\n", myId);
        printf("Child %d acknowledges to Parent: received!\n", myId);
        write(channelFromReceivers[1], "received!", 9);
      }
      else if (sum == 0)
      {
        // if all receivers have received the message, send ack to parent
        printf("Child %d: the msg is not for me. and I am the last one. Womp Womp\n", myId);
        write(channelFromReceivers[1], "completed!", 10);
      }
      else
      {
        // otherwise, write the message back to the pipe for the receivers
        // yet to receive the mssage
        printf("Child %d: the msg is not for me.\n", myId);
        printf("Child %d: write the msg back to pipe.\n", myId);
        write(channelToReceivers[1], &msg, sizeof(msg));
      }

      // end of the child process
      exit(0);
    }
    else
    {
      printf("Parent: creates child process with id: %d\n", i);
    }
    sleep(1);
  }

  /*following is the parent's code*/

  // to broadcast message
  struct msg_t msg;
  for (int i = 0; i < numReceiver; i++)
    msg.flags[i] = 1;
  strcpy(msg.content, argv[3]);
  write(channelToReceivers[1], &msg, sizeof(struct msg_t));
  printf("Parent sends to Child %d: %s\n", id_of_receiver, msg.content);

  // to receive acknowledgement
  char recvBuf[sizeof(struct msg_t)];
  read(channelFromReceivers[0], &recvBuf, sizeof(struct msg_t));
  printf("Parent receives: %s\n", recvBuf);

  // end of parent process
  exit(0);
}
// unicast 5 5 Hello