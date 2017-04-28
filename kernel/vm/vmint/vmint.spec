:mod:`vmint` -- Virtual Machine Interrupt Management
====================================================

.. module:: vmint


H2K_vm_interrupt_post
---------------------

.. c:function:: void H2K_vm_interrupt_post(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param first_cpu: Pointer to the first CPU to try
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function posts a virtual interrupt into a virtual machine.  The CPU
"first_cpu" will attempt to be notified of the interrupt, if possible.
Otherwise, some other virtual CPU that can take the interrupt will be notified.

Functionality
~~~~~~~~~~~~~

The bit corresponding to intno is set in the virtual ipend.  If the interrupt
is globally enabled, we start looking for a CPU with the interrupt locally 
enabled.  When we find one, we note that the CPU needs VM work, and we try
to deliver an IPI to the CPU, if currently scheduled and interruptible.


H2K_vm_interrupt_enable
-----------------------

.. c:function:: void H2K_vm_interrupt_enable(H2K_vmblock_t *vmblock, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function globally enables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically set the global enable mask for intno.  
If the interrupt is pending, try to deliver.


H2K_vm_interrupt_disable
------------------------

.. c:function:: void H2K_vm_interrupt_disable(H2K_vmblock_t *vmblock, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function globally disables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically clear the enable bit for interrupt intno.  


H2K_vm_interrupt_localunmask
----------------------------

.. c:function:: void H2K_vm_interrupt_localunmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param cpu: CPU to unmask
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function locally enables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically set the local enable bit for interrupt intno.  


H2K_vm_interrupt_localmask
--------------------------

.. c:function:: void H2K_vm_interrupt_localmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param cpu: CPU to mask
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function locally disables the interrupt intno.

Functionality
~~~~~~~~~~~~~

Atomically clear the local enable bit for interrupt intno.  

H2K_vm_interrupt_setaffinity
----------------------------

.. c:function:: void H2K_vm_interrupt_setaffinity(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno)

	:param vmblock: Pointer to the VM Memory Block
	:param cpu: CPU to receive interrupt
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function locally disables the interrupt intno for all CPUs except the CPU specified by the `cpu` argument.

Functionality
~~~~~~~~~~~~~

Atomically clear the local enable bit for interrupt intno for all CPUs, except
CPU `cpu`, which is enabled.  


H2K_vm_interrupt_get
--------------------

.. c:function:: s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, u8_t cpu)

	:param vmblock: Pointer to the VM Memory Block
	:param cpu: CPU to receive interrupt
	:param intno: Interrupt number to post

Description
~~~~~~~~~~~

This function tries to take an interrupt pending in the virtual interrupt controller.

Functionality
~~~~~~~~~~~~~

Find the highest priority pending interrupt that is enabled both globally and
locally.  If one is found, it is returned.  If no pending interrupt can be
taken, a negative value is returned.


H2K_vmtrap_intop
----------------

.. c:function:: void H2K_vmtrap_intop(H2K_thread_context *me)

	:param me: pointer to the thread context


Description
~~~~~~~~~~~

Handles operations on the virtual interrupt controller.

Functionality
~~~~~~~~~~~~~

First, parameters are fixed up to appropriate values.  We then call the appropriate
operation from the optab in the first handling information block.


