
:mod:`thread_stop` -- terminate thread
======================================

.. module:: thread_stop

H2K_thread_stop
---------------

.. c:function:: void H2K_thread_stop(s32_t status, H2K_thread_context *me)

	:param status: The status value to record in ``vmblock->status``
	:param me: The pointer to the current thread context

Description
~~~~~~~~~~~

Terminate the calling thread and decrement the count of running virtual CPUs
for the calling VM.  If the new count is 0 or the status is non-zero, post the
child-event interrupt to the virtual cpu (parent) that created the calling VM.
If the parent no longer exists, deallocate storage for the calling VM if the
CPU count has reached 0.  Does not return.

The BKL must NOT be held on entry.  ``H2K_thread_stop`` acquires the BKL and
then calls :c:func:`H2K_thread_stop_withlock()`.  Callers that already hold
the BKL must call :c:func:`H2K_thread_stop_withlock()` directly.

Functionality
~~~~~~~~~~~~~

First, we acquire the BKL.  The remainder of the work is performed by
:c:func:`H2K_thread_stop_withlock()`.


H2K_thread_stop_withlock
------------------------

.. c:function:: void H2K_thread_stop_withlock(s32_t status, H2K_thread_context *me)

	:param status: The status value to record in ``vmblock->status``
	:param me: The pointer to the current thread context

Description
~~~~~~~~~~~

The body of :c:func:`H2K_thread_stop()` for callers that already hold the
BKL.  Used by paths such as :c:func:`H2K_vm_do_work_withlock()` that are
entered with the BKL held (for example, the deferred-work hook in
``H2K_switch``).  Does not return.

INPUT_ASSERT(kernel_locked)

Functionality
~~~~~~~~~~~~~

Attempts to post the child-event interrupt if the new vcpu count is 0, or
the status is non-zero, and the parent context can be found and its status
is not ``H2K_STATUS_DEAD``.  Else, when the vcpu count is 0, calls
:c:func:`H2K_mem_alloc_release()` to mark the vmblock as freeable (will not
be freed until we relinquish BKL).

Cancels associated timers, clears the thread context (which sets the valid field to DEAD),
then inserts the thread into the H2K_kg.free_threads list.

Finally, calls :c:func:`H2K_dosched()` to pick a new thread.  The current
thread is specified as NULL, rather than as the now-dead thread context
pointer.  ``H2K_dosched()`` consumes the BKL (released at the tail of ``H2K_switch``).




Testing
-------


Samples
~~~~~~~

* Input: `me` 

Important cases
~~~~~~~~~~~~~~~

* Valid thread as input
* Free thread list is empty
* Free thread list is non-empty

Harness
~~~~~~~

We link directly with the stop object file.

The test harness defines :c:func:`H2K_dosched()` to set a flag indicating that the dosched
routine was called.

The test harness calls :c:func:`H2K_thread_stop()` with a thread context pointer.  We
check to make sure :c:func:`H2K_dosched()` was called, and that the thread was added to the
free thread list, and that the values were cleared.


