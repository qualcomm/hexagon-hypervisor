:mod:`tlb` -- direct manipulation of the TLB
================================================

.. module:: tlb


H2K_tlb_tlbread
----------------

.. c:function:: s64_t H2K_tlb_tlbread(u32_t unused0, u32_t idx, u64_t unused32, H2K_thread_context *me)

	:param idx: TLB index to read
	:param me: Current thread context; unused by this operation
	:returns: Raw 64-bit TLB entry at ``idx``

Reads an entry directly from the hardware TLB.


H2K_tlb_tlbwrite
-----------------

.. c:function:: s64_t H2K_tlb_tlbwrite(u32_t unused0, u32_t idx, u64_t entry_in, H2K_thread_context *me)

	:param idx: TLB index to write
	:param entry_in: Raw 64-bit TLB entry
	:param me: Current thread context
	:returns: 0

Replaces the entry's ASID with the current thread's ASID, then writes the entry
while holding the TLB lock.


H2K_tlb_tlbquery
-----------------

.. c:function:: s64_t H2K_tlb_tlbquery(u32_t unused0, u32_t va, u64_t unused32, H2K_thread_context *me)

	:param va: Virtual address to probe
	:param me: Current thread context
	:returns: Matching TLB index, or a negative hardware probe result when no entry matches

Probes the hardware TLB using ``va`` and the current thread's ASID.


H2K_tlb_invalidate_overlap
--------------------------

.. c:function:: static int H2K_tlb_invalidate_overlap(u64_t entry_in)

	:param entry_in: Raw 64-bit TLB entry to check
	:returns: 0 if there is at most one overlap, or -1 if multiple entries overlap

Checks whether ``entry_in`` overlaps existing translations.  A single matching
entry is invalidated; no match requires no action.  Multiple matches are
reported as an error.


H2K_tlb_tlballoc
----------------

.. c:function:: s64_t H2K_tlb_tlballoc(u32_t unused0, u32_t unused1, u64_t entry_in, H2K_thread_context *me)

	:param entry_in: Raw 64-bit TLB entry to pin
	:param me: Current thread context
	:returns: Allocated TLB index, or -1 if allocation fails

Replaces the entry's ASID with the current thread's ASID and allocates the
highest available slot in the 64-entry pinned-TLB window.  Allocation fails if
the new entry has multiple overlaps or the pinned window is full.  A single
overlapping entry is invalidated before allocation.  If the allocation consumes
the last replaceable entry, the replaceable range is reduced accordingly.
The TLB lock protects the complete operation.


H2K_tlb_tlbfree
---------------

.. c:function:: s64_t H2K_tlb_tlbfree(u32_t unused0, u32_t index, u64_t unused32, H2K_thread_context *me)

	:param index: Index of the pinned TLB entry to free
	:param me: Current thread context; unused by this operation
	:returns: 0 on success, or -1 if ``index`` is outside the allocator-owned range

Invalidates the selected entry and clears its bit in ``pinned_tlb_mask``.  It
then extends ``last_tlb_index`` across consecutive free slots, restoring them
to the replaceable range.  Protected kernel entries at the top of the TLB
cannot be freed.  The TLB lock protects the operation.


H2K_tlb_dmaset
--------------

.. c:function:: s64_t H2K_tlb_dmaset(u32_t unused0, u32_t idx, u64_t entry_in, H2K_thread_context *me)

	:param idx: Index relative to the start of the DMA TLB
	:param entry_in: Raw 64-bit DMA TLB entry
	:param me: Current thread context; unused by this operation
	:returns: Absolute hardware TLB index on ARCHV >= 73, otherwise -1

On supported architectures, adds ``dma_tlb_start`` to ``idx`` and writes the
entry while holding the TLB lock.


H2K_tlb_tlbop
-------------

.. c:function:: s64_t H2K_tlb_tlbop(u32_t op, u32_t val32, u64_t val64, H2K_thread_context *me)

	:param op: One of ``TLBOP_TLBREAD``, ``TLBOP_TLBWRITE``, ``TLBOP_TLBQUERY``, ``TLBOP_TLBALLOC``, ``TLBOP_TLBFREE``, or ``TLBOP_DMASET``
	:param val32: Operation-specific 32-bit argument
	:param val64: Operation-specific 64-bit argument
	:param me: Current thread context
	:returns: Result of the selected operation, or -1 if ``op`` is invalid

Dispatches the request to the operation selected by ``op``.


Testing
-------

The standalone test verifies allocation until the pinned window is full,
freeing entries in different orders, preservation of reserved entries, and
read/write/query behavior with the current thread's ASID.
