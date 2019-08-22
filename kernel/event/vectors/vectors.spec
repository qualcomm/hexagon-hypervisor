
ASM_REF_CODE(Vectors cannot be written in C)

:mod:`vectors` -- Event Vector Table
====================================

.. module:: vectors

H2K_event_vectors
-----------------

.. c:function:: H2K_event_vectors()

Description
~~~~~~~~~~~

The event vectors are the entry points into the kernel.  Every exception, or
interrupt causes the program counter to change to an entry in the event
vectors.

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


Testing
-------


Samples
~~~~~~~

* Input: Event number.
* Flow: go to event handler for event number

Important cases
~~~~~~~~~~~~~~~

0. H2K_handle_reset
1. H2K_handle_nmi
2. H2K_handle_error
3. Invalid input.
4. H2K_handle_tlbmissx
5. Invalid input.
6. H2K_handle_tlbmissrw
7. Invalid input.
8. H2K_handle_trap0
9. H2K_handle_trap1
10. Invalid input.
11. Invalid input.
12. Invalid input.
13. Invalid input.
14. Invalid input.
15. Invalid input.

16-48 should jump to :c:func:`H2K_handle_int()`.

Harness
~~~~~~~

We define H2K_handle_reset, H2K_handle_nmi, H2K_handle_error,
H2K_handle_tlbmissx, H2K_handle_tlbmissrw, H2K_handle_trap0,
H2K_handle_trap1, and H2K_handle_int.  These functions set 
a flag when executed, and then return.  We link only with the 
object file.  We then iterate over the valid event numbers,
calling indirectly through a pointer.  After each call, we check
to make sure the correct handler was called.

