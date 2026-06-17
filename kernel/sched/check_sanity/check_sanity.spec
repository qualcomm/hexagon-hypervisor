:mod:`check_sanity` -- routines for checking scheduler state
============================================================

.. module:: check_sanity

H2K_check_sanity
----------------

.. c:function:: u64_t H2K_check_sanity(const u64_t returnval)

	:param returnval: value to be returned
	:returns: the value that was passed in at `returnval`

Description
~~~~~~~~~~~

This function checks that the kernel is in a correct state before leaving the
kernel, and takes any necessary action to fix the state if it is incorrect.
   
This function is not an assertion that no errors have happened, but instead
is a check to see if there is more work to do.

Some state is allowed to be temporarily incorrect for performance or feature
reasons.  One example of this is whether or not the worst priority running
thread is better or equal to the best priority ready thread.  For example, to
support multiple wakeup, we place all woken threads into the ready data
structure.  The newly ready threads may be higher priority than all running
threads.  We use :c:func:`H2K_check_sanity()` to detect this situation and take corrective
action.

This function must return the input argument.  This facilitates use during 
the system call return process.
   

.. fixme InputAssert::
   assert(checker_kernel_locked());

   
.. fixme OutputAssert::
   assert(checker_kernel_locked());
   assert(retval == arg0);


Functionality
~~~~~~~~~~~~~

If the priomask is zero, then no thread has been designated the lowest priority
running thread.  Make the lowest priority running thread interruptible
(:c:func:`H2K_lowprio_notify`).

Program the BESTWAIT register with the priority of the best ready thread
(:c:func:`H2K_ready_best_prio`).  On hardware that supports the reschedule
interrupt feature (SCHEDCFG.EN set at boot), the hardware then compares every
hardware thread's effective STID.PRIO against BESTWAIT and raises the reschedule
interrupt automatically when a running thread is worse than the best ready
thread -- replacing the former software scan of the worst running priority.
When it fires, the hardware resets BESTWAIT to 0x1FF (``BESTWAIT_MASK``) so the
interrupt is not raised again until BESTWAIT is reprogrammed.

The hardware comparator only covers the case where a *running* thread is worse
than a ready thread.  It cannot wake an idle (waiting) hardware thread purely
because work exists, so software still raises the reschedule interrupt directly
when there is a thread in wait mode (``wait_mask`` non-zero) and a ready thread
to run (``best < MAX_PRIOS``).  The ``H2K_get_bestwait() != BESTWAIT_MASK`` guard
skips this when the hardware has already fired for this arming.

Note: the BESTWAIT comparator reads the hardware STID.PRIO register, which is
written from a thread's software priority at switch-in.  Any code that changes a
*running* thread's priority in place (notably futex priority inheritance) must
also update that thread's STID.PRIO, or the comparator would use a stale value.

H2K_check_sanity_unlock
-----------------------

.. c:function:: u64_t H2K_check_sanity_unlock(const u64_t returnval)

	:param returnval: value to be returned
	:returns: the value that was passed in at `returnval`

Description
~~~~~~~~~~~
   
This function performs the same checks as :c:func:`H2K_check_sanity()`, and additionally
unlocks the kernel.  This facilitates its use as a sibling call, as
check_sanity and unlock are common in the system call return process.
   
This function must return the input argument.  This facilitates use
during the system call return process.


.. fixme InputAssert::
   assert(checker_kernel_locked());


.. fixme OutputAssert::
   assert(!checker_kernel_locked());
   assert(retval == arg0);


Functionality
~~~~~~~~~~~~~
   
Implement the functionality of :c:func:`H2K_check_sanity()`.
   
Unlock the BKL.
   
Return returnval.



Testing
-------

Samples
~~~~~~~

* input	MAX_PRIOS		number of priority levels available (default 256)
* input	MAX_HTHREADS		number of hardware threads available (default 6)
* input	H2K_kg.ready_valids	priorities with ready jobs
* input	H2K_kg.runlist_prios	priorities of currently running threads
* input	H2K_kg.waitmask		indicates hardware threads which are in wait
* output	bestwait		best ready priority armed for the hardware comparator (SCHEDCFG.EN)
* output	ipend			shows interrupts pending (see H2K_resched_int())
* i/o	H2K_kg.priomask		indicates lowest priority mask (see H2K_lowprio_notify)

States
~~~~~~

Important cases
~~~~~~~~~~~~~~~

* H2K_kg.waitmask == 0
* H2K_kg.waitmask != 0
* MAX(H2K_kg.runlist_prios) > CL0(H2K_kg.ready_valids)
* MAX(H2K_kg.runlist_prios) <= CL0(H2K_kg.ready_valids)
* H2K_kg.priomask == 0
* H2K_kg.priomask != 0


Harness
~~~~~~~

H2 lib kernel will be built, and run with testcases as the main user 
thread.  It should not switch out.

Assertions for checking return value and kernel locking should be 
turned on, and the call() convention used to call the debug wrappers.

Output state will be inspected by looking at IPEND and H2K_priomask.

The assertion for !checker_kernel_locked() should only be valid in a 
controlled unit testcase where nothing else can grab the lock before
we can check it.



