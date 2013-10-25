
:mod:`tlbfill` -- Fill a TLB Entry
==================================

.. module:: tlbfill

H2K_mem_tlb_insert_unlock
------------------

.. cfunction:: static inline void H2K_mem_tlb_insert_unlock(H2K_mem_tlbfmt_t entry)

	:param entry: TLB entry


Description
~~~~~~~~~~~

Inserts an entry into the TLB at the next index to be replaced and releases the TLB lock.

Functionality
~~~~~~~~~~~~~

First, we obtain the index, increment to the next appropriate replacement 
value, and store the next replacement index.

This function uses the architecture-specific code to insert an entry into the
TLB.  For V3 and earlier, the TLBHI, TLBLO, and TLBIDX registers must be used
to read, write, and probe TLB entries.  If the entry is being filled in
response to a guest miss, we set the guest bit in the ASID of the entry.  For
V4 and later, registers can be used directly.

Call :cfunc:`H2K_mutex_unlock_tlb()`.


H2K_mem_tlb_fill
----------------

.. cfunction:: void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me);

	:param va: virtual address that missed
	:param me: pointer to current thread context

Description
~~~~~~~~~~~

This code finds a translation for a TLB miss, and inserts it into the TLB. 

Functionality
~~~~~~~~~~~~~

First, we check the STLB (if available).  If the STLB returns a vaild
translation, we insert it into the TLB and return.

Otherwise, we use the appropriate handler function to search the translation
backing store.  If a translation is found there, we insert it into the TLB 
and return. 

Finally, if we can not find a translation, we call to
:cfunc:`H2K_mem_pagefault()` and return.

