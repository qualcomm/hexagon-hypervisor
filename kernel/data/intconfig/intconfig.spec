
MODULE: intconfig

Configure interrupts

FUNCTION: void BLASTK_intconfig_init()

Initializes the interrupt configuration data and fast interrupt contexts

INTPUTS:

OUTPUTS:

FUNCTIONALITY:

All handler pointers are initialized to NULL.  The fastint mask is initialized 
to zeros.  The handler for reschedule is initialized to BLASTK_resched.

Next, we initialize all fastint contexts to zeros.  We then update the hthread
fields to the correct hardware thread that will be using the fastint context, 
and the trapmask to exclude any trap may not be called by the fast interrupt
handler.


FUNCTION: void BLASTK_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), BLASTK_thread_context *me)

Modifies the interrupt configuration data to register a new fast interrupt handler.

INPUTS;

Argument 0: which interrupt to register
Argument 1: Address of the fast interrupt handler
Argument 2: Context of the calling thread




