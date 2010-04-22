
:mod:`pagewalk` -- Memory Map Page Table Walking
================================================

.. module:: pagewalk

H2K_mem_translate_pagetable
---------------------------

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

The page tables are located at me->gptb.






H2K_read_word_phys
------------------

.. cfunction:: u32_t H2K_read_word_phys(u64_t pa)

	:param pa: Physical address 

Description
~~~~~~~~~~~

The :cfunc:`H2K_read_word_phys()` function reads a word at a chosen physical
address.  It is assumed to be L1 and L2 cacheable.  

Functionality
~~~~~~~~~~~~~

For V4, we use the memw_phys instruction.  For V3, we set up a temporary mapping
in monitor memory space (upper 16MB) and read from there.



