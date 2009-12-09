
:mod:sched_yield -- yield to next thread at same priority
=================

.. module:: sched_yield

H2K_sched_yield
---------------

.. cfunction:: void H2K_sched_yield(thread_context *me)

Description
~~~~~~~~~~~

The H2K_sched_yield function yields execution to all other threads at the same priority.

Input
~~~~~

Argument 0: Pointer to the current thread context

Output
~~~~~~

None

Functionality
~~~~~~~~~~~~~

First, we acquire the Big Kernel Lock.

If the readylist at the current priority is empty, the H2K_sched_yield function may
return immediately after releasing the BKL.  (EJP: can we do this outside the BKL?
Another thread might be inserting itself into the ready list.  Can we spec the function
so that it does a best effort?)

Next, the H2K_sched_yield function removes the current thread from the runlist, and
appends it on the end of the ready list.  

We can then call dosched to pick a new thread to run.

As an optimization, we can instead remove the thread at the head of the
readylist at the same priority, and insert it into the runlist.  We switch to
the thread inserted into the runlist.

