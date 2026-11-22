:mod:`trace` -- Hardware trace emission
==========================================

.. module:: trace

The current trace implementation emits a 32-bit hardware trace word.  The
word is represented by ``H2K_trace_entry_t``:

* ``what``: 8-bit trace type in bits 0-7
* ``info``: 24-bit trace information in bits 8-31

The current reference implementation does not encode ``pcyclelo`` or
``hwtnum`` into the word; those arguments are retained by the interface for
compatibility with other implementations.


H2K_trace
---------

.. c:function:: void H2K_trace(s8_t type, u32_t info, u32_t pcyclelo, u8_t hwtnum) IN_SECTION(".text.misc.trace")

	:param type: Trace type stored in the 8-bit ``what`` field
	:param info: Trace information; the low 24 bits are stored
	:param pcyclelo: Cycle value retained for interface compatibility
	:param hwtnum: Hardware-thread number retained for interface compatibility

Builds a ``H2K_trace_entry_t`` from ``type`` and ``info`` and emits its raw word
through :c:func:`H2K_hw_trace()`.  ``pcyclelo`` and ``hwtnum`` are not read by
the reference implementation.

Trace constants include ``H2K_TRACE_SWITCH``,
``H2K_TRACE_SWITCH_FROM_SLEEP``, ``H2K_TRACE_SWITCH_TO_SLEEP``,
``H2K_TRACE_INTERRUPT``, ``H2K_TRACE_FATAL_THREAD``, and
``H2K_TRACE_FATAL_KERNEL``.


H2K_trace_init
--------------

.. c:function:: void H2K_trace_init(void) IN_SECTION(".text.init.trace")

The current reference implementation performs no initialization.


ASM_INLINE_TRACE
----------------

.. c:macro:: ASM_INLINE_TRACE(MSG, HTNUM_PCYCLELO, HTNUM, PCYCLELO, ENTRIES_BUFADDR, ENTRIES, BUFADDR, PRED, OLDIDX, NEWIDX, IDXADDR, EXTRAINSNS0, EXTRAINSNS1)

This assembly-only macro emits ``trace(MSG)`` in the active implementation.
The remaining arguments are retained for compatibility with the older inline
trace-buffer implementation, which allocated a trace-buffer slot with an
atomic index and stored a hardware-thread/pcycle pair.  The macro may only be
included from assembly because ``trace_asm_macro.h`` requires ``ASM``.
