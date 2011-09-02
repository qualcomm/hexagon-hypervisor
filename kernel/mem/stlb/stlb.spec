
:mod:`stlb` -- Software Translation Lookaside Buffer
=====================================================

.. module:: stlb


The STLB is a software-managed cache of translations for an address space.

By default, for minimal footprint, there is no STLB.  However, memory for
an STLB can be added by using the configuration trap.

The STLB is arranged in the following way:

* For each ASID, there is a small structure with information about the
  STLB.  This information includes:
 * Recommended Page Size
 * Bits indicating which sets have valid entries for this ASID
 * A mask indicating which ways may be replaced (to facilitate QoS)
 * Storage Base address
* Sets of translations.  The set of translations is based on a hash of
  the virtual address
* Several ways in each set.  Each way is looked up on a hardware TLB miss.
  The replaced way is chosen randomly (or fifo?).  Recommended to be 4 or 8.

Traps must have low, bounded latency.  Therefore, procedures such as
invalidating the STLB for an entire ASID must be able to be performed in a
small amount of time.  We do this by using dczeroa on the ASID valid bits.
This allows caches containing many sets per ASID to be invalidated in a 
small amount of time.  

When adding a new entry to the translation cache, if the valid bit for the
ASID is clear for this set, all ways must be cleaned of any translation
matching the ASID before adding the translation.

The number of sets times the number of ways may not be equal to the total 
amount of storage in the STLB.  Different ASIDs may start at different 
locations in the storage to make more effective use of the storage while
reducing collisions corresponding to common virtual address.

Each address space can specify the recommended page size.  This value is chosen 
as a hint for which bits to use to hash into the STLB.  Guests using many 4K 
pages will find best use of the STLB with lower order bits, however this will
result in many duplicated pages for larger pages.

H2K_mem_stlb_asid_info_t
------------------------

.. ctype:: H2K_mem_stlb_asid_t

	stlb information for asid

	.. cmember:: u64_t valids[STLB_MAX_SETS/64] 

		Array of valid bits for this stlb

	.. cmember:: u32_t pagesize 

		Pagesize in this stlb

	.. cmember:: u32_t waymask

	        Which ways may be replaced for QoS (FIXME)

	.. cmember:: H2K_mem_tlbfmt_t *baseaddr

	        Pointer to available memory


H2K_mem_stlb_init
-----------------

.. cfunction:: void H2K_mem_stlb_init()

Description
~~~~~~~~~~~

This function initilizes the stlb information.

H2K_mem_stlb_lookup
-------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_stlb_lookup(u32_t va, u32_t asid, H2K_thread_context *me)

	:param va: virtual address to look up
	:param asid: address space to look up
	:param me: Pointer to the current thread context
	:returns: the translation to fill into the TLB, or zero


Description
~~~~~~~~~~~

This function looks for a translation in the STLB.  If one is not found, zero is returned.

Behavior
~~~~~~~~

The recommended page size is used to collect bits to hash into the TLB.

We load and compare STLB entries in the expected set.

Optionally, we may select other sets corresponding to larger page sizes.

If we find a matching translation, we return the translation.

If no translation is found, we return zero. 

H2K_mem_stlb_add
----------------

.. cfunction:: void H2K_mem_stlb_add(u32_t va, u32_t asid, H2K_mem_tlbfmt_t entry, H2K_thread_context *me)

	:param va: virtual address to look up
	:param asid: address space to look up
	:param entry: Entry to add into the STLB
	:param me: pointer to the current thread context

Description
~~~~~~~~~~~

This function inserts an entry into the STLB.

Behavior
~~~~~~~~

The recommended page size and the VA are used to collect bits to hash into the TLB.
If the valid bit is cleared for the set, we clear any translation in the set with 
a matching ASID and set the valid bit.  We then select a random entry to replace
with "entry".


H2K_mem_stlb_invalidate_va
--------------------------

.. cfunction:: void H2K_mem_stlb_invalidate_va(u32_t va, u32_t asid, H2K_thread_context *me)

	:param va: virtual address to look up
	:param asid: address space to look up
	:param me: pointer to the current thread context

Description
~~~~~~~~~~~

This function looks for a translation in the STLB.  If one is found, it is cleared.

Behavior
~~~~~~~~

The same lookup behavior as :cfunc:`H2K_mem_stlb_lookup()` is used, but if a match is 
found, we clear the translation instead of returning the value.

H2K_mem_stlb_invalidate_asid
----------------------------

.. cfunction:: void H2K_mem_stlb_invalidate_asid(u32_t asid)

	:param asid: address space to invalidate


Description
~~~~~~~~~~~

This function marks all translations for an ASID as invalid.

Behavior
~~~~~~~~

The valid bits are cleared using DCZEROA for large numbers of 
sets, or stores for small numbers of sets.


Testing
-------


Important Cases
~~~~~~~~~~~~~~~

* STLB not initalized
 * add/lookup/invalidate_va/invalidate_asid, should return.
* STLB initalized
 * invalidate already invalid asid, should leave asid invalid.
 * invalidate already invalid va, should leave va invalid.
 * add followed by lookup, should return the same entry.
 * add followed by invalidate_va, subsequent lookup should return 0.
 * add followed by invalidate_asid, subsequent lookup should return 0.
 * add/lookup with different va  within the same page should succeed.

Harness
~~~~~~~

..cfunction void TH_mem_stlb_init() 

This function initializes the extern H2K_mem_stlb_asid_info_t *H2K_mem_stlb_asid_infos
pointer to a local array H2K_mem_stlb_asid_info_t TH_mem_stlb_asid_infos[MAX_ASIDS].
Each of these structures had its baseaddr pointer set throuought an array of H2K_mem_tlbfmt_t
entires in TH_mem_stlb[STLB_MAX_SETS*2][STLB_MAX_WAYS].  The entries are randomized.

..cfunction void TH_compare_tlbfmt(H2K_mem_tlbfmt_t original, H2K_mem_tlbfmt_t test)

This tests for equality between entries and FAILS if they differ.

..cfunction void TH_tlbfmt_iszero(H2K_mem_tlbfmt_t test)

This tests checks for an empty tlbfmt entry.

