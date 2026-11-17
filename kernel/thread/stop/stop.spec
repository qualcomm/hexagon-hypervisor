:mod:`thread_stop` -- terminate a thread
=========================================

.. module:: thread_stop

H2K_thread_stop
---------------

.. c:function:: void H2K_thread_stop(s32_t status, H2K_thread_context *me)

	:param status: Status to record for the VM
	:param me: Current thread context

Description
~~~~~~~~~~~

Stops the current thread and does not return.  The BKL must not be held on
entry.

Functionality
~~~~~~~~~~~~~

Acquires the BKL and calls :c:func:`H2K_thread_stop_withlock()`.


H2K_thread_stop_withlock
------------------------

.. c:function:: void H2K_thread_stop_withlock(s32_t status, H2K_thread_context *me)

	:param status: Status to record for the VM
	:param me: Current thread context

Description
~~~~~~~~~~~

Stops a thread when the caller already holds the BKL.  Does not return.

Functionality
~~~~~~~~~~~~~

Decrements the VM CPU count, notifies the parent when required, and marks the
VM for release when no CPUs remain.  It cancels timers, removes the thread from
the runlist, clears its context, adds it to the free-thread list, and calls
:c:func:`H2K_dosched()` with no current thread.

.. note::

	Callers that already hold the BKL must use this function.  The wrapper
	:c:func:`H2K_thread_stop()` is for callers that do not hold it.


Testing
-------

Verify that stopping a thread clears its context, places it on the free list,
and calls :c:func:`H2K_dosched()`.
