
ASM_REF_CODE(Bootup code is difficult to write in C)

MODULE: boot

FUNCTION: void blast_init()

This function starts the kernel, if it has not been started already.

INPUTS:

OUTPUTS:

FUNCTIONALITY:

This function merely returns. However, it must be called from main from the
boot thread.  This forces the linker to include the object in the final
executable.



FUNCTION: start()

This routine boots the machine.

INPUTS:

OUTPUTS:

FUNCTIONALITY:

We assume that the machine is entirely off.

We initialize caches, etc.

We initialize all kernel functionality.

We do other stuff

Then we jump to symbol that should be in crt0

We need to include a reference to BLASTK_symbols to ensure that 
is also pulled in.


STRUCT: void *BLASTK_symbols[]

This data structure contains references to all functions
that are required in a BLAST kernel, to force the linker
to link them in.


