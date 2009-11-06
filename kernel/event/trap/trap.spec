
ASM_REF_CODE(Trap context save/restore not possible in C)

UNIT: interrupt

FUNCTION: handle_trap0()

DESCRIPTION:

The trap0 instruction goes to the trap0 vector, which jumps to this routine.

The handle_trap0 function saves callee-save registers according to the ABI,
including special registers.  It also determines the functionality requested
by the trap, and calls the corresponding kernel routine.

INPUT:

OUTPUT:

FUNCTIONALITY:

The BLAST Trap API specifies that only the callee-save registers be saved by the
kernel during a trap0.  This allows the kernel to immediately use scratch registers
for saving context and determining the correct behavior for the trap.

In simulation environments, and in environments attached to T32, the program
can use special semihosting routines to request that functionalities like
printing to the console and file I/O be emulated by the simulation or debug
environment.  Semihosting requests use the trap0 immediate 0.  This should be
detected early, and should branch to a label that can be used for detecting the 
semihosting request.  After the branch, we can return immediately.  Note that trap0(#0)
should have no effect (other than clobbered registers) for a node not connected to 
an appropriate semihosting provider.

We then check to ensure that the thread can use the requested trap.  This is used to
reduce capabilities of less-priviledged threads, and is also used to assert that fast
interrupt handlers cannot block.

We then jump to the handler for the requested trap.  This is done via the
traptab, which is a table of pairs of instructions.  One instruction of the
pair is the jump instruction to the appropriate place, and the other
instruction in the pair transfers the pointer to the current thread context to
the appropriate argument register, as this is an argument for most functions.

The trap request can return, or can call the trap continuation.



