
MODULE: hw

This unit contains intrinsics which implement interfaces
to hardware.


FUNCTION: static inline void ciad(u32_t mask)

DESCRIPTION:

Clears the Interrupt Auto Disable register using the specified mask.

INPUT:

Argument 0: A mask of which interrupts to clear.  It is implementation-defined on which 
	interrupts in the mask correspond to which hardawre interrupts.

OUTPUT:

FUNCTIONALITY:

We invoke the CIAD instruction with the designated mask.


FUNCTION: static inline void change_imask(u32_t thread, u32_t imask)

DESCRIPTION:

Changes the IMASK register for the specified thread to ``imask``

INPUT:

Argument 0: The thread to change
Argument 1: The IMASK value to write

OUTPUT:

FUNCTIONALITY:

The destination thread is put in a predicate register, and then we use the
SETIMASK instruction to modify the IMASK for the thread.


FUNCTION: static inline void resched_int()

DESCRIPTION:

This function raises the reschedule interrupt.

INPUT:


OUTPUT:


FUNCTIONALITY:

The constant RESCHED_INT_INTMASK is placed into a register, and the SWI
instruction is invoked with that register.


FUNCTION: static inline void highprio_imask(u32_t hthread)

DESCRIPTION:

Changes the IMASK of the current thread to be appropriate for a non-low-priority thread.

INPUT:

Argument 0: The hardware thread 

OUTPUT:

FUNCTIONALITY:

We form the appropriate IMASK value for the current hardware thread to mask off
most of the interrupts, and then place that value in IMASK.




FUNCTION: static inline void lowprio_imask(u32_t hthread)

DESCRIPTION:

Changes the IMASK of the current thread to be appropriate for a low-priority thread.

INPUT:

Argument 0: The hardware thread 

OUTPUT:

FUNCTIONALITY:

We form the appropriate IMASK value for the current hardware thread to enable
most of the interrupts, and then place that value in IMASK.


FUNCTION: static inline u32_t get_ssr()

DESCRIPTION:

Returns the value of the SSR register.

INPUT:

OUTPUT:

Returns the value of the SSR register

FUNCTIONALITY:

The SSR value is transfered to a temporary value that is returned.




FUNCTION: static inline u32_t get_hwtnum()

DESCRIPTION:

Returns the current hardware thread number.

INPUT:

OUTPUT:

Returns the current hardware thread number.

FUNCTIONALITY:

For V3 and earlier, we extract the TNUM field from SSR.  For V4, we use the HTNUM register.




FUNCTION: static inline void BLASTK_mutex_lock_k0()

AVAILABILITY(ARCH >= 3)

DESCRIPTION:

Lock the K0 hardware lock.

INPUT:

OUTPUT:

FUNCTIONALITY:

This merely executes the K0LOCK instruction.






FUNCTION: static inline void BLASTK_mutex_unlock_k0()

AVAILABILITY(ARCH >= 3)

DESCRIPTION:

Unlock the K0 hardware lock.

INPUT:

OUTPUT:

FUNCTIONALITY:

This merely executes the K0UNLOCK instruction.








