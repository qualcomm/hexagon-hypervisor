:mod:`shint` -- Shared virtual interrupt management
====================================================

.. module:: shint

The shared interrupt handler stores pending and globally enabled state in the VM
block.  It also stores a local-enable mask for each virtual CPU.  Operations
whose interrupt number is outside this handler's range pass the request to the
next interrupt handler using ``info->num_ints`` as the range size.


H2K_vm_shint_post
-----------------

.. c:function:: s32_t H2K_vm_shint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param dest: Virtual CPU used as the initial delivery context
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Sets the interrupt's pending bit unless it is already pending.  If the
interrupt is globally enabled, tries to deliver it to a virtual CPU whose
local-enable mask contains the interrupt.  Waiting CPUs are considered first;
if none can take it, all virtual CPUs are tried in circular order starting at
``dest``.  Interrupts outside this handler's range are passed to the next
handler.


H2K_vm_shint_clear
------------------

.. c:function:: s32_t H2K_vm_shint_clear(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Current virtual CPU; unused by this operation
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Atomically clears the interrupt's pending bit.  Interrupts outside this
handler's range are passed to the next handler.


H2K_vm_shint_enable
-------------------

.. c:function:: s32_t H2K_vm_shint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU used for delivery if the interrupt is pending
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Atomically sets the global-enable bit.  If the interrupt is mapped to a
physical interrupt, re-enables that physical interrupt.  If the interrupt is
already pending, attempts delivery using ``me`` as the initial delivery
context.  Interrupts outside this handler's range are passed to the next
handler.


H2K_vm_shint_disable
--------------------

.. c:function:: s32_t H2K_vm_shint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Current virtual CPU; unused by this operation
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Atomically clears the global-enable bit.  Interrupts outside this handler's
range are passed to the next handler.


H2K_vm_shint_localen
--------------------

.. c:function:: s32_t H2K_vm_shint_localen(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU whose local mask is changed
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Atomically sets the local-enable bit for ``intno`` on ``me``'s virtual CPU.  If
the interrupt is both pending and globally enabled, attempts delivery to that
CPU.  Interrupts outside this handler's range are passed to the next handler.


H2K_vm_shint_localdis
---------------------

.. c:function:: s32_t H2K_vm_shint_localdis(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU whose local mask is changed
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Atomically clears the local-enable bit for ``intno`` on ``me``'s virtual CPU.
Interrupts outside this handler's range are passed to the next handler.


H2K_vm_shint_setaffinity
------------------------

.. c:function:: s32_t H2K_vm_shint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU to which the interrupt is steered
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: 0

Clears the local-enable bit for the interrupt on every virtual CPU, then sets
it on ``me``'s virtual CPU.  Interrupts outside this handler's range are passed
to the next handler.


H2K_vm_shint_get
----------------

.. c:function:: s32_t H2K_vm_shint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU whose local mask is examined
	:param offset: Interrupt-number offset
	:param info: Interrupt-operation information
	:returns: ``offset + interrupt_number`` for the selected interrupt, or the chained handler result

Scans the shared interrupt range in ascending order for an interrupt that is
pending, globally enabled, and locally enabled for ``me``.  Atomically clears
its global-enable and pending bits before returning its number plus ``offset``.
If no interrupt is available, calls the next handler with ``offset`` increased
by ``vmblock->num_ints``.


H2K_vm_shint_peek
-----------------

.. c:function:: s32_t H2K_vm_shint_peek(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU whose local mask is examined
	:param offset: Interrupt-number offset
	:param info: Interrupt-operation information
	:returns: ``offset + interrupt_number`` for the lowest-numbered eligible interrupt, or the chained handler result

Scans the shared interrupt range in ascending order for an interrupt that is
pending, globally enabled, and locally enabled for ``me``.  Returns its number
plus ``offset`` without changing interrupt state.  If no interrupt is
available, calls the next handler with ``offset`` increased by
``vmblock->num_ints``.


H2K_vm_shint_status
-------------------

.. c:function:: s32_t H2K_vm_shint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:param vmblock: VM memory block
	:param me: Virtual CPU whose local mask is examined
	:param intno: Shared interrupt number
	:param info: Interrupt-operation information
	:returns: Bit 0 for pending, bit 1 for local-enable, and bit 2 for global-enable

Returns the interrupt status as ``pending | (local_enable << 1) |
(global_enable << 2)``.  Interrupts outside this handler's range are passed to
the next handler.


H2K_vm_shint_nop
----------------

.. c:function:: s32_t H2K_vm_shint_nop(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int")

	:returns: 0

Performs no operation.
