:mod:`vmtrap` -- Virtual machine trap dispatch
================================================

.. module:: vmtrap

The VM trap entry handles ``trap1`` requests from guest mode.  It saves the
trap context, selects a handler from ``H2K_vmtraptab`` using the low five bits of
the saved SSR, and transfers control to that handler.  VM trap handlers receive
the current thread context in their context argument and return through the
continuation saved by the entry code.


H2K_handle_trap1
----------------

.. c:function:: void H2K_handle_trap1(void)

This is the assembly entry point for a VM ``trap1`` request.  It swaps and saves
the processor context, checks the saved SSR guest-mode bit, and branches to
:c:func:`H2K_vmtrap_bad()` when the request did not originate in guest mode.
For a guest request, it uses the low five bits of the saved SSR to select an
entry in :c:data:`H2K_vmtraptab`, saves the return continuation in the thread
context, passes the context to the selected handler, and jumps to that handler.

The optimized and reference implementations may save different sets of
registers, but both provide the same dispatch behavior and continuation
contract.


H2K_vmtraptab
-------------

.. c:data:: H2K_vmtraptab

``H2K_vmtraptab`` is an assembly jump table indexed by the low five bits of the
saved SSR.  Each entry is one instruction and transfers control to a VM trap
handler.  The table contains these entries:

* ``0x00`` — :c:func:`H2K_vmtrap_version()`
* ``0x01`` — :c:func:`H2K_vmtrap_return()`
* ``0x02`` — :c:func:`H2K_vmtrap_setvec()`
* ``0x03`` — :c:func:`H2K_vmtrap_setie()`
* ``0x04`` — :c:func:`H2K_vmtrap_getie()`
* ``0x05`` — :c:func:`H2K_vmtrap_intop()`
* ``0x06``–``0x09`` — :c:func:`H2K_vmtrap_bad()`
* ``0x0A`` — :c:func:`H2K_vmtrap_clrmap()`
* ``0x0B`` — :c:func:`H2K_vmtrap_newmap()`
* ``0x0C`` — :c:func:`H2K_vmtrap_bad()`
* ``0x0D`` — :c:func:`H2K_vmtrap_cachectl()`
* ``0x0E`` — :c:func:`H2K_vmtrap_get_pcycles()`
* ``0x0F`` — :c:func:`H2K_vmtrap_set_pcycles()`
* ``0x10`` — :c:func:`H2K_vmtrap_wait()`
* ``0x11`` — :c:func:`H2K_vmtrap_yield()`
* ``0x12`` — :c:func:`H2K_vmtrap_start()`
* ``0x13`` — :c:func:`H2K_vmtrap_stop()`
* ``0x14`` — :c:func:`H2K_vmtrap_vmpid()`
* ``0x15`` — :c:func:`H2K_vmtrap_setregs()`
* ``0x16`` — :c:func:`H2K_vmtrap_getregs()`
* ``0x17`` — :c:func:`H2K_vmtrap_bad()`
* ``0x18`` — :c:func:`H2K_vmtrap_timer()`
* ``0x19`` — :c:func:`H2K_vmtrap_pmuctrl()`
* ``0x1A`` — :c:func:`H2K_vmtrap_info()`
* ``0x1B``–``0x1F`` — :c:func:`H2K_vmtrap_bad()`

The handlers listed above are implemented and documented in their respective
modules.  The bad entries are deliberate reserved or invalid VM trap slots.


H2K_vmtrap_bad
--------------

.. c:function:: void H2K_vmtrap_bad(void)

Handles a VM trap that is not permitted or whose table slot is invalid.  It
calls :c:func:`H2K_vm_event()` with the current thread context, bad virtual
address ``0``, error code ``0x1B`` (no guest permission), and
``ERROR_GEVB_OFFSET``.  The event is therefore reported through the guest
error-event vector rather than returning as a successful VM operation.


Testing
-------

Verify that a trap from guest mode reaches the handler selected by each table
slot, that a trap from non-guest mode reaches :c:func:`H2K_vmtrap_bad()`, and
that the handler returns through the continuation saved by
:c:func:`H2K_handle_trap1()`.  Reserved table entries should produce the same
bad-trap event as a non-guest request.
