
:mod:`tlbfill` -- Fill a TLB Entry
==================================

.. module:: tlbfill

H2K_mem_tlb_insert_unlock
-------------------------

.. c:function:: static inline void H2K_mem_tlb_insert_unlock(H2K_mem_tlbfmt_t entry, H2K_thread_context *me)

	:param entry: TLB entry to insert
	:param me: Current thread context, used for the TLB index mask


Description
~~~~~~~~~~~

Inserts an entry into the TLB at the next replacement index and releases the
TLB lock.

Functionality
~~~~~~~~~~~~~

For DMA-address-space entries on V73 and later, the DMA-TLB replacement index
and limit are selected; otherwise the regular TLB values are used.  The
current index is masked by ``me->tlbidxmask`` and passed to
:c:func:`H2K_mem_tlb_insert_index()`.  If insertion succeeds, the selected
replacement index is advanced and wrapped to zero at its last usable index.
The TLB lock is then released.


H2K_mem_tlb_fill
----------------

.. c:function:: void H2K_mem_tlb_fill(u32_t va, H2K_thread_context *me);

	:param va: virtual address that missed
	:param me: pointer to current thread context

Description
~~~~~~~~~~~

Finds a translation for a TLB miss and inserts it into the TLB.

Functionality
~~~~~~~~~~~~~

The function first looks up the virtual address in the STLB.  If a valid
entry is found, it is inserted into the TLB and the function returns.

On an STLB miss, the ASID translation entry is used to resolve the address
through the configured translation chain.  On V73 and later, the DMA TLB size
is supplied when converting the translation to TLB format.  If conversion
fails, :c:func:`H2K_mem_pagefault()` is called.  Otherwise, the resulting entry
is added to the STLB and inserted into the TLB.

