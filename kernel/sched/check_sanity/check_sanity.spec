:mod:`check_sanity` -- routines for checking scheduler state
============================================================

.. module:: check_sanity

H2K_check_sanity
-------------------

.. cfunction:: u64_t H2K_check_sanity(const u64_t returnval)

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
threads.  We use check_sanity to detect this situation and take corrective
action.

This function must return the input argument.  This facilitates use during 
the system call return process.
   
Input
~~~~~

Argument 0: The value that must be returned by the function.


Output
~~~~~~

Should return Argument 0.


.. InputAssert::
   assert(kernel_locked());
   
   
.. OutputAssert::
   assert(retval == arg0);


Functionality
~~~~~~~~~~~~~
 
If the highest priority ready thread is higher than the lowest priority running
thread, raise the reschedule interrupt.

If the ready thread mask is non-zero (indicating there is a ready thread), and
the thread waitmask is also non-zero (indicating there is a thread that is
asleep), raise the reschedule interrupt.

If the running thread mask has any bits set that are not set in the
ready_validmask, then we need to make the lowest priority running thread
interruptible, and raise the reschedule interrupt.

If the priomask is zero, then no thread has been designated the lowest priority
running thread.  Make the lowest priority running thread interruptible.


H2K_check_sanity_unlock
--------------------------

.. cfunction:: u64_t H2K_check_sanity_unlock(const u64_t returnval)
   
Description
~~~~~~~~~~~
   
This function performs the same checks as check_sanity, and additionally
unlocks the kernel.  This facilitates its use as a sibling call, as
check_sanity and unlock are common in the system call return process.
   
This function must return the input argument.  This facilitates use
during the system call return process.

   
Input
~~~~~
Argument 0: The value that must be returned by the function.

  
Output
~~~~~~

Should return Argument 0.


.. InputAssert::
   assert(kernel_locked());


.. OutputAssert::
   assert(retval == arg0);
   assert(kernel_unlocked());
   

Functionality
~~~~~~~~~~~~~
   
Implement the functionality of check_sanity.
   
Unlock the BKL.
   
Return returnval.



Testing
-------

Samples
~~~~~~~

MAX_PRIOS -- number of priority levels available
MAX_HTHREADS -- number of hardware threads available

H2K_priomask defined in lowprio.h (extern)
H2K_wait_mask defined in lowprio.h (extern)
H2K_ready_valids defined in readylist.h (extern)

H2K_runlist_worst_prio() defined in runlist.h (static inline); uses runlist_valids
H2K_ready_best_prio() defined in readylist.h (static inline); uses ready_valids

H2K_lowprio_notify() defined in lowprio.h (static inline); changes imask 
H2K_resched_int() defined in hw.h (static inline); sends the reschedule interrupt

States
~~~~~~


Matrix
~~~~~~


Harness
~~~~~~~

H2 lib kernel will be built, and run with testcases as the main user thread.  It should not switch out.

Assertions should be turned on, and the call() convention used to call the debug wrappers.

Output will be inspected by looking at ipend and possibly diverting some other defines (?)


