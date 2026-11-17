:mod:`vmint` -- Virtual machine interrupt management
=====================================================

.. module:: vmint

The VM interrupt layer dispatches interrupt operations to the handlers in a
VM's interrupt-operation table.  It also delivers selected interrupts to VCPU
contexts and maintains the guest interrupt-enable state.


H2K_vm_int_deliver_locked
-------------------------

.. c:function:: s32_t H2K_vm_int_deliver_locked(H2K_vmblock_t *vmblock, H2K_thread_context *thread, u32_t intno) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param thread: VCPU context to notify
	:param intno: Interrupt number to deliver
	:returns: 0 when delivery is handled, or -1 when the VCPU is dead or invalid

Delivers an interrupt while the caller holds the BKL.  A VCPU in ``VMWAIT`` is
removed from the waiting mask when tracked, receives ``intno`` in ``r00``, and
is appended to the ready list.  A running VCPU with guest interrupts enabled
receives an IPI.  An interrupt-blocked or futex-blocked VCPU with interrupts
enabled is cancelled from that blocking state and made ready.  Ready VCPUs
require no additional action.  Delivery to a VCPU whose interrupts are
disabled is not performed and returns 0.


H2K_vm_int_deliver
------------------

.. c:function:: s32_t H2K_vm_int_deliver(H2K_vmblock_t *vmblock, H2K_thread_context *thread, u32_t intno) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param thread: VCPU context to notify
	:param intno: Interrupt number to deliver
	:returns: Result from :c:func:`H2K_vm_int_deliver_locked()`

Acquires the BKL, calls :c:func:`H2K_vm_int_deliver_locked()`, and releases the
BKL before returning its result.


H2K_vm_interrupt_peek
---------------------

.. c:function:: static s32_t H2K_vm_interrupt_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me)

	:param vmblock: VM memory block
	:param me: VCPU context to inspect
	:returns: Next pending enabled interrupt number, or ``-1`` when none is available

Calls the first interrupt handler's ``peek`` operation with an offset of zero.
This is the non-consuming path used when guest interrupts are disabled.


H2K_vm_interrupt_get
--------------------

.. c:function:: static s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, H2K_thread_context *me)

	:param vmblock: VM memory block
	:param me: VCPU context to inspect
	:returns: Next pending enabled interrupt number, or ``-1`` when none is available

Calls the first interrupt handler's ``get`` operation with an offset of zero.
The selected interrupt is consumed according to the active interrupt handler.


H2K_vmint_fixup_default
-----------------------

.. c:function:: static H2K_thread_context *H2K_vmint_fixup_default(H2K_thread_context *me, u32_t *argptr)

	:param me: Current VCPU context
	:param argptr: Operation argument pointer; unchanged
	:returns: ``me``

Uses the current VCPU and leaves the operation argument unchanged.  This fixup
is used by operations that do not require a different target context.


H2K_vmint_fixup_getpeek
-----------------------

.. c:function:: static H2K_thread_context *H2K_vmint_fixup_getpeek(H2K_thread_context *me, u32_t *argptr)

	:param me: Current VCPU context
	:param argptr: Operation argument pointer
	:returns: ``me``

Uses the current VCPU and sets the operation argument to zero.  This is used by
``get`` and ``peek`` before their handlers are called.


H2K_vmint_fixup_post
--------------------

.. c:function:: static H2K_thread_context *H2K_vmint_fixup_post(H2K_thread_context *me, u32_t *argptr)

	:param me: Current VCPU context, possibly NULL for a fast interrupt caller
	:param argptr: Operation argument pointer; unchanged
	:returns: Target VCPU context, or ``NULL`` for an invalid or unauthorized target

Selects the VCPU targeted by ``me->r02`` for a post operation.  A zero target
ID means the current VCPU.  For a nonzero target ID, validates the VM ID,
looks up its VM block and VCPU context, and permits only the same VM, its parent,
or its child relationship.  The selected target context is returned; an
invalid VM or unauthorized relationship returns ``NULL``.


H2K_vmint_fixup_aff
-------------------

.. c:function:: static H2K_thread_context *H2K_vmint_fixup_aff(H2K_thread_context *me, u32_t *argptr)

	:param me: Current VCPU context
	:param argptr: Operation argument pointer; unchanged
	:returns: VCPU context for the CPU index in ``me->r02``

Selects the VCPU in the current VM block whose CPU index is supplied in
``me->r02``.  The resulting context is used for the affinity operation.


H2K_vmtrap_intop
----------------

.. c:function:: void H2K_vmtrap_intop(H2K_thread_context *me) IN_SECTION(".text.vm.int")

	:param me: Current VCPU context

Dispatches the VM interrupt operation selected by ``me->r00``.  The supported
operation indexes are ``nop``, ``enable``, ``disable``, ``localen``,
``localdis``, ``setaffinity``, ``get``, ``peek``, ``status``, ``post``, and
``clear``.

The operation argument normally comes from ``me->r01``.  ``get`` and ``peek``
use an argument of zero.  ``post`` uses ``me->r02`` as a target VCPU ID when it
is nonzero, or targets the current VCPU when it is zero.  ``setaffinity`` uses
``me->r02`` as a VCPU index.  The operation is called with the selected VCPU's
VM block and context.  Its result is returned in ``me->r00``; invalid operation
indexes and invalid or inaccessible target IDs return ``-1``.


H2K_enable_guest_interrupts
----------------------------

.. c:function:: u32_t H2K_enable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int")

	:param me: VCPU context whose interrupts are enabled
	:returns: Previous guest interrupt-enable state

Atomically sets the guest interrupt-enable bit.  When interrupts transition
from disabled to enabled, checks for a pending interrupt and processes it.


H2K_disable_guest_interrupts
-----------------------------

.. c:function:: u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int")

	:param me: VCPU context whose interrupts are disabled
	:returns: Previous guest interrupt-enable state

Atomically clears the guest interrupt-enable bit.


H2K_vm_check_interrupts
-----------------------

.. c:function:: s32_t H2K_vm_check_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int")

	:param me: VCPU context to inspect
	:returns: Selected interrupt number, or ``-1`` when none is available

When guest interrupts are enabled, obtains the next pending enabled interrupt
from the VM interrupt controller.  If one is found, removes it from the
controller and raises it through :c:func:`H2K_vm_event()` using
``INTERRUPT_GEVB_OFFSET``.  When guest interrupts are disabled, only peeks at
the next pending enabled interrupt and leaves it pending.


H2K_vm_int_intinfo_init
-----------------------

.. c:function:: void H2K_vm_int_intinfo_init(H2K_vmblock_t *vmblock, u32_t num_ints) IN_SECTION(".text.config.config")

	:param vmblock: VM memory block to initialize
	:param num_ints: Number of shared interrupts; zero disables the shared-interrupt handler

Initializes the VM's interrupt-handler chain.  The first handler always handles
``PERCPU_INTERRUPTS`` per-CPU interrupts.  When ``num_ints`` is nonzero, a
shared-interrupt handler for that many interrupts is added after it.  A final
invalid-interrupt handler terminates the chain.
