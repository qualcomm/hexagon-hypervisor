:mod:`dosched` -- schedule a new thread
=======================================

.. module:: dosched

H2K_dosched
-----------

.. c:function:: void H2K_dosched(H2K_thread_context *me, u32_t hthread)

	:param me: the context of the currently running thread
	:param hthread: the hardware thread number

Description
~~~~~~~~~~~

:c:func:`H2K_dosched()` schedules the highest priority ready thread for execution, or goes to
sleep if there is no ready thread.

INPUT_ASSERT(kernel_locked)

.. fixme InputAssert::
	assert(kernel_locked());




Functionality
~~~~~~~~~~~~~

The BKL must be held before calling dosched.

The currently running thread must have already been removed from the list of 
running threads.

First, we remove the best ready thread.  If no ready thread is available, we 
go to sleep by switching from me to NULL

Otherwise, we set the new thread's hthread, switch to the thread by jumping to
:c:func:`H2K_switch()`, which does not return.

Testing
-------


Samples
~~~~~~~

* Input: me, which may or may not have a valid thread
* Input: correct ready queue, which may or may not have ready threads
* I/O: waitmask, modified if new is NULL

Important cases
~~~~~~~~~~~~~~~

* There are ready threads
* There are NOT ready threads

* me is NULL
* me is a valid thread

* Other waiting threads
* No other waiting threads

Cross of these options


Harness
~~~~~~~

In all cases, :c:func:`H2K_switch()` should be called.  

We define :c:func:`H2K_switch()` in the test harness.  This is a dummy function which sets
a flag indicating that the function was called.  :c:func:`H2K_switch()` should not return,
so we use setjmp/longjump to return to the function where dosched was called.

We link with the whole kernel lib.  However, :c:func:`H2K_switch()` will be already resolved, and
so will not be linked in.  We will not be in the real kernel environment; :c:func:`H2K_dosched()`
will be called directly.


