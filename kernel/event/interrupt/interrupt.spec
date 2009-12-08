
ASM_REF_CODE(Interrupt context save/restore not possible in C)

UNIT: interrupt

FUNCTION: H2K_handle_int()

DESCRIPTION:

This handles the common set of interrupts.  The interrupt handler carefully
saves any required context, and jumps to the appropriate handler for the 
next step.

INPUT:

OUTPUT:

FUNCTIONALITY:

Most interrupts redirect to handle_int.

Handle_int carefully saves off any required state.

If the idle thread was interrupted, we detect the case as early as possible and
skip the remainder of the context save.  If we have interrupted the idle
thread, we do not care about saving the values of the registers.

If the fast interrupt handler "interrupt check" was interrupted, we vector off
to the approrpate code to handle it.  See fast interrupts for more information.

Interrupts can be handled by calling the reschedule function, or calling the
function to handle fast interrupts.  The fast interrupt handler may be called
before saving the registers saved by the ABI to improve performance.

What function to call is determined by the table H2K_inthandlers.  The format
of each inthandler should be:
	H2K_handler(u32_t intno, H2K_thread_context *me, u32_t hwtnum)

Real interrupted threads (as opposed to, for example, the idle thread) that may 
reschedule should have int_context_restore as the end of their continuation
function.

We leave to execute the interrupt handler that calls a stub routine.  This stub
routine saves the return address in the continuation field before jumping to
the appropriate location.

FUNCTION: H2K_int_context_restore()

DESCRIPTION:

This routine returns to an interrupted thread.  It restores the registers from
the thread context and returns to the thread in the appropriate place.

INPUT:

OUTPUT:

FUNCTIONALITY:

The int_context_restore routine restores registers from the thread context and returns.



FUNCTION: H2K_interrupted_waitmode()

DESCRIPTION:

This routine is called when we detect that we were interrupted while in wait.  
This is a common case, and we can avoid context save by branching to this 
routine early.

INPUT:

OUTPUT:

FUNCTIONALITY:

We set up the stack pointer and call the correct function.


