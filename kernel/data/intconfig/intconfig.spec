
MODULE: intconfig

Configure interrupts

FUNCTION: void BLASTK_intconfig_init()

Initializes the interrupt configuration data

INTPUTS:

OUTPUTS:



FUNCTION: void BLASTK_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), BLASTK_thread_context *me)

Modifies the interrupt configuration data to register a new fast interrupt handler.

INPUTS;

Argument 0: which interrupt to register
Argument 1: Address of the fast interrupt handler
Argument 2: Context of the calling thread



