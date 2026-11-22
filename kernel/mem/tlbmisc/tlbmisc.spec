:mod:`tlbmisc` -- Miscellaneous TLB Services
============================================

.. module:: tlbmisc

H2K_mem_tlb_probe
-----------------

.. c:function:: static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)

	:param va: Virtual address to probe
	:param asid: Address-space identifier
	:returns: Hardware TLB probe result

Description
~~~~~~~~~~~

Wraps the hardware ``tlbp`` instruction.  The probe tag is formed from the
virtual page number and ASID.


H2K_mem_tlb_read
----------------

.. c:function:: static inline u64_t H2K_mem_tlb_read(u32_t index)

	:param index: TLB entry index
	:returns: Raw 64-bit TLB entry

Description
~~~~~~~~~~~

Wraps the hardware ``tlbr`` instruction.


H2K_mem_tlb_write
-----------------

.. c:function:: static inline void H2K_mem_tlb_write(u32_t index, u64_t entry)

	:param index: TLB entry index
	:param entry: Raw 64-bit TLB entry to write

Description
~~~~~~~~~~~

Wraps the hardware ``tlbw`` instruction.


H2K_mem_tlboc
-------------

.. c:function:: static inline int H2K_mem_tlboc(u64_t entry)

	:param entry: TLB entry to check for overlap
	:returns: Hardware overlap result on ARCHV >= 5; a fixed fallback value on older architectures

Description
~~~~~~~~~~~

Checks for overlapping TLB entries on architectures that provide ``tlboc``.
Older architectures use a fallback implementation because the instruction is
not available.


H2K_mem_tlb_invalidate_va
-------------------------

.. c:function:: void H2K_mem_tlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me)

	:param va: Virtual address to invalidate
	:param count: Number of pages to invalidate
	:param asid: Address-space identifier
	:param me: Current thread context

Description
~~~~~~~~~~~

Invalidates the matching regular TLB entry, or invalidates the complete ASID
when more than one page is requested.

Functionality
~~~~~~~~~~~~~

For a single page, the function protects the temporary-map and TLB state,
probes the TLB, and clears the entry only when its index is within the regular
TLB range.  It then synchronizes the instruction and, when DMA support exists,
synchronizes the DMA TLB state.  The thread context is part of the interface
for compatibility with the other TLB services; this implementation does not
otherwise use it.


H2K_mem_tlb_invalidate_asid
---------------------------

.. c:function:: void H2K_mem_tlb_invalidate_asid(u32_t asid)

	:param asid: Address-space identifier to invalidate

Description
~~~~~~~~~~~

Invalidates all regular TLB entries belonging to an ASID.

Functionality
~~~~~~~~~~~~~

Uses the hardware ``tlbinvasid`` operation while holding the TLB lock, then
synchronizes the instruction stream and releases the lock.  When DMA support
is present, the DMA TLB is synchronized as well.


H2K_mem_tlb_dma_clear
---------------------

.. c:function:: void H2K_mem_tlb_dma_clear(void)

Description
~~~~~~~~~~~

Clears the DMA TLB entries on ARCHV >= 81.  On older architectures it performs
no operation.
