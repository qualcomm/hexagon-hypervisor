
:mod:`intpool` -- Management for a pool of interrupt handling threads
=====================================================================

.. module:: intpool


H2K_intpool_int
---------------

.. c:function:: H2K_intpool_int(u32_t intno, H2K_thread_context *me, u32_t hthread, H2K_vmblock_t *vmblock)

	:param intno: Interrupt number
	:param me: Context for the current thread
	:param hthread: Current hardware thread number
	:param vmblock: VM which has the thread pool for handling this interrupt

Description
~~~~~~~~~~~

This function attempts to schedule a thread to handle an interrupt in an efficient way.

Functionality
~~~~~~~~~~~~~

The :c:func:`H2K_inpool_int()` attempts to schedule a thread to handle an interrupt in an efficient way.

First, acquire the BKL.

If there is no thread available to take the interrupt, soft-pend the interrupt.
A thread attempting to wait for a new interrupt will check the soft-pend list first.
If we soft pend the interrupt, we are done.

If a thread was interrupted, we unschedule it and put it in the ready queue.
Otherwise, the thread was idle, and so we mark ourselves as no longer idle.

We clear the low-priority status of the current thread, change our IMASK
appropriately, and place the woken thread directly into the runlist and switch
to it.  This is speculative; the woken thread may not be higher priority than 
the interrupted thread.  However we expect this to be likely, and if it is incorrect
it will be remedied by :c:func:`H2K_check_sanity()`.

Note that we do not re-enable the interrupt at this time.  The interrupt is only
re-enabled when a thread explicitly calls to enable the interrupt, or waits with 
the int_ack field set to a valid interrupt.  By keeping the interrupt disabled,
we allow it to pend while the interrupt is being handled.

Note that this functionality is very similar to H2K_popup_int


H2K_intpool_int
---------------

.. c:function:: H2K_intpool_wait(u32_t int_ack_num, H2K_thread_context *me)

	:param int_ack_num: Interrupt to enable, or -1 for no interrupt
	:param me: Context for the current thread

Description
~~~~~~~~~~~

This function optionally enables an interrupt, and then waits for an interrupt to arrive.

Functionality
~~~~~~~~~~~~~

Check to make sure interrupt number is valid.  If so, reenable the interrupt.

Acquire the BKL.

Check to see if an interrupt was already pending (due to no thread being ready to take it).
If so, clear the interrupt and return immediately.

Otherwise, add the thread to the ring of ready interrupt handling threads, mark as BLOCKED,
and select a new thread to schedule.

TO INVESTIGATE: try to preempt before calling dosched in case the interrupt can be taken
immediately?



Testing
-------

Important cases
~~~~~~~~~~~~~~~

* Interrupts pending
* Interrupts not pending
* Handling threads available
* Handling threads not available
* Cancellation


Harness
~~~~~~~





