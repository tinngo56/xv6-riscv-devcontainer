#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getppid(void)
{
  /*TODO:
  (1) call myproc() to get the caller’s struct proc
  (2) follow the field “parent” in the struct proc to find the parent’s struct
  proc
  (3) in the parent’s struct proc, find the pid and return it
  */
 // gets current process
  struct proc *p = myproc();
  // return current process's parent's pid
  return p->parent->pid;
}
extern struct proc proc[NPROC]; // declare array proc which is defined in proc.c already
uint64
sys_ps(void)
{
  // define the ps_struct for each process and ps[NPROC] for all processes
  struct ps_struct
  {
    int pid;
    int ppid;
    char state[10];
    char name[16];
  } ps[NPROC];
  int numProc = 0; // variable keeping track of the number of processes in the system

  /*To do: From array proc, find the processes that are still in the system (i.e.,
  their states are not NUNUSED. For each of the process, retrieve the
  information and put into a ps_struct defined above*/
  for (int i = 0; i < NPROC; i++)
  {
    // initializing current process
    struct proc *p = &proc[i];
    // check if the process slot is used
    if (p->state != UNUSED)
    {
      // Set the process ID
      ps[numProc].pid = p->pid;     
      // Set the parent process ID; -1 if no parent                      
      ps[numProc].ppid = p->parent ? p->parent->pid : 0; 

      // Set the process state using strncpy
      switch (p->state)
      {
      case UNUSED:
        strncpy(ps[numProc].state, "UNUSED", sizeof(ps[numProc].state) - 1);
        break;
      case SLEEPING:
        strncpy(ps[numProc].state, "SLEEPING", sizeof(ps[numProc].state) - 1);
        break;
      case RUNNING:
        strncpy(ps[numProc].state, "RUNNING", sizeof(ps[numProc].state) - 1);
        break;
      case ZOMBIE:
        strncpy(ps[numProc].state, "ZOMBIE", sizeof(ps[numProc].state) - 1);
        break;
      default:
        strncpy(ps[numProc].state, "UNKNOWN", sizeof(ps[numProc].state) - 1);
      }
      // ensure null termination
      ps[numProc].state[sizeof(ps[numProc].state) - 1] = '\0'; 

      strncpy(ps[numProc].name, p->name, sizeof(ps[numProc].name) - 1);
      // ensure null termination
      ps[numProc].name[sizeof(ps[numProc].name) - 1] = '\0'; 

      // increment the number of processed entries
      numProc++; 
    }
  }

  // save the address of the user space argment to arg_addr
  uint64 arg_addr;
  argaddr(0, &arg_addr);
  // copy array ps to the saved address
  if (copyout(myproc()->pagetable,
              arg_addr,
              (char *)ps,
              numProc * sizeof(struct ps_struct)) < 0)
    return -1;
  // return numProc as well
  return numProc;
}

uint64
sys_getschedhistory(void)
{
  // define the struct for reporting scheduling history
  struct sched_history
  {
    int runCount;
    int systemcallCount;
    int interruptCount;
    int preemptCount;
    int trapCount;
    int sleepCount;
  } my_history;
  // To do: retrieve the current process’ information to fill my_history
  struct proc *p = myproc();
  // filling my_history information from current process
  my_history.runCount = p->runCount;
  my_history.systemcallCount = p->systemcallCount;
  my_history.interruptCount = p->interruptCount;
  my_history.preemptCount = p->preemptCount;
  my_history.trapCount = p->trapCount;
  my_history.sleepCount = p->sleepCount;

  // save the address of the user space argment to arg_addr
  uint64 arg_addr;
  argaddr(0, &arg_addr);
  // To do: copy the content in my_history to the saved address
  if (copyout(p->pagetable,
              arg_addr,
              (char *)&my_history,
              sizeof(struct sched_history)) < 0)
    return -1;
  // To do: return the pid as well
  return p->pid;
}
