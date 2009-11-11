
ASM_REF_CODE(Switch involves special registers and sleep, and cannot be written in C.)

UNIT: switch

FUNCTION: void BLASTK_switch(thread_context *from, thread_context *to)

DESCRIPTION:

BLASTK_switch switches to a new thread that has been chosen to be scheduled.
If the new thread is NULL, 

INPUT:

INPUT_ASSERT(kernel_locked)

Argument 0: the context of the currently running thread (or NULL)
Argument 1: the context of the new thread to run (or NULL)

OUTPUT:



FUNCTIONALITY:

TBD: where do we accumulate thread/wait time?
TBD: do we set up PMU stuff here?

If ``to`` is NULL, we go to wait mode:
	0. Unlock the big kernel lock
	1. Set SGP to the idle context pointer
	2. Jump to BLASTK_wait_forever

Otherwise, we set SGP to the new thread context, load the continuation for the
new thread, and jump to the continuation.

