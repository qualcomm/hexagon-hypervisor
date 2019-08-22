
:mod:`sched_yield` -- yield to next thread at same priority
===========================================================

.. module:: sched_yield

H2K_sched_yield
---------------

.. c:function:: void H2K_sched_yield(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

The :c:func:`H2K_sched_yield()` function yields execution to all other threads at the same priority.

Functionality
~~~~~~~~~~~~~

First, we acquire the Big Kernel Lock.

If the readylist at the current priority is empty, the :c:func:`H2K_sched_yield()` function may
return immediately after releasing the BKL.  (EJP: can we do this outside the BKL?
Another thread might be inserting itself into the ready list.  Can we spec the function
so that it does a best effort?)

Next, the :c:func:`H2K_sched_yield()` function removes the current thread from the runlist, and
appends it on the end of the ready list.  

We can then call :c:func:`H2K_dosched()` to pick a new thread to run.

As an optimization, we can instead remove the thread at the head of the
readylist at the same priority, and insert it into the runlist.  We switch to
the thread inserted into the runlist.




Testing
-------


Samples
~~~~~~~

* Input: me, which may or may not have a valid thread
* Input: correct ready queue, which may or may not have ready threads
* I/O: runlist, which is updated if there is a new thread to schedule
* I/O: waitmask, modified if new is NULL

Important cases
~~~~~~~~~~~~~~~

* Runlist has no valid threads at current thread's priority
* Runlist has valid threads at current thread's priority

Harness
~~~~~~~

Link with H2 kernel library.

If valid threads exist at the current priority, :c:func:`H2K_sched_yield()` should lock
the kernel, remove the current thread from the runlist, append the current
thread to the ring at the current thread priority, and call :c:func:`H2K_dosched()`.

If no valid threads exist at the current priority, :c:func:`H2K_sched_yield()` may
return immediately.  Since this optimization may be important for performance
during spin code, we want to verify that it has been implemented.



