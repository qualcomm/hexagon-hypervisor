
UNIT: check_sanity

FUNCTION: unsigned long long int check_sanity(const unsigned long long int returnval);

DESCRIPTION:

Check_sanity checks that the kernel is in a correct state before leaving the
kernel, and takes any necessary action to fix the state if it is incorrect.

Check_sanity is not an assertion that no errors have happened, but instead
is a check to see if there is more work to do.

Some state is allowed to temporarily incorrect for performance or feature
reasons.  One example of this is whether or not the worst priority running
thread is better or equal to the best priority ready thread.  For example, to
support multiple wakeup, we place all woken threads into the ready data
structure.  The newly ready threads may be higher priority than all running
threads.  We use check_sanity to detect this situation and take corrective
action.

check_sanity must return the input argument.  This facilitates use during 
the system call return process.

INPUT:

INPUT_ASSERT(kernel_locked)

Argument 0: The value that must be returned by the function.

OUTPUT:

OUTPUT_ASSERT(kernel_locked)
The specified return value should be returned.


FUNCTIONALITY:

If the highest priority ready thread is higher than the lowest priority running
thread, raise the reschedule interrupt.

If the ready thread mask is non-zero (indicating there is a ready thread), and
the thread waitmask is also non-zero (indicating there is a thread that is
asleep), raise the reschedule interrupt.

If the running thread mask has any bits set that are not set in the
ready_validmask, then we need to make the lowest priority running thread
interruptible, and raise the reschedule interrupt.

If the priomask is zero, then no thread has been designated the lowest priority
running thread.  Make the lowest priority running thread interruptable.


FUNCTION: unsigned long long int check_sanity_unlock(const unsigned long long int returnval);

DESCRIPTION:

check_sanity_unlock performs the same checks as check_sanity, and additionally
unlocks the kernel.  This facilitates its use as a sibling call, as
check_sanity and unlock are common in the system call return process.

check_sanity_unlock must return the input argument.  This facilitates use
during the system call return process.

INPUT:

INPUT_ASSERT(kernel_locked)

OUTPUT:

OUTPUT_ASSERT(!kernel_locked)

FUNCTIONALITY:

Implement the functionality of check_sanity.

Unlock the BKL.

Return returnval.

