/*
 * kthread.c            A module that runs a kernel thread
 *                      for one minute.  The task remains
 *                      in the runqueue for the one minute,
 *                      but yields control to other tasks
 *                      during that time.
 *
 * based on:
 * http://tuxthink.blogspot.com/2011/02/kernel-thread-creation-1.html
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/time.h>   // for using jiffies  
#include <linux/timer.h>

static struct task_struct *task;
static const char TASK_NAME[] = "erins_task";

/*
 * Create a task which will be scheduled on a cpu,
 * and then relinquish control to other tasks
 * for 1 minute (effectively sleeping).
 *
 * re: schedule: http://www.linuxjournal.com/article/8144
 */
int task_fn(void *data)
{
  unsigned long j0,j1;
  int delay = 60*HZ;

  printk(KERN_INFO "task is executing\n");

  // A jiffy is the time between two
  // ticks of the system timer interrupt.
  //
  // It is a consistent measurement
  // (usually 1-10ms) on a single os
  // and platform.
  //
  // The global variable jiffies
  // is a counter of the number of ticks
  // since the system booted.
  //
  // There are HZ timer interrupts in a
  // second.  The system uptime is then
  // jiffies/HZ.
  // 
  // We can use the current jiffies count
  // as representation of the time now.
  j0 = jiffies; 
  j1 = j0 + delay; 

  // time_before compares jiffies counts
  // accounting for common overflow problems.
  while (time_before(jiffies, j1)) {

    // Voluntarily relinquish the cpu,
    // so the scheduler can schedule other
    // processes on this cpu.
    //
    // Important: if the task is in the state
    // TASK_RUNNING, this keeps the task in
    // the runqueue (tied to this cpu).  That
    // means that other tasks with lower priorities
    // will be scheduled instead, but the scheduler
    // still needs to recalculate priorities with
    // this extra do-nothing task.
    // 
    // If instead the task is put into state
    // TASK_INTERRUPTIBLE or TASK_UNINTERRUPTIBLE,
    // the task is removed from the runqueue entirely.
    // Then another task needs to call wake_up_process
    // to put the task back in TASK_RUNNING, and
    // therefore unto the runqueue, when this task
    // should stop sleeping.
    //
    // Here we do the former because we don't want
    // to deal with another process just for waking
    // up this task after a certain amount of time.
    //
    // In a module with lots of tasks, it might make
    // more sense for a supervisor task to manage
    // waking up unqueued tasks.
    schedule();
  }

  // Once this task has stopped letting other
  // processes be scheduled, simply exit.
  // This triggers do_exit().
  printk(KERN_INFO "task is exiting\n");
  return 0;
}

/*
 * Create a task and set its state to TASK_RUNNING,
 * which puts it on a runqueue.
 */
int task_init(void)
{

  // Create a task that can be scheduled.
  task = kthread_create(task_fn,NULL,TASK_NAME);
  printk(KERN_INFO "created task\n");

  if(task) {
    
    // Set the task struct as runnable.
    // The scheduler will start executing (parts of)
    // this task's time slice asap.
    wake_up_process(task);
    printk(KERN_INFO "marked task as runnable\n");
  }
  return 0;
}

/*
 * No cleanup work is necessary because the task itself
 * exits.  This causes do_exit() to be called with the
 * task's return value.  do_exit() sets the task's state
 * to TASK_DEAD and calls schedule(), which ensures that
 * the task never again regains control and is removed by
 * the scheduler from the runqueue.
 *
 * We could stop the task by sending a signal to it using
 * kthread_stop.  This allows the ending task to pass up
 * a return value to task calling kthread_stop.
 * The ending task must explicitly listen for signals, or
 * else kthread_stop does nothing.
 */
void task_cleanup(void)
{
}

/*
 * Create the module.
 */
MODULE_LICENSE("GPL");
module_init(task_init);
module_exit(task_cleanup);
