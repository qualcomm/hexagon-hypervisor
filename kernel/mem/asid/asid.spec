
:mod: `asid` -- Address Space IDentification Management
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
is considered dead, and the ASID is available for reuse.

H2K_asid_table
--------------

.. ctype:: H2K_asid_entry_t

	Entry for the ASID table

	.. cmember:: u32_t ptb

		Base address for translation tables

	.. cmember:: u32_t count

		Reference count for the address space

.. cdata:: H2K_asid_entry_t H2K_asid_table[MAX_ASIDS]


H2K_asid_table_lookup
---------------------

.. cfunction:: H2K_asid_entry_t *H2K_asid_table_lookup(u32_t ptb)

	:param ptb: Address of the translation base

Description
~~~~~~~~~~~

This routine searches for an ASID already configured to specify the
address space pointed to by ptb.

The routine returns the address of the entry in the ASID table, or 
NULL if no entry is found.

Functionality
~~~~~~~~~~~~~

We hash into the asid table, and if not found, we traverse the table looking
for a matching entry.


H2K_asid_table_add
------------------

.. cfunction:: s32_t H2K_asid_table_add(u32_t ptb)

	:param ptb: Address of the translation base


Description
~~~~~~~~~~~

This routine searches for an empty ASID and configures it for "ptb".
The reference count is initialized to one.  

If no free ASID is available, this routine returns -1.  Otherwise, 
this routine returns the ASID that was chosen.


