
ASM_REF_CODE(Switch involves special registers and sleep, and cannot be written in C.)

:mod:`switch` -- Switch to a new thread
========================================

.. module:: switch

H2K_switch
----------

.. cfunction:: void H2K_switch(thread_context *from, thread_context *to)

Description
~~~~~~~~~~~

H2K_switch switches to a new thread that has been chosen to be scheduled.
If the new thread is NULL, we will go to wait mode.

Input
~~~~~

.. InputAssert::
	ASSERT(kernel_locked())

Argument 0: the context of the currently running thread (or NULL)
Argument 1: the context of the new thread to run (or NULL)

Output
~~~~~~



Functionality
~~~~~~~~~~~~~

TBD: where do we accumulate thread/wait time?
TBD: do we set up PMU stuff here?

If ``from`` is not NULL, we accumulate the difference between the 
thread execution pcycles and the current pcycles is added to the 
cumulative CPU cycles for the thread.

If ``to`` is NULL, we go to wait mode:
	0. Unlock the big kernel lock
	1. Set SGP to NULL
	2. Jump to H2K_wait_forever

Otherwise, we set SGP to the new thread context, set the current hthread 
in the new thread context, save the current pcycles as the thread start time,
load the continuation for the new thread into the link register, load the
saved r1:0 (which contain a return value from the blocking function), and jump
to H2K_check_sanity_unlock.  H2K_check_sanity_unlock will return to the
continuation.

