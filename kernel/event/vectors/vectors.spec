
ASM_REF_CODE(Vectors cannot be written in C)

:mod: `vectors` -- Event Vector Table

..cfunction:: H2K_event_vectors()

Description
~~~~~~~~~~~

The event vectors are the entry points into the kernel.  Every exception, or
interrupt causes the program counter to change to an entry in the event
vectors.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The event vectors are primarily a series of jump instructions.  The machine will
set the PC to the appropriate address for the particular event being serviced.
The jump instruction at this PC redirects to the appropriate handler or trampoline.

Certain events have one or more reserved spaces following them.  These can be
used for pulling up an additional instruction in V3 and later architectures.

The vector offsets are specified by the Hexagon System Architecture Spec.

For V2 implementations, six interrupts are reserved for changing the priority.
These vector to special handlers, rather than using the common interrupt 
handler path.

