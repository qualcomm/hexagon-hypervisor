
ASM_REF_CODE(Switch involves special registers and sleep, and cannot be written in C.)

UNIT: switch

FUNCTION: void BLASTK_switch(thread_context *from, thread_context *to)

DESCRIPTION:

BLASTK_switch switches to a new thread that has been chosen to be scheduled.
If the new thread is NULL, we will go to wait mode.

INPUT:

INPUT_ASSERT(kernel_locked)

Argument 0: the context of the currently running thread (or NULL)
Argument 1: the context of the new thread to run (or NULL)

OUTPUT:



FUNCTIONALITY:

TBD: where do we accumulate thread/wait time?
TBD: do we set up PMU stuff here?

If ``from`` is not NULL, we accumulate the difference between the 
thread execution pcycles and the current pcycles is added to the 
cumulative CPU cycles for the thread.

If ``to`` is NULL, we go to wait mode:
	0. Unlock the big kernel lock
	1. Set SGP to NULL
	2. Jump to BLASTK_wait_forever

Otherwise, we set SGP to the new thread context, save the current pcycles as
the thread start time, load the continuation for the new thread into the link
register, and jump to BLASTK_check_sanity_unlock.  BLASTK_check_sanity_unlock
will return to the continuation.

