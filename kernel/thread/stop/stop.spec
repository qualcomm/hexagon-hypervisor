
:mod:`thread_stop` -- terminate thread
======================================

.. module:: thread_stop

H2K_thread_stop
---------------

.. cfunction:: void H2K_thread_stop(s32_t status, H2K_thread_context *me)

	:param me: The pointer to the current thread context

Description
~~~~~~~~~~~

Terminate the calling thread and decrement the count of running virtual CPUs
for the calling VM.  If the new count is 0 or the status is non-zero, post the
child-event interrupt to the virtual cpu (parent) that created the calling VM.
If the parent no longer exists, deallocate storage for the calling VM if the
CPU count has reached 0.  Does not return.

Functionality
~~~~~~~~~~~~~

First, we acquire the BKL and attempt to post the child-event interrupt if the
new vcpu count is 0, or the status is non-zero, and the parent context can be
found and its status is not H2K_STATUS_DEAD.  Else, when the vcpu count is 0,
we call :cfunc: `H2K_mem_alloc_release()` to mark the vmblock as freeable (will
not be freed until we relinquish BKL).

Next, we cancel associated timers and remove the current thread from the
runlist.  We then clear the thread context.  This has the effect of setting the
valid field to DEAD.

We then insert the thread into the H2K_kg.free_threads list.

Finally, we call :cfunc:`H2K_dosched()` to pick a new thread.  The current thread
should be specified as NULL, rather than as the now-dead thread context pointer.




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

We link directly with the stop object file, and also the runlist object file.

The test harness defines :cfunc:`H2K_dosched()` to set a flag indicating that the dosched
routine was called.  It also adds test threads correctly into the runlist.

The test harness calls :cfunc:`H2K_thread_stop()` with a thread context pointer.  We
check to make sure :cfunc:`H2K_dosched()` was called, and that the thread was added to the
free thread list, and that the values were cleared.


