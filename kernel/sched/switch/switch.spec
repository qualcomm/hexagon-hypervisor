
ASM_REF_CODE(Switch involves special registers and sleep, and cannot be written in C.)

:mod:`switch` -- Switch to a new thread
=======================================

.. module:: switch

H2K_switch
----------

.. cfunction:: void H2K_switch(thread_context *from, thread_context *to)

	:param from: the context of the currently running thread (or NULL)
	:param to: the context of the new thread to run (or NULL)

Description
~~~~~~~~~~~

:cfunc:`H2K_switch()` switches to a new thread that has been chosen to be scheduled.
If the new thread is NULL, we will go to wait mode.

.. InputAssert::
	ASSERT(kernel_locked())


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
	2. Jump to :cfunc:`H2K_wait_forever()`

Otherwise, we set SGP to the new thread context, set the current hthread 
in the new thread context, save the current pcycles as the thread start time,
load the continuation for the new thread into the link register, load the
saved r1:0 (which contain a return value from the blocking function), and jump
to :cfunc:`H2K_check_sanity_unlock()`.  :cfunc:`H2K_check_sanity_unlock()` will return to the
continuation.






Testing
-------

Samples
~~~~~~~

* Input: from, which may be NULL
* Input: to, which may be NULL

* Flow: If to is non-NULL, we should arrive at :cfunc:`H2K_check_sanity_unlock()` with the return address 
    set to the continuation value in "to"
* Ouptut: If to is NULL, we should unlock the kernel lock, 
* Flow: if to is NULL, we should jump to H2K_wait_forever
* I/O: H2K_wait_mask bit set for our hardware thread only if to is NULL
* I/O: H2K_priomask bit set for our hardware thread only if to is NULL
* Output: if to is NULL, we should additionally have the machine configured to be in WAIT mode:
  * The kernel is unlocked
  * SGP for our hardware thread is set to NULL
  * IMASK for our hardware thread is set to receive interrupts
  * SSR.IE should be set and SSR.EX should be cleared.
* TBD: cycle accounting
* TBD: tracing
* TBD: tid setup


Important Cases
~~~~~~~~~~~~~~~

* From non-NULL, to non-NULL
* From non-NULL, to NULL
* From NULL, to non-NULL
* From NULL, to NULL

Harness
~~~~~~~

Since the H2K_check routine has low requirements on the rest of the kernel, we 
only will link the switch object file with the test harness.

One problem with testing H2K_switch is that it will attempt to go into wait mode. 
For tests that should go into wait mode, we replace the H2K_wait_forever function
with a jump to the code that checks to make sure the variables have been set correctly.
We do this in the following manner::

	u32_t *code_snippet_address;
	__asm__ __volatile__ (
		" call 1f \n"
		" r28.h = #hi(TH_wait_check) \n"
		" r28.l = #lo(TH_wait_check) \n"
		" jumpr r28 \n"
		"1: \n"
		" %0 = r31 \n"
		: "=r"(code_snippet_address) : : "r28","r31");
	memcpy(H2K_wait_forever,code_snippet_address,3*sizeof(u32_t));

This will create a code snippet that jumps to our TH_wait_check routine.  We
get the address of this code snippet, and overwrite the H2K_wait_forever code
with our new snippet.  Our TH_wait_check routine can check to make sure the
machine is correctly configured to be in WAIT mode.


