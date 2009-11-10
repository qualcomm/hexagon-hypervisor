
UNIT: rings

SUMMARY:

Rings are circular doubly-linked lists.  

Rings are designed to hold any kind of data, provided the first two words may
be used as next and prev pointers, and are aligned on a doubleword boundary.

This is defined as BLASTK_ringnode_t::

	typedef struct _BLASTK_ringnode {
		struct _BLASTK_ringnode *next;
		struct _BLASTK_ringnode *prev;
	} __attribute__((aligned(8))) BLASTK_ringnode_t;


The rings.h header file defines the ring functions as inline wrappers that 
cast the input pointers to the correct BLASTK_ringnode_t type.  This spec
specifies the wrapped names.

FUNCTION: void BLASTK_ring_remove(void *ring, void *node);

DESCRIPTION:

The ring_remove function removes "node" from "ring".  

By calling the function, you guarantee that "node" is a current member of
"ring".  The ring_remove function does not check this.

INPUT:

Argument 0: BLASTK_ringnode_t **ring, a pointer to the ring holding node
Argument 1: The node in the ring to remove.

OUTPUT:

None

FUNCTIONALITY:

The next and prev nodes pointed to by ``node`` have their prev and next
(respectively) pointers set to each other.  

If ``ring`` was pointing to ``node``, we need to point ``ring`` at a new
node.  If ``node->next`` equals ``node``, we point ``ring`` to ``NULL``, 
because the ring is now empty.  Otherwise, we point ``ring`` to ``node->next``.





