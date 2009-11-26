
MODULE: config

This module contains the runtime kernel configuration routines.

FUNCTION: void BLASTK_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, BLASTK_thread_context *me)

Trap handler for kernel configuration.

INPUTS:

Argument 0: Configuration operation to do
Argument 1: Operation-dependent pointer value
Argument 2: Operation-dependent value
Argument 3: Operation-dependent value
Argument 4: Pointer to the current thread context

OUTPUTS:


FUNCTION: void BLASTK_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, BLASTK_thread_context *me)

Adds additional memory for use as thread contexts.

INPUTS:

Argument 0: Unused parameter
Argument 1: Pointer for additional memory for use as thread contexts
Argument 2: Size of the data area in bytes
Argument 3: Unused parameter
Argument 4: Pointer to the current thread context

OUTPUTS:

FUNCTIONALITY:

First, the pointer should be adjusted so that it is 32-byte aligned.
This must be a rounding up.  Size must be decremented by any difference
in rounding.

Next, size should be rounded down to the nearest multiple of CONTEXT_SIZE.

Finally, the memory is chunked into size/CONTEXT_SIZE thread contexts.
Each context is cleared, and then inserted into the BLASTK_free_threads list.


TBD: remove threads?
TBD: keep list of thread areas for better threadids.


FUNCTION: void BLASTK_trap_config_schedint(u32_t unused, void *unused2, u32_t what_int, u32_t unused3, BLASTK_thread_context *me)

Changes the reschedule interrupt to the specified interrupt.

INPUTS:

Argument 0: Unused parameter
Argument 1: Unused parameter
Argument 2: Which interrupt should be used as the reschedule interrupt
Argument 3: Unused parameter
Argument 4: Pointer to the current thread context

OUTPUTS:

FUNCTIONALITY:

If any interrupt is already configured as schedint, change it to fastint

Then, change interrupt pointer at what_int to point to reschedule_from_lowprio.



