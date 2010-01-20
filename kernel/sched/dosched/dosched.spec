:mod:`dosched` -- schedule a new thread
=======================================

.. module:: dosched

H2K_check_sanity
----------------

.. cfunction:: void H2K_dosched(thread_context *me, u32_t hthread)

	:param me: the context of the currently running thread
	:param hthread: the hardware thread number

Description
~~~~~~~~~~~

:cfunc:`H2K_dosched()` schedules the highest priority ready thread for execution, or goes to
sleep if there is no ready thread.

INPUT_ASSERT(kernel_locked)

.. InputAssert::
	assert(kernel_locked());




Functionality
~~~~~~~~~~~~~

The BKL must be held before calling dosched.

The currently running thread must have already been removed from the list of 
running threads.

First, we remove the best ready thread.  If no ready thread is available, we 
go to sleep:
0. If a running hthread is marked as lowest priority, it should be made non-lowprio
1. Check to see if the ready valid mask requires further threads to be removed from the schedule, and take action if so.
2. We switch from me to NULL.

Otherwise, if the waitmask is zero, and the priority of the new thread is worse than all the
other running threads, and the current hthread is not already marked as the lowprio thread,
we make ourselves the lowest priority thread:
0. Make the current lowprio thread non-lowprio
1. The bit corresponding to the current hthread should be set in the priomask
2. Our IMASK should change to be appropriate for the lowprio hthread.

Otherwise, if the waitmask is nonzero, or the priority of the new thread is
better than all the other running threads, we check to see if we were marked as the 
lowprio thread.  If we were also marked as the lowprio hthread, we make ourselves non-lowprio:
0. Our IMASK should change to be appropriate for the non-lowprio thread.
1. If the waitmask is zero, we should "notify" the new lowprio hthread.

It is essential that we make ourselves non-lowprio here if appropriate, however it is NOT
essential that we find a lowprio hthread at this point.  check_sanity will find a lowprio
hthread if priomask is zero.




Testing
-------


Samples
~~~~~~~

* Input: me, which may or may not have a valid thread
* Input: correct ready queue, which may or may not have ready threads
* I/O: runlist, which is updated if there is a new thread to schedule
* I/O: priomask, modified if new is NULL or the newly selected thread 
  is lowest priority (XXX: or if newly selected thread is non-lowprio?)
* I/O: waitmask, modified if new is NULL

Important cases
~~~~~~~~~~~~~~~

* There are ready threads
* There are NOT ready threads

* me is NULL
* me is a valid thread

* We are the new lowest priority thread
* We are no longer the lowest priority thread
* We remain the lowest priority thread

* Other waiting threads
* No other waiting threads

Cross of these options


Harness
~~~~~~~

In all cases, :cfunc:`H2K_switch()` should be called.  

We define :cfunc:`H2K_switch()` in the test harness.  This is a dummy function which sets
a flag indicating that the function was called.  :cfunc:`H2K_switch()` should not return,
so we use setjmp/longjump to return to the function where dosched was called.

We link with the whole kernel lib.  However, :cfunc:`H2K_switch()` will be already resolved, and
so will not be linked in.  We will not be in the real kernel environment; :cfunc:`H2K_dosched()`
will be called directly.


