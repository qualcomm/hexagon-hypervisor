
:mod:`asid` -- Address Space IDentification Management
=======================================================

.. module:: asid

Overview
--------

The Hexagon Architecture uses Address Space IDentifiers (ASIDs) to 
distinguish between different virtual address spaces.

The Hexagon Hypervisor attempts to abstract the ASID feature of the
machine.  

Different Address Spaces must have different ASIDs.  Currently,
oversubscription of ASIDs is not supported.  This means that the Hexagon
Hypervisor will support a fixed maximum number of address spaces maintained by
the hypervisor.  

Guest environments can have arbitrarily large numbers of address spaces,
however a limited number of address spaces may be specified by all threads
simultaneously.

Each ASID has a reference count and a translation base.  When a thread is
created, it increments the reference count on the interhited address space.
When a thread changes to a new set of translations, the reference count for the
old address space is decremented and the reference count for the new address
space is incremented.  If a reference count drops to zero, the address space 
is considered not currently used, and the ASID is available for reuse.  However,
the address space is still considered valid until notifed otherwise by the guest.

This allows guests to leave cached translations around for several address spaces.

H2K_asid_table
--------------

.. ctype:: H2K_asid_entry_t

	Entry for the ASID table

	.. cmember:: u32_t ptb

		Base address for translation tables

	.. cmember:: u16_t count

		Reference count for the address space

	.. cmember:: u8_t maxhops

		Maximum number of hops from collisions at this bin.  When we add
		a new entry to the table, we set maxhops to the maximum of the old
		value and the new number of hops that was required to insert the entry.

        .. cmember:: translation_type transtype

	        Type of translation for this address space

.. cvar:: H2K_asid_entry_t H2K_asid_table[MAX_ASIDS]


H2K_asid_table_search
---------------------

.. cfunction:: static inline H2K_asid_entry_t *H2K_asid_table_search(u32_t ptb)

Description
~~~~~~~~~~~

Looks for an existing ASID entry configured for PTB.  Returns NULL if one is
not found.



H2K_asid_table_eviction
-----------------------

.. cfunction:: H2K_asid_entry_t *H2K_asid_table_eviction(u32_t ptb)

Description
~~~~~~~~~~~

Looks for an appropriate ASID to evict to create space for a new entry.

Returns NULL if all ASIDs are busy.

Updates maxhops in the bucket hashed to.

H2K_asid_table_inc
------------------

.. cfunction:: s32_t H2K_asid_table_inc(u32_t ptb)

	:param ptb: Address of the translation base

Description
~~~~~~~~~~~

This routine searches for an ASID already configured to specify the
address space pointed to by ptb.

If an ASID was already configured to specify the address space, 
the reference count is incremented.  Otherwise, we find a free 
ASID and set the reference count to 1.

If no free ASID is available, -1 is returned.  Otherwise, the ASID
corresponding to the "ptb" is returned.

Functionality
~~~~~~~~~~~~~

We hash into the asid table, and if not found, we traverse the table looking
for a matching entry.  If an entry is found, we increment the reference count.
Otherwise, we find the best free ASID (either an invalid one, or one with a 
reference count of zero) and use that.


H2K_asid_table_dec
------------------

.. cfunction:: void H2K_asid_table_dec(u32_t asid)

	:param asid: Address space to decrement reference count for


Description
~~~~~~~~~~~

This routine decrements the reference count for the ASID specified.


H2K_asid_table_invalidate
-------------------------

.. cfunction:: s32_t H2K_asid_table_invalidate(u32_t ptb)

	:param ptb: Address of the translation base

Description
~~~~~~~~~~~

This routine searches for an ASID that has been used for the specified 
translations, and assures that no cached translations remain valid for 
the specified PTB.

This routine must be called before modifying memory at "ptb" to be a new
set of pagetables.

Returns -1 if the asid is still in use.

Functionality
~~~~~~~~~~~~~

We search the ASID table for PTB.  If found, and the reference count is not
zero, the call was erroneous, and we return -1.  Otherwise, we invalidate the
TLB and STLB for the asid.  We then set the PTB field in the ASID table to
zero.


