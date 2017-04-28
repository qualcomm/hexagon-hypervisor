
:mod:`popup` -- Start up a popup thread, or wait for an interrupt to pop up
===========================================================================

.. module:: popup


H2K_popup_int
-------------

.. c:function:: H2K_popup_int(u32_t intno, H2K_thread_context *me, u32_t hthread)

	:param intno: Interrupt number
	:param me: Context for the current thread
	:param hthread: Current hardware thread number

Description
~~~~~~~~~~~

This function schedules a thread that is waiting for an interrupt.

Functionality
~~~~~~~~~~~~~

The :c:func:`H2K_popup_int()` schedules a thread that is waiting for an interrupt.

First, acquire the BKL.

If the popup thread is NULL, we merely unlock and return.

If a thread was interrupted, we unschedule it and put it in the ready queue.
Otherwise, the thread was idle, and so we mark ourselves as no longer idle.

We clear the low-priority status of the current thread, change our IMASK
appropriately, and place the woken thread directly into the runlist and switch
to it.  This is speculative; the woken thread may not be higher priority than 
the interrupted thread.  However we expect this to be likely, and if it is incorrect
it will be remedied by :c:func:`H2K_check_sanity()`.

Note that we do not re-enable the interrupt at this time.  The interrupt is only
re-enabled when the thread blocks waiting for another interrupt.  By keeping the 
interrupt disabled, we allow it to pend while the popup thread executes.


H2K_popup_int
-------------

.. c:function:: H2K_popup_wait(u32_t intno, H2K_thread_context *me)

	:param intno: Interrupt number
	:param me: Context for the current thread

Description
~~~~~~~~~~~

This function blocks a thread to waiting for an interrupt.

Functionality
~~~~~~~~~~~~~

Check to make sure interrupt number is valid.

Acquire the BKL.

Check to make sure that the interrupt handler is not already taken.  If it is, unlock and 
return error.

Set the interrupt handler to popup_int, and the extra interrupt argument to the thread context.

Do the work of blocking: take the thread out of the runlist, mark it as BLOCKED, and 
set the return value to a successful return of the interrupt.

Finally, enable the interrupt and select a new thread to schedule.

TO INVESTIGATE: try to preempt before calling dosched in case the interrupt can be taken
immediately?



Testing
-------

Samples
~~~~~~~

* Input: Interrupted thread context 
* Input: popup thread 
* Input: Interrupt number
* Flow: call to H2K_switch or H2K_dosched

Important cases
~~~~~~~~~~~~~~~

* Interrupted thread valid or NULL
* Popup thread valid or NULL
* All interrupt numbers

Harness
~~~~~~~




