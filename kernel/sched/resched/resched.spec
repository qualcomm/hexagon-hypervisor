
:mod:`resched` -- handle a reschedule interrupt
================================================

.. module:: resched

H2K_resched
-----------

.. cfunction:: void H2K_resched(u32_t unused, H2K_thread_context *me, u32_t hthread)

Description
~~~~~~~~~~~

H2K_resched handles a rescheduling interrupt 

Input
~~~~~

Argument 0: unused
Argument 1: A pointer to the currently running thread's context (NULL if idle thread was interrupted)
Argument 2: the hardware thread number

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

First, this function acknowledges the reschedule interrupt.

Next, we acquire the Big Kernel Lock. 

If the current thread is NULL, WAIT mode was interrupted, so we clear the wait
mask bit.  Otherwise, we remove the currently running thread, and append it to
the ready queue.

We then call H2K_dosched.


Testing
-------

Samples
~~~~~~~

* input: Pointer to the current thread (or NULL)
* input: Hardware Thread number
* input: If me != NULL, Runlist structure including me thread in the correct location
* output: if me == NULL, clear the H2K_wait_mask bit corresponding to the hardware thread number
* i/o:  if me != NULL, readylist structure will have me appended to it
* Must call H2K_dosched when finished

Important Cases
~~~~~~~~~~~~~~~

* me == NULL, bit set in H2K_wait_mask (bit must be cleared)
* me == NULL, bit not set in H2K_wait_mask (bit must stay cleared)
* me != NULL, thread must be removed from runlist and added to readylist
* All: must call H2K_dosched at end of routine

Check to make sure continuation is set?

Harness
~~~~~~~

H2 lib kernel will be built, and linked with the test harness.

The test harness will define our own H2K_dosched(), to replace the function in
the H2 kernel.  It will set a flag indicating that it was called.

For each test, we initialize and set up the appropriate structures (runlist,
readylist, wait_mask, etc), and then call H2K_resched().  We check to make
sure that our version of H2K_dosched() was called, and check to make sure
other appropraite actions were taken.


