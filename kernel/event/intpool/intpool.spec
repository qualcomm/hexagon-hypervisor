
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

Note that we do not re-enable the interrupt at this time.  The interrupt is only
re-enabled when a thread explicitly calls to enable the interrupt, or waits with 
the int_ack field set to a valid interrupt.  By keeping the interrupt disabled,
we allow it to pend while the interrupt is being handled.

Note that this functionality is very similar to H2K_popup_int


H2K_intpool_wait
---------------

.. c:function:: int H2K_intpool_wait(u32_t int_ack_num, H2K_thread_context *me)

	:param int_ack_num: Interrupt to enable, or -1 for no interrupt
	:param me: Context for the current thread
	:returns: the interrupt number on wake, -1 on failure (including spurious VMWORK bail)

Description
~~~~~~~~~~~

This function optionally enables an interrupt, and then waits for an interrupt to arrive.

Per the VMWORK contract (see :doc:`../../util/vmdefs/vmdefs`), -1 may
be returned spuriously when VMWORK is observed after the BKL is
acquired.  Callers must treat -1 as "did not block -- re-check the
wait condition and retry."

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


H2K_intpool_configure
---------------------

.. c:function:: int H2K_intpool_configure(u32_t intno, u32_t enable, H2K_thread_context *me)

	:param intno: Interrupt to configure
	:param enable: Nonzero to enable interrupt-pool handling, zero to disable it
	:param me: Thread context whose VM owns the interrupt pool
	:returns: 0 on success, -1 if the interrupt number is not available

Description
~~~~~~~~~~~

Enables or disables interrupt-pool handling for an interrupt belonging to the
current VM.

Functionality
~~~~~~~~~~~~~

The function accepts L1 or L2 interrupt numbers in the available interrupt
ranges.  When enabling, it installs :c:func:`H2K_intpool_int()` as the
interrupt handler, associates the handler with ``me->vmblock``, and enables
the interrupt.  When disabling, it disables the interrupt and clears its
handler entry.  The configuration update is protected by the BKL.


H2K_intpool_cancel
------------------

.. c:function:: void H2K_intpool_cancel(H2K_thread_context *dest)

	:param dest: Interrupt-pool thread to cancel

Description
~~~~~~~~~~~

Cancels a thread's wait in its VM's interrupt pool.

Functionality
~~~~~~~~~~~~~

The caller must hold the BKL.  The function removes ``dest`` from its VM's
interrupt-pool ring and sets its return interrupt number to ``-1``.


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





