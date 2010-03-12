
:mod:`tlbfill` -- Fill a TLB Entry
==================================

.. module:: tlbfill

H2K_mem_tlb_insert
------------------

.. cfunction:: static inline void H2K_mem_tlb_insert(u64_t entry)

	:param entry: TLB entry


Description
~~~~~~~~~~~

Inserts an entry into the TLB in the appriate place.

Functionality
~~~~~~~~~~~~~

First, we obtain the index, increment to the next appropriate replacement 
value, and store the next replacement index.

This function uses the architecture-specific code to insert an entry into the
TLB.  For V3 and earlier, tthe TLBHI, TLBLO, and TLBIDX registers must be used
to read, write, and probe TLB entries.  For V4 and later, registers can be 
used directly.


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

