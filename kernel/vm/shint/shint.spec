:mod:`shint` -- Shared Interrupt Management
===========================================

.. module:: shint

H2K_vm_shint_post
-----------------

.. c:function:: s32_t H2K_vm_shint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo *info)

	:param vmblock: Pointer to the VM Memory Block
	:param dest: destination cpu for interrupt posting
	:param intno: Interrupt number to post
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interurpt is owned as a shared interrupt, this function posts a shared
virtual interrupt into the of a virtual machine.  The specified virtual CPU
is attempted to be interrupted, or other CPUs are tried if the specified one
can not take the interrupt.  If the interrupt cannot be handled, the next
interrupt post handler is called.

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

The interrupt is posted into the pending array.  If the global enable is set
and the per-cpu enable is set for the specified CPU, we attempt to take the
interrupt on the specified CPU.  Otherwise, we try other CPUs.


H2K_vm_shint_enable
-------------------

.. c:function:: s32_t H2K_vm_shint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: CPU for shared interrupt
	:param intno: Interrupt number to enable
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

This function globally enables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Set the global enable bit for intno.  If this virtual interrupt is mapped to a
physical interrupt in the vmblock, re-enable the physical interrupt via
:c:func:`H2K_intcontrol_enable()`.  If the interrupt is pending, try to deliver.


H2K_vm_shint_disable
--------------------

.. c:function:: s32_t H2K_vm_shint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: CPU for shared interrupt
	:param intno: Interrupt number to disable
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

This function globally disables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically clear the global enable bit for interrupt intno.  


H2K_vm_shint_localen
--------------------

.. c:function:: s32_t H2K_vm_shint_localen(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: Specified virtual CPU
	:param intno: Interrupt number to unmask
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, we enable the specified interrupt locally.  

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, we set the local enable bit for the specified interrupt and CPU.  If
an interrupt is pending and globally enabled, we attempt to take the interrupt.


H2K_vm_shint_localdis
---------------------

.. c:function:: s32_t H2K_vm_shint_localdis(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: destination cpu for interrupt local disable
	:param intno: Interrupt number to mask
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, we disable the specified interrupt locally.

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, clear the local enable bit for the specified interrupt and CPU.

H2K_vm_shint_setaffinity
------------------------

.. c:function:: void H2K_vm_shint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: destination cpu
	:param intno: Interrupt number
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, cause the specified interrupt to be steered to the specified cpu.


Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, we locally disable the interrupt for all CPUs, then locally enable
the interrupt for the specified CPU.



H2K_vm_shint_get
----------------

.. c:function:: s32_t H2K_vm_shint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param dest: destination cpu for interrupt posting
	:param offset: Offset of these interrupts
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

This function tries to take an interrupt pending in the virtual interrupt controller.

Functionality
~~~~~~~~~~~~~

Find the highest priority pending interrupt that is enabled.  If one is found,
take the interrupt and return the number plus the specified offset.  If no interrupt
is found, call the next handler.



H2K_vm_shint_peek
-----------------

.. c:function:: s32_t H2K_vm_shint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param offset: Offset of these interrupts
	:param intno: Interrupt number to post
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

This function checks to see if a valid interrupt is pending in the virtual interrupt controller.

Functionality
~~~~~~~~~~~~~

Find the highest priority pending interrupt that is enabled.  If one is found,
return the number plus the specified offset, but do not take the interrupt.  If
no interrupt is found, call the next handler.


H2K_vm_shint_status
-------------------

.. c:function:: s32_t H2K_vm_shint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int");

	:param vmblock: Pointer to the VM Memory Block
	:param dest: destination cpu for interrupt posting
	:param intno: Interrupt number to post
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

This function returns the status of the interrupt.

Functionality
~~~~~~~~~~~~~

If handled, return pending, local-enable, and global-enable as bits in the return value.



