:mod:`std` -- C and assembly environment definitions
========================================================

.. module:: std

The standard utility headers provide the fixed-width aliases, compiler helpers,
and assembly section macros used throughout the kernel.


C definitions
-------------

``c_std.h`` defines ``NULL``, ``ARCHV`` fallback detection, and these aliases:

* ``u8_t``, ``u16_t``, ``u32_t``, ``u64_t`` — unsigned 8-, 16-, 32-, and 64-bit types
* ``s8_t``, ``s16_t``, ``s32_t``, ``s64_t`` — signed 8-, 16-, 32-, and 64-bit types
* ``pa_t`` — physical address type, aliased to ``u64_t``

It also defines ``likely`` and ``unlikely`` compiler branch hints,
``IN_SECTION`` for placing declarations in a named section, ``offsetof``,
``member_size``, ``containerof``, ``bits``, ``max``, ``min``, and
``__barrier``.  ``H2K_LANG_IS_C`` is defined as 1.

``containerof(type, element, ptr)`` computes the containing object from a
pointer to one of its members.  ``__barrier()`` is a compiler memory barrier;
it does not emit a processor synchronization instruction.


Assembly definitions
---------------------

``asm_std.h`` defines ``ASM``, ``H2K_GP`` as ``R28``, ``NULL`` as zero, and
``H2K_LANG_IS_ASM`` as 1.  It provides:

* ``FUNC_START`` / ``FUNC_END`` — declare and size global assembly functions;
* ``DATA_START`` / ``DATA_END`` — declare and size global assembly objects;
* ``ASM_TICKET_LOCK`` / ``ASM_TICKET_UNLOCK`` — acquire and release the
  software ticket BKL when ``SOFT_BKL`` is enabled;
* ``ASM_BKL_LOCK`` / ``ASM_BKL_UNLOCK`` — select the software ticket lock or
  hardware K0 lock according to ``SOFT_BKL``.

The ticket-lock macros use the upper 16 bits as the next-ticket counter and the
lower 16 bits as the serving-ticket counter.  Their scratch registers and
predicate arguments are clobbered as specified by the macro invocation.
