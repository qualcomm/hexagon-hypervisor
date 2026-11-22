:mod:`cpuint` -- Per-CPU virtual interrupt management
========================================================

.. module:: cpuint

The per-CPU interrupt handler manages the interrupt state stored in a virtual
CPU's thread context.  For interrupt numbers at or above ``PERCPU_INTERRUPTS``,
operations that support chaining invoke the next interrupt handler with the
number adjusted by ``PERCPU_INTERRUPTS``.


H2K_vm_cpuint_post
------------------

.. c:function:: s32_t H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param dest: Destination virtual CPU
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Sets the interrupt pending bit and the VM-work bit for ``dest``.  If the
interrupt is enabled, attempts to deliver it to the virtual CPU.  A delivery
failure for a dead virtual CPU is ignored.  Interrupts outside the per-CPU
range are passed to the next handler.


H2K_vm_cpuint_post_locked
-------------------------

.. c:function:: s32_t H2K_vm_cpuint_post_locked(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param dest: Destination virtual CPU
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Performs the same pending-state updates as
:c:func:`H2K_vm_cpuint_post`, but uses the locked interrupt-delivery path.
The caller must already hold the lock required by that path.  This handler does
not chain interrupt numbers outside the per-CPU range.


H2K_vm_cpuint_clear
-------------------

.. c:function:: s32_t H2K_vm_cpuint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU whose interrupt is cleared
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Clears the pending bit for the per-CPU interrupt.  Interrupts outside the
per-CPU range are passed to the next handler.


H2K_vm_cpuint_enable
--------------------

.. c:function:: s32_t H2K_vm_cpuint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU whose interrupt is enabled
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Enables the per-CPU interrupt.  If the interrupt is mapped to a physical
interrupt, re-enables that physical interrupt.  A pending interrupt sets the
VM-work bit.  Interrupts outside the per-CPU range are passed to the next
handler.


H2K_vm_cpuint_disable
---------------------

.. c:function:: s32_t H2K_vm_cpuint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU whose interrupt is disabled
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Disables the per-CPU interrupt by clearing its enable bit.  Interrupts outside
the per-CPU range are passed to the next handler.


H2K_vm_cpuint_localen
---------------------

.. c:function:: s32_t H2K_vm_cpuint_localen(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: Chained handler result, or -1 for a per-CPU interrupt

Passes non-per-CPU interrupts to the next handler.  Local enable is not
supported for per-CPU interrupts.


H2K_vm_cpuint_localdis
----------------------

.. c:function:: s32_t H2K_vm_cpuint_localdis(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: Chained handler result, or -1 for a per-CPU interrupt

Passes non-per-CPU interrupts to the next handler.  Local disable is not
supported for per-CPU interrupts.


H2K_vm_cpuint_setaffinity
-------------------------

.. c:function:: s32_t H2K_vm_cpuint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: Chained handler result, or -1 for a per-CPU interrupt

Passes non-per-CPU interrupts to the next handler.  Affinity changes are not
supported for per-CPU interrupts.


H2K_vm_cpuint_get
-----------------

.. c:function:: s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU to inspect
	:param offset: Interrupt-number offset
	:param info: Interrupt-operation information
	:returns: ``offset + interrupt_number`` for a pending enabled interrupt, or the chained handler result

Finds the lowest-numbered pending interrupt that is enabled.  Atomically clears
its pending and enabled-pending state before returning its number plus
``offset``.  If none is available, passes the request to the next handler with
``offset`` increased by ``PERCPU_INTERRUPTS``.


H2K_vm_cpuint_peek
------------------

.. c:function:: s32_t H2K_vm_cpuint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

	:param vmblock: VM memory block
	:param me: Virtual CPU to inspect
	:param offset: Interrupt-number offset
	:param info: Interrupt-operation information
	:returns: ``offset + interrupt_number`` for a pending enabled interrupt, or the chained handler result

Finds the lowest-numbered pending interrupt that is enabled without clearing
it.  If none is available, passes the request to the next handler with
``offset`` increased by ``PERCPU_INTERRUPTS``.


H2K_vm_cpuint_status
--------------------

.. c:function:: s32_t H2K_vm_cpuint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU to inspect
	:param intno: Interrupt number
	:param info: Interrupt-operation information
	:returns: Bit 0 set when pending and bit 2 set when enabled, or the chained handler result

Returns the per-CPU interrupt status as ``pending | (enabled << 2)``.
Interrupts outside the per-CPU range are passed to the next handler.


H2K_vm_cpuint_nop
-----------------

.. c:function:: s32_t H2K_vm_cpuint_nop(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:returns: 0

Performs no operation.
