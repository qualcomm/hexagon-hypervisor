:mod:`tmpmap` -- Temporary physical-memory mapping
====================================================

.. module:: tmpmap

H2K_tmpmap_add_and_lock
-----------------------

.. c:function:: u32_t H2K_tmpmap_add_and_lock(pa_t pa, u32_t cccc, u32_t size)

	:param pa: Physical address to map
	:param cccc: Cacheability attributes
	:param size: Mapping-size encoding, or ``SIZE_DEFAULT`` to use the default
	:returns: Virtual address corresponding to ``pa``

Description
~~~~~~~~~~~

Creates a temporary mapping from ``TEMP_MAP_VA`` to the physical page
containing ``pa`` and returns with the temporary-map spinlock held.  The
caller must later call :c:func:`H2K_tmpmap_remove_and_unlock()`.

Functionality
~~~~~~~~~~~~~

If ``size`` is ``SIZE_DEFAULT``, ``TEMP_MAP_PG_SIZE`` is used.  The function
constructs a global, monitor-only TLB entry for the requested physical page,
including the physical-address extension bits required on V73 and later.  It
then locks the temporary-map spinlock and TLB, reserves the current last TLB
entry for the mapping, inserts the entry, and releases the TLB lock.

The returned address is ``TEMP_MAP_VA`` combined with the offset of ``pa``
within the selected page.  The temporary-map spinlock remains held until the
matching removal function is called.


H2K_tmpmap_remove_and_unlock
----------------------------

.. c:function:: void H2K_tmpmap_remove_and_unlock(void)

Description
~~~~~~~~~~~

Removes the current temporary mapping and releases the temporary-map spinlock.

Functionality
~~~~~~~~~~~~~

Locks the TLB, invalidates the temporary entry at the next available last-TLB
index, restores that index, releases the TLB lock, and then releases
``H2K_gp->tmpmap_lock``.


H2K_tmpmap_init
---------------

.. c:function:: void H2K_tmpmap_init(void)

Description
~~~~~~~~~~~

Initializes the temporary-map lock.
