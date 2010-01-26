
:mod:`rings` -- circular doubly-linked lists
============================================

.. module:: rings

SUMMARY
-------

Rings are circular doubly-linked lists.  

Rings are designed to hold any kind of data, provided the first two words may
be used as next and prev pointers, and are aligned on a doubleword boundary.

This is defined as H2K_ringnode_t::

	typedef struct _H2K_ringnode {
		struct _H2K_ringnode *next;
		struct _H2K_ringnode *prev;
	} __attribute__((aligned(8))) H2K_ringnode_t;


The rings.h header file defines the ring functions as inline wrappers that 
cast the input pointers to the correct H2K_ringnode_t type.  This spec
specifies the wrapped names.

Optimization Note
~~~~~~~~~~~~~~~~~

Optimized ring functions will only clobber r0-r5, with the exception of the
fused :cfunc:`H2K_ring_add_remove()`, which will additionally clobber r6 and r7.

H2K_ring_remove
---------------
.. cfunction:: void H2K_ring_remove(void *ring, void *node);

	:param ring: Actually `H2K_ringnode_t **ring`, a pointer to the ring holding node
	:param node: The node in the ring to remove.

Description
~~~~~~~~~~~

The :cfunc:`H2K_ring_remove()` function removes "node" from "ring".  

By calling the function, you guarantee that "node" is a current member of
"ring".  The :cfunc:`H2K_ring_remove()` function does not check this.

Functionality
~~~~~~~~~~~~~

The next and prev nodes pointed to by ``node`` have their prev and next
(respectively) pointers set to each other.  

If ``ring`` was pointing to ``node``, we need to point ``ring`` at a new
node.  If ``node->next`` equals ``node``, we point ``ring`` to ``NULL``, 
because the ring is now empty.  Otherwise, we point ``ring`` to ``node->next``.


H2K_ring_insert
---------------

.. cfunction:: H2K_ring_insert(void *ring, void *node)

	:param ring: A pointer to the ring to add the node to.
	:param node: The node to be added

The :cfunc:`H2K_ring_insert()` function adds ``node`` to ``ring``, as the node pointed to by
``ring``.

Functionality
~~~~~~~~~~~~~

If ``ring`` points to a NULL pointer, we set ``node->next`` and ``node->prev``
to ``node``.  Otherwise, we set ``node->next`` to ``*ring``, ``node->prev`` to
`*ring->prev``, and then set ``node->next->prev`` and ``node->prev->next`` to
``node``.

We then set ``*ring`` to ``node``.



H2K_ring_insert
---------------

.. cfunction:: H2K_ring_insert(void *ring, void *node)

	:param ring: A pointer to the ring to add the node to.
	:param node: The node to be added

The :cfunc:`H2K_ring_insert()` function adds ``node`` to ``ring``, as the node previous to the
node pointed to by ``ring``.

Functionality
~~~~~~~~~~~~~

If ``ring`` points to a NULL pointer, we set ``node->next`` and ``node->prev``
to ``node``, and set ``*ring`` to ``node``.  Otherwise, we set ``node->next``
to ``*ring``, ``node->prev`` to `*ring->prev``, and then set
``node->next->prev`` and ``node->prev->next`` to
``node``.



H2K_ring_remove_append
----------------------

.. cfunction:: H2K_ring_remove_append(void *fromring, void *toring, void *node)

	:param fromring: A pointer to the ring to remove the node from.
	:param toring: A pointer to the ring to add the node to.
	:param node: The node to be moved

The :cfunc:`H2K_ring_remove_append()` function removes ``node`` from ``fromring`` and adds it to
``toring``, as the node previous to the node pointed to by ``toring``.

Functionality
~~~~~~~~~~~~~

This function removes ``node`` from ``fromring`` in the same manner
as :cfunc:`H2K_ring_remove()`, and then adds ``node`` to ``toring`` in the same 
manner as :cfunc:`H2K_ring_append()`.



Testing
-------

Variants
~~~~~~~~

* i/o: Number of elements in ring
* input: Which element to remove/append
* output: final ring state, including order


Important cases
~~~~~~~~~~~~~~~

* Insert & Append to ring with 0, 1, 2, and 3 entries
* Remove from ring with 1, 2, and 3 entries
* Remove_Append from rings with different number of elements already existing.

Harness
~~~~~~~

H2K_ring functions do not require the whole kernel, merely the H2K_ring object file.

The test harness will set up various scenarios for functions, and then check to make
sure the rings have the expected configuration following the call under test.

Specific Tests
~~~~~~~~~~~~~~

* Remove node a from ring [ a ] -- must set ring pointer to NULL
* Remove node a from ring [ a b c ] -- must set ring pointer to b
* Remove node a from ring [ b a c ] -- ring pointer must be unchanged
* Remove node a from ring [ b c a ] -- ring pointer must be unchanged

* Insert node a to ring [ ] -- Must set ring pointer to a
* Insert node a to ring [ b ] -- must set ring pointer to a
* Insert node a to ring [ b c ] -- must set ring pointer to a

* Append node a to ring [ ] -- Must set ring pointer to a
* Append node a to ring [ b ] -- ring pointer must be unchanged
* Append node a to ring [ b c ] -- ring pointer must be unchanged

* remove_append node a from ring [ a ] to ring [ ] -- check for updated ring pointers
* remove_append node a from ring [ a b ] to ring [ ] -- check for updated ring pointers
* remove_append node a from ring [ b a ] to ring [ ] -- check for dest ptr update only

* remove_append node a from ring [ a ] to ring [ c ] -- check for src ptr update only
* remove_append node a from ring [ a b ] to ring [ c ] -- check for src ptr update only 
* remove_append node a from ring [ b a ] to ring [ c ] -- check for no ptr updates

