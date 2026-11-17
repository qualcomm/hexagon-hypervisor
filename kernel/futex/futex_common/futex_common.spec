
:mod:`futex_common` -- Futex Common Routines
============================================

.. module:: futex_common

H2K_futex_hash_add_ring
------------------

.. c:function:: void H2K_futex_hash_add_ring(H2K_thread_context **ring, H2K_thread_context *me)

Description
~~~~~~~~~~~

:c:func:`H2K_futex_hash_add_ring()` adds the thread context to the ring

Functionality
~~~~~~~~~~~~~

Put in the hash at the right place (priority sorted)


H2K_futex_hash_remove_one
-------------------------

.. c:function:: H2K_thread_context *H2K_futex_hash_remove_one(u32_t locklo, H2K_thread_context **ring, H2K_thread_context **pos)

	:param locklo: the low 32 bits of the physical address being matched
	:param ring: the futex hash bucket ring to search
	:param pos: the position in the ring at which to resume searching
	:returns: the removed thread, or NULL if no matching thread was found

Description
~~~~~~~~~~~

:c:func:`H2K_futex_hash_remove_one()` finds the next thread in the ring
whose futex address matches "locklo", removes it from the ring, and makes
it ready.

Functionality
~~~~~~~~~~~~~

If the ring is empty, we return NULL.

Otherwise, starting at "pos", we walk the ring looking for a thread whose
`futex_ptr_lo` field equals "locklo".  Comparing only the low 32 bits of the
physical address is sufficient here, because the hash function guarantees
that addresses with distinct upper bits hash to distinct buckets.

If a match is found, we remove it from the ring with
:c:func:`H2K_ring_remove()`, add it to the ready list with
:c:func:`H2K_ready_append()`, and set "pos" to the position following the
removed thread, so that a subsequent call can resume searching without
re-examining earlier elements.  The removed thread is returned, both for the
multi-wake caller and so that a PI caller can inspect it.

If we walk all the way back around to the start of the ring without finding
a match, we return NULL.

