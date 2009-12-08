

UNIT: dosched

FUNCTION: void H2K_dosched(thread_context *me, u32_t hthread)

DESCRIPTION:

H2K_dosched schedules the highest priority ready thread for execution, or goes to
sleep if there is no ready thread.

INPUT:

INPUT_ASSERT(kernel_locked)

Argument 0: the context of the currently running thread
Argument 1: the hardware thread number

OUTPUT:


FUNCTIONALITY:

The BKL must be held before calling dosched.

The currently running thread must have already been removed from the list of 
running threads.

First, we remove the best ready thread.  If no ready thread is available, we 
go to sleep:
	0. If a running hthread is marked as lowest priority, it should be made non-lowprio
	1. The bits corresponding to the current hthread should be set in the priomask and
		the waitmask: this thread will be waiting and lowest priority
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

