:mod:`vmdefs` -- VM compile-time definitions
===============================================

.. module:: vmdefs

These definitions are shared by guest C code and VM assembly.  They define the
VM ABI version, guest event-vector offsets, guest status bits, and asynchronous
VCPU state bits.


VM version and guest event vectors
----------------------------------

.. c:macro:: H2K_VM_VERSION

Supported VM ABI version, currently ``0x00000800``.

The ``*_GEVB_OFFSET`` constants select entries relative to the guest event
vector base:

* ``RESET_GEVB_OFFSET`` — reset event;
* ``CHECK_GEVB_OFFSET`` — check event;
* ``ERROR_GEVB_OFFSET`` — error event;
* ``DEBUG_GEVB_OFFSET`` — debug event;
* ``TRAP0_GEVB_OFFSET`` — trap0 event;
* ``INTERRUPT_GEVB_OFFSET`` — virtual interrupt event.


Guest SSR bits
--------------

``H2K_GSSR_UM`` is the guest user-mode bit, ``H2K_GSSR_IE`` is the guest
interrupt-enable bit, and ``H2K_GSSR_SS`` is the guest single-step bit.  The
corresponding ``*_BIT`` macros identify bit positions 31, 30, and 29.


VCPU status bits
----------------

``H2K_VMSTATUS_VMWORK`` (bit 0) tells a VCPU to reconsider deferred VM work
before blocking.  ``H2K_VMSTATUS_KILL`` (bit 1) marks it for termination;
killers set KILL and VMWORK together so the VCPU reaches
:c:func:`H2K_vm_do_work_withlock()`.

``H2K_VMSTATUS_SAVEXT`` (bit 6) says that extended registers are live and must
be saved/restored.  ``H2K_VMSTATUS_IE`` (bit 7) mirrors whether guest
interrupts are enabled.

The ``*_BIT`` and value macros are both provided for each status field.  VMWORK
is a deferred-work signal rather than the work itself; substantive state is
kept in the interrupt controller or KILL bit.  Clearing VMWORK means that the
pending causes have been considered, not necessarily that a masked interrupt
was delivered.


VM block flags
--------------

``H2K_VMBLOCK_FLAGS_USE_EXT_BIT`` and
``H2K_VMBLOCK_FLAGS_DO_EXT_BIT`` identify VM-block extended-register behavior.
