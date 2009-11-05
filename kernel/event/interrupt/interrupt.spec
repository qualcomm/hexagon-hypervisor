
ASM_REF_CODE(Interrupt context save/restore not possible in C)

UNIT: interrupt

FUNCTION: handle_int()

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

Real interrupted threads (as opposed to, for example, the idle thread) that may 
reschedule should have int_context_restore as the end of their continuation
function.

FUNCTION: int_context_restore()

DESCRIPTION:

This routine returns to an interrupted thread.  It restores the registers from
the thread context and returns to the thread in the appropriate place.

INPUT:

OUTPUT:

FUNCTIONALITY:

The int_context_restore routine restores registers from the thread context and returns.

