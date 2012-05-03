:mod:`vmevent` -- generate a VM event
=====================================

.. module:: vmevent


H2K_vm_event
------------

.. cfunction:: H2K_vm_event(u32_t gbadva, u32_t cause, u32_t vec_offset, H2K_thread_context *me)

	:param gbadva: Value to fill into the Guest Bad Virtual Address
	:param cause: Cause Code
	:param vec_offset: Offset of the Vector
	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Generates a virtual machine event.

Functionality
~~~~~~~~~~~~~

Sets GELR to the ELR value saved in the context.

Sets GBADVA to the specifed value.

Uses vec_offset, combined with GEVB to form the ELR value.

If the saved SSR has the guest bit set, the R29 and GOSP values are swapped in the context.

Uses cause and the SSR guest bit to form the GSSR value.

Save IE state in GSSR

Disable guest interrupts.

Sets the SSR guest bit.



