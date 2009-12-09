
:mod: `rings` -- circular doubly-linked lists
=============================================

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

H2K_ring_remove
---------------
.. cfunction: void H2K_ring_remove(void *ring, void *node);

Description
~~~~~~~~~~~

The H2K_ring_remove function removes "node" from "ring".  

By calling the function, you guarantee that "node" is a current member of
"ring".  The H2K_ring_remove function does not check this.

Input
~~~~~

Argument 0: H2K_ringnode_t **ring, a pointer to the ring holding node
Argument 1: The node in the ring to remove.

Output
~~~~~~

None

Functionality
~~~~~~~~~~~~~

The next and prev nodes pointed to by ``node`` have their prev and next
(respectively) pointers set to each other.  

If ``ring`` was pointing to ``node``, we need to point ``ring`` at a new
node.  If ``node->next`` equals ``node``, we point ``ring`` to ``NULL``, 
because the ring is now empty.  Otherwise, we point ``ring`` to ``node->next``.


H2K_ring_insert
---------------

.. cfunction: H2K_ring_insert(void *ring, void *node)

The H2K_ring_insert function adds ``node`` to ``ring``, as the node pointed to by
``ring``.

Input
~~~~~

Argument 0: A pointer to the ring to add the node to.
Argument 1: The node to be added

Output
~~~~~~

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

The H2K_ring_insert function adds ``node`` to ``ring``, as the node previous to the
node pointed to by ``ring``.

Input
~~~~~

Argument 0: A pointer to the ring to add the node to.
Argument 1: The node to be added

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

If ``ring`` points to a NULL pointer, we set ``node->next`` and ``node->prev``
to ``node``, and set ``*ring`` to ``node``.  Otherwise, we set ``node->next``
to ``*ring``, ``node->prev`` to `*ring->prev``, and then set
``node->next->prev`` and ``node->prev->next`` to
``node``.



H2K_ring_remove_append
-----------------------

.. cfunction:: H2K_ring_remove_append(void *fromring, void *toring, void *node)

The H2K_ring_remove_append function removes ``node`` from ``fromring`` and adds it to
``toring``, as the node previous to the node pointed to by ``toring``.

Input
~~~~~

Argument 0: A pointer to the ring to remove the node from.
Argument 1: A pointer to the ring to add the node to.
Argument 2: The node to be added

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

This function removes ``node`` from ``fromring`` in the same manner
as H2K_ring_remove, and then adds ``node`` to ``toring`` in the same 
manner as H2K_ring_append.


