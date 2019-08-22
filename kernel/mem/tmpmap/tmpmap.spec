:mod:`tmpmap` -- Add/remove temporary translation
=================================================

.. module:: tmpmap

H2K_tmpmap_add_and_lock
-----------------------

.. c:function:: u32_t H2K_tmpmap_add_and_lock(pa_t pa, u32_t cccc)

	:param pa:  physical address to map
	:param cccc:  cacheability attributes

Description
~~~~~~~~~~~

Maps the page TEMP_MAP_VA, size TEMP_MAP_PG_SIZE to the physical page containing pa; returns with spinlock held.

Functionality
~~~~~~~~~~~~~

Acquire tmpmap_lock; lock TLB; free up a TLB entry by decrementing last_tlb_index; insert new mapping at free entry (unlocks TLB); return va corresponding to pa.


H2K_tmpmap_remove_and_unlock
----------------------------

.. c:function:: void H2K_tmpmap_remove_and_unlock()

Description
~~~~~~~~~~~

Removes temporary mapping; releases spinlock.

Functionality
~~~~~~~~~~~~~

Lock TLB; invalidate temporary entry; increment last_tlb_index; unlock TLB; release tmpmap_lock.



H2K_tmpmap_init
---------------

.. c:function:: void H2K_tmpmap_init()

Description
~~~~~~~~~~~

Initializes tmpmap_lock.
