
UNIT: lowprio

FUNCTION: static inline void H2K_lowprio_notify()

DESCRIPTION:

H2K_lowprio_notify identifies a new thread to be the lowest-priority thread
for receiving interrupts, and takes action to make that thread be receptive to
interrupts.

INPUT:

OUTPUT:

FUNCTIONALITY:

First, we get the priority of the worst priority running thread.  We look at the 
H2K_runlist entry at that priority, which points to the thread that was inserted
at the worst priority most recently.  We then look at the hthread field of this 
thread, and use that to set the H2K_priomask bit corresponding to the 
hardware thread and to call thread_prio_change_low().


FUNCTION: static inline void H2K_lowprio_raise()

DESCRIPTION:

H2K_lowprio_raise takes the thread currently marked as lowest priority, and 
modifies the state to indicate it is no longer lowest priority.

INPUT:

OUTPUT:

FUNCTIONALITY:

If H2K_wait_mask is nonzero, we return, as we should never mask interrupts on 
a waiting thread.  Otherwise, count the trailing zeros of H2K_priomask, which
yields the hardware thread that should no longer be the low priority thread.  
We clear that bit from the H2K_priomask and call H2K_prio_change_high() for
the hardware thread.  


