
FUNCTION: void resched(u32_t unused, H2K_thread_context *me, u32_t hthread)

DESCRIPTION:

H2K_reschedule handles a rescheduling interrupt 

INPUT:

Argument 0: unused
Argument 1: A pointer to the currently running thread's context
Argument 2: the hardware thread number

OUTPUT:

FUNCTIONALITY:

First, this function acknowledges the reschedule interrupt.

Next, we acquire the Big Kernel Lock. 

If the current thread is NULL, WAIT mode was interrupted, so we clear the wait
mask bit.  Otherwise, we remove the currently running thread, and append it to
the ready queue.

We then call H2K_dosched.


