:mod:`cpuint` -- Per-CPU Interrupt Management
=============================================

.. module:: cpuint

H2K_vm_cpuint_post
------------------

.. c:function:: s32_t H2K_vm_cpuint_post(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo *info)

	:param vmblock: Pointer to the VM Memory Block
	:param dest: destination cpu for interrupt posting
	:param intno: Interrupt number to post
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interurpt is owned as a per-cpu interrupt, this function posts a per-cpu
virtual interrupt into the specified cpu of a virtual machine.  If the
interrupt cannot be handled, the next interrupt post handler is called.

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

The interrupt is posted in the thread context.  If the vCPU can take the interrupt,
we try and deliver the interrupt to the virtual CPU.


H2K_vm_cpuint_enable
--------------------

.. c:function:: s32_t H2K_vm_cpuint_enable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: CPU for per-cpu interrupt
	:param intno: Interrupt number to enable
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

This function globally enables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Set the enable mask for intno.  If this virtual interrupt is mapped to a
physical interrupt in the vmblock, re-enable the physical interrupt via
:c:func:`H2K_intcontrol_enable()`.  If the interrupt is pending, try to deliver.


H2K_vm_cpuint_disable
---------------------

.. c:function:: s32_t H2K_vm_cpuint_disable(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: CPU for per-cpu interrupt
	:param intno: Interrupt number to disable
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

This function globally disables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically clear the enable bit for interrupt intno.  


H2K_vm_cpuint_localunmask
-------------------------

.. c:function:: s32_t H2K_vm_cpuint_localunmask(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: Specified virtual CPU
	:param intno: Interrupt number to unmask
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1


H2K_vm_cpuint_localmask
-----------------------

.. c:function:: s32_t H2K_vm_cpuint_localmask(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: destination cpu for interrupt posting
	:param intno: Interrupt number to mask
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1

Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1

H2K_vm_cpuint_setaffinity
-------------------------

.. c:function:: void H2K_vm_cpuint_setaffinity(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info)

	:param vmblock: Pointer to the VM Memory Block
	:param me: destination cpu
	:param intno: Interrupt number
	:param info: Interrupt Info for this interrupt type
	:returns: -1 on failure, 0 otherwise

Description
~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1


Functionality
~~~~~~~~~~~~~

If the interrupt is not handled in this way, we choose the next interrupt type.

Otherwise, return -1



H2K_vm_cpuint_get
-----------------

.. c:function:: s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

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



H2K_vm_cpuint_peek
------------------

.. c:function:: s32_t H2K_vm_cpuint_get(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset, H2K_vm_int_opinfo_t *info)

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


H2K_vm_cpuint_status
--------------------

.. c:function:: s32_t H2K_vm_cpuint_status(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno, H2K_vm_int_opinfo_t *info) IN_SECTION(".text.vm.int");

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

If handled, return pending and enabled as bits in the return value.



