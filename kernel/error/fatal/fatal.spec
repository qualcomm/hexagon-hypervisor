
UNIT: fatal

FUNCTION: void BLASTK_fatal_kernel(s16_t error_id, thread_context *me, u32_t info0, u32_t info1, u32_t hthread)

DESCRIPTION: 

BLASTK_fatal_kernel handles a fatal kernel error.  The kernel will attempt to
halt execution completely.

INPUT:

Argument 0: The ID of the fatal kernel error
Argument 1: The context of the currently running thread
Argument 2: The hardware thread number

OUTPUT:

FUNCTIONALITY:

First, we log the error.

Next, we shutdown all threads by trapping to the angel handler to exit simulation.

TBD: how on target?


FUNCTION: void BLASTK_fatal_thread(s16_t error_id, BLASTK_thread_context *me, u32_t info0, u32_t info1, u32_t hthread)

DESCRIPTION:

BLASTK_fatal_thread handles a fatal thread error.  A fatal thread error is an error
from a thread that the thread cannot handle.  For example, if the thread event vectors
have a page fault, the thread is incapable of recovering from an error.  

When a fatal thread error occurs, the thread will be terminated and the error
will be logged.

INPUT:

Argument 0: The ID of the fatal thread error
Argument 1: The context of the currently running thread
Argument 2: The hardware thread number

FUNCTIONALITY:

We log the error, and then sibcall to BLASTK_thread_stop.


