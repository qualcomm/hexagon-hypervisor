
:mod:`pagewalk` -- Memory Map Page Table Walking
================================================

.. module:: pagewalk

H2K_mem_get_pagetable
---------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_get_pagetable(u32_t badva, H2K_thread_context *me)

	:param badva: Virtual Address to translate
	:param me: Context of the current thread
	:returns: TLB format of the translation

Description
~~~~~~~~~~~

The :cfunc:`H2K_mem_get_pagetable()` function walks the page tables, finding
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

Look up the page-table base in the ASID table.  Call :cfunc:`H2K_mem_pagewalk_l1()`.


H2K_mem_translate_pagetable
---------------------------

.. cfunction:: static inline H2K_translation_t H2K_mem_translate_pagetable(H2K_pte_t entry, u32_t va)

	:param entry: Page table entry
	:param va: Address to translate

Description
~~~~~~~~~~~

Translate an address using the given entry.  Provides both the fully translated
address as well as the page attributes of the translation.

Functionality
~~~~~~~~~~~~~

Form the translated address and store in return struct.  Copy attributes to
return struct.


H2K_mem_pagewalk_l1
-------------------

.. cfunction:: H2K_pte_t H2K_mem_pagewalk_l1(u32_t va, u32_t baseaddr, H2K_vmblock_t *vmblock)

	:param va: Address to translate
	:param baseaddr: Page table base address
	:param vmblock: vmblock pointer

Description
~~~~~~~~~~~

Return the page-table entry for the given address.

Functionality
~~~~~~~~~~~~~

Calculate the physical address of the entry and read.  For a level-2 entry, call :cfunc:`H2K_mem_pagewalk_l2()`.


H2K_mem_pagewalk_l2
-------------------

.. cfunction:: static inline H2K_pte_t H2K_mem_pagewalk_l2(u32_t va, u32_t l2addr, u32_t tablesize, u32_t pagesize, H2K_vmblock_t *vmblock)

	:param va: Address to translate
	:param l2addr: Guest address of level-2 page table
	:param tablesize: Size of level-2 table
	:param pagesize: page size from level-1 table
	:param vmblock: vmblock pointer

Description
~~~~~~~~~~~

Return the page-table entry for the given address.

Functionality
~~~~~~~~~~~~~

First translate the guest address of the level-2 table to a physical address,
using the vmblock's translations.  If this fails, return an invalid
translation.  Calculate the physical address of the entry, read and return.
