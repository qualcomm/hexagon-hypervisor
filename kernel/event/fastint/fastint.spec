
ASM_REF_CODE(fastint requires special register changes difficult in C)

UNIT: fastint

FUNCTION: BLASTK_fastint_call(unsigned int intno, BLASTK_thread_context *me, unsigned int hthread, 
			void *fastint_contexts, void **fastint_functions, unsigned ciad_mask)

DESCRIPTION:

This function calls a fast interrupt

INPUT:

Argument 0: Interrupt number
Argument 1: Context for the current thread
Argument 2: Current hardware thread number
Argument 3: Pointer to the fastint contexts (passed for performance)
Argument 4: Pointer to the table of fastint function addresses (passed for performance)
Argument 5: Mask of current interrupt to use with the CIAD instruction (passed for performance)
Register 6: Value to use as GP (passed for performance)

OUTPUT:

FUNCTIONALITY:

The fastint_call routine sets up the environment for a fast interrupt handler, and
then calls the fast interrupt handler.

When the fast interrupt handler returns, we clear the IAD bit for the fast interrupt.
We then check to see if any interrupts are pending in IPEND.  

If so, we set IMASK to mask off non-fast interrupts, set SSR to enable
interrupts, and execute enough instructions to receive interrupts -- an isync
and a nop.  We then set SSR to disable interrupts, and jump to the context restore.

If IPEND was clear all along, we simply jump to the context restore.

TBD: should fast interrupts be able to jump to resched also?  Handy for the commonish
case that a fastint causes a reschedule event...


FUNCTION: BLASTK_interrupted_fastint_check()

DESCRIPTION:

This routine gets called from handle_int when we detect that the check at the end of 
the fastint handler was interrupted.  Call the correct fastint handler.

INPUT:

Several registers are assumed to be in the correct state from fastint_call.

OUTPUT:

FUNCTIONALITY:

This function depends on knowing the state at the time of the interrupt. 

SGP and R0 have been swapped by the context save routine.

IMASK needs to be restored to the value in R13

Precomputed SSR value for the fast interrupt is in R14

Registers r28-r31 should be restored from the context.  R31 contains the correct
return address for the fast interrupt, which means that a JUMPR instruction will
return to the correct place.

Register r4 contains the address of the fast interrupt function pointers

The function uses the known values to compute the correct fast interrupt to jump to,
and makes the jump.  The function will return to the correct location in fastint_call.

