:mod:`badint` -- Invalid Interrupt Support
==========================================

.. module:: badint

H2K_vm_badint_func
------------------

.. cfunction:: s32_t H2K_vm_badint_func(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_intopinfo_t *info)

	:param vmblock: Pointer to the VM Block (unused)
	:param me: Pointer to the thread context (unused)
	:param intno: Interrupt number (unused)
	:param info: Information about this set of interrupts (unused)
	:returns: -1
	

Description
~~~~~~~~~~~

This function returns failure.

Functionality
~~~~~~~~~~~~~

We continue down the list of interupts until the interrupt is handled.  This module
always handles the interrupt as invalid.

