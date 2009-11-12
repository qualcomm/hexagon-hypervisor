
MODULE: offsets

This module is for use at *BUILD TIME*.

DESCRIPTION:

This module creates includes for assembly files that define the structure
offsets for structures defined in C, ``asm_offsets.h``.  This helps the
assembly remain correct as structure definitions change.

Additionally, we define the total size of the context structure.

FUNCTIONALITY:

We use the ``offsetof()`` macro in C to generate the byte offsets to elements 
of the data structure.  These offsets are printed out with the corresponding
``#define *offset_name*`` information to make the syntax valid.  

The makefile will run the binary, and use the standard out to create the
asm_offsets.h file for use in assembly parts of the kernel.

