
:mod:`pagewalk` -- Memory Map Page Table Walking
================================================

.. module:: pagewalk

.. cfunction:: H2K_pte_t H2K_mem_translate_pagetable(u32_t badva, H2K_thread_context *me)

	:param badva: Virtual Address to translate
	:param me: Context of the current thread

Description
~~~~~~~~~~~

The :cfunc:`H2K_mem_translate_pagetable()` function walks the page tables, finding
an entry corresponding to the virtual address.  If no such translation exists,
we return all zeros

Functionality
~~~~~~~~~~~~~

The page tables are located at me->gtpb.



