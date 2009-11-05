
UNIT: resched

FUNCTION: void reschedule_from_wait(int hthread)

DESCRIPTION:

Reschedule_from_wait handles a rescheduling interrupt from an idle thread.

INPUT:

Argument 0: the hardware thread number

OUTPUT:

FUNCTIONALITY:

First, this function acknowledges the reschedule interrupt.

Next, we acquire the Big Kernel Lock. 

We clear the bit corresponding to our hthread out of the waitmask.

We then call dosched.



FUNCTION: void reschedule_from_lowprio(int hthread)

DESCRIPTION:

Reschedule_from_lowprio handles a rescheduling interrupt from a low priority
running thread.

INPUT:

Argument 0: unused
Argument 1: A pointer to the currently running thread's context
Argument 2: the hardware thread number

OUTPUT:

FUNCTIONALITY:

First, this function acknowledges the reschedule interrupt.

Next, we acquire the Big Kernel Lock. 

We remove the currently running thread, and append it to the ready queue.

We then call dosched.


