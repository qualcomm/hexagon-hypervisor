
:mod:`tlbmisc` -- Miscellaneous TLB Services
============================================

.. module:: tlbmisc

H2K_mem_tlb_probe
-----------------

.. c:function:: static inline u32_t H2K_mem_tlb_probe(u32_t va, u32_t asid)

Description
~~~~~~~~~~~

This is a wrapper for the hardware TLBP instruction.


H2K_mem_tlb_read
----------------

.. c:function:: static inline u64_t H2K_mem_tlb_read(u32_t index)

Description
~~~~~~~~~~~

This is a wrapper for the hardware TLBR instruction


H2K_mem_tlb_write
-----------------

.. c:function:: static inline void H2K_mem_tlb_write(u32_t index, u64_t entry)

Description
~~~~~~~~~~~

This is a wrapper for the hardware TLBW instruction


H2K_mem_tlb_invalidate_va
-------------------------

.. c:function:: void H2K_mem_tlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)

	:param va: virtual address to look up
	:param asid: address space to look up
	:param me: pointer to the current thread context

Description
~~~~~~~~~~~

This function looks for a translation in the hardware TLB.  If one is found, it is cleared.
Entries reserved for the kernel are not cleared.

Behavior 
~~~~~~~~

The TLB is probed using the TLBP instruction.  If a matching entry is found,
and it is not a kernel entry, we set the TLB entry to all zeros.


