:mod:`ring` -- Circular doubly-linked lists
===============================================

.. module:: ring

A ring is a circular doubly-linked list.  ``H2K_ringnode_t`` must be the first
part of an embedded object and is 8-byte aligned.  The ring pointer is a
pointer to the current node, or ``NULL`` for an empty ring.  The APIs do not
allocate nodes or validate that a node belongs to a ring.


H2K_ringnode_t
--------------

.. c:type:: H2K_ringnode_t

Contains ``next`` and ``prev`` pointers.  A single-node ring points both
pointers back to itself.


H2K_ring_next
-------------

.. c:function:: static inline H2K_ringnode_t *H2K_ring_next(void *ring, void *node)

	:param ring: Current ring node
	:param node: Node whose successor is requested
	:returns: ``node->next``, or ``NULL`` when the successor is ``ring``

Returns the next node while treating the ring pointer as the traversal
boundary.  It does not modify the ring.


H2K_ring_remove
---------------

.. c:function:: static inline void H2K_ring_remove(void *ring, void *node)

	:param ring: Address of the ring pointer
	:param node: Node to remove

Unlinks ``node`` from the ring.  If ``node`` is the ring pointer, advances the
ring pointer to ``node->next``; removing the only node sets the ring pointer to
``NULL``.  The node's own links are not cleared.


H2K_ring_insert
---------------

.. c:function:: static inline void H2K_ring_insert(void *ring, void *node)

	:param ring: Address of the ring pointer
	:param node: Node to insert

Inserts ``node`` at the ring pointer.  An empty ring becomes a one-node ring;
otherwise the old ring pointer remains after ``node`` and the ring pointer is
updated to ``node``.


H2K_ring_append
---------------

.. c:function:: static inline void H2K_ring_append(void *ring, void *node)

	:param ring: Address of the ring pointer
	:param node: Node to append

Inserts ``node`` immediately before the ring pointer.  An empty ring becomes a
one-node ring; for a nonempty ring the ring pointer is unchanged.


H2K_ring_remove_append
----------------------

.. c:function:: static inline void H2K_ring_remove_append(void *fromring, void *toring, void *node)

	:param fromring: Address of the source ring pointer
	:param toring: Address of the destination ring pointer
	:param node: Node to move

Removes ``node`` from ``fromring`` using the same pointer-update rules as
:c:func:`H2K_ring_remove()` and appends it to ``toring`` using the same rules as
:c:func:`H2K_ring_append()`.  The operation does not allocate or copy the node.
