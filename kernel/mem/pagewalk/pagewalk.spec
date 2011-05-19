
:mod:`pagewalk` -- Memory Map Page Table Walking
================================================

.. module:: pagewalk

H2K_mem_translate_pagetable
---------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_translate_pagetable(u32_t badva, H2K_thread_context *me)

	:param badva: Virtual Address to translate
	:param me: Context of the current thread
	:returns: TLB format of the translation

Description
~~~~~~~~~~~

The :cfunc:`H2K_mem_translate_pagetable()` function walks the page tables, finding
an entry corresponding to the virtual address.  If no such valid translation exists,
we return all zeros; an invalid TLB entry.

An invalid translation can be due to an L1 PTE with an invalid size (7), or no r, w, or x 
permissions.

Functionality
~~~~~~~~~~~~~

The page tables are located at me->gptb.






H2K_mem_pagewalk
----------------

.. cfunction:: H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me)

	:param badva: Virtual Address to translate
	:param me: Context of the current thread
	:returns: PTE format of the translation

Description
~~~~~~~~~~~

The :cfunc:`H2K_mem_pagewalk()` function walks the page tables, finding
an entry corresponding to the virtual address.  If no such valid translation exists,
we return all zeros.

An invalid translation can be due to an L1 PTE with an invalid size (7), or no r, w, or x 
permissions.

Functionality
~~~~~~~~~~~~~

The page tables are located at me->gptb.







