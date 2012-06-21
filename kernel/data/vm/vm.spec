:mod:`vm` -- vmblock functions and offset translations
======================================================

.. module:: vm

H2K_vmblock_clear
-----------------

.. cfunction:: void H2K_vmblock_clear(H2K_vmblock_t *vmblock)

	:param vmblock: Pointer to vmblock to clear

Description
~~~~~~~~~~~

Set VM block to all 0s.

Functionality
~~~~~~~~~~~~~

Indeed.


H2K_vm_translate
----------------

.. cfunction:: s32_t H2K_vm_translate(u32_t addr, H2K_vmblock_t *vmblock, u32_t *result)

	:param addr:  Address to translate
	:param vmblock: Pointer to vmblock
	:param result: Pointer to result

Description
~~~~~~~~~~~

Translate the given address using the pmap of the given vmblock

Functionality
~~~~~~~~~~~~~

If the translation type of the vmblock is 'offset', calculate the translated
address based on the offset descriptor (offset pages, page size).  Store result
and return 0 if the translated address falls within the fence parameters of the
vmblock.  Else return -1.

For other translation types call :cfunc:`H2K_translate()` to handle the translation.


H2K_vm_get_offset
-----------------

.. cfunction:: static inline H2K_mem_tlbfmt_t H2K_vm_get_offset(u32_t addr, H2K_thread_context *me)

	:param addr: Missed address
	:param me: Pointer to current thread context

Description
~~~~~~~~~~~

Create a TLB entry using an offset translation.

Functionality
~~~~~~~~~~~~~

Calculate the physical page number (virtual page number + offset from thread's
vmblock).  If this falls within the range of the vmblock fence parameters, fill
in the new TLB entry according to the offset descriptor and return it.  Else
return 0.

