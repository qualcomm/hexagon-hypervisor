:mod:`tree` -- Binary search trees
====================================

.. module:: tree

A tree node contains two child pointers and a 64-bit key.  Keys less than or
equal to a node key are stored in the left subtree; greater keys are stored in
the right subtree.  The tree routines do not allocate or free nodes.


Types
-----

.. c:type:: H2K_treekey_t

Unsigned 64-bit tree key.

.. c:type:: H2K_treenode_t

Node containing ``left``, ``right``, and ``key`` fields.  The child pointers
are also available as ``leaf[0]`` and ``leaf[1]``.


H2K_tree_add_key
----------------

.. c:function:: void H2K_tree_add_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key)

	:param root: Pointer to the tree root
	:param node: Node to insert
	:param key: Key used to select its position

Inserts ``node`` into the tree.  Equal keys follow the left branch.


H2K_tree_add
------------

.. c:function:: static inline void H2K_tree_add(H2K_treenode_t **root, H2K_treenode_t *node)

Inserts ``node`` using ``node->key`` by calling
:c:func:`H2K_tree_add_key()`.


H2K_tree_remove_key
-------------------

.. c:function:: void H2K_tree_remove_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key)

	:param root: Pointer to the tree root
	:param node: Node to remove
	:param key: Key used to locate the node

Removes ``node`` from the tree when found.  If the removed node has a left
subtree, that subtree becomes the replacement root and the right subtree is
reinserted into it.  A null tree is ignored.


H2K_tree_remove
---------------

.. c:function:: static inline void H2K_tree_remove(H2K_treenode_t **root, H2K_treenode_t *node)

Removes ``node`` using ``node->key`` by calling
:c:func:`H2K_tree_remove_key()`.


H2K_tree_find
-------------

.. c:function:: H2K_treenode_t *H2K_tree_find(H2K_treenode_t *root, H2K_treekey_t key)

	:param root: Tree root
	:param key: Key to find
	:returns: Matching node, or ``NULL``

Searches the binary tree for an exact key.


H2K_tree_bisect
---------------

.. c:function:: void H2K_tree_bisect(H2K_treenode_t **le_tree_p, H2K_treenode_t **gt_tree_p, H2K_treenode_t *root, H2K_treekey_t key)

	:param le_tree_p: Output tree containing keys less than or equal to ``key``
	:param gt_tree_p: Output tree containing keys greater than ``key``
	:param root: Root of the tree to split
	:param key: Split key

Destructively splits the tree into the two output trees.  Nodes are relinked;
no nodes are allocated or copied.  A null input root leaves the outputs
unchanged.


H2K_tree_destructive_iterate
----------------------------

.. c:function:: void H2K_tree_destructive_iterate(H2K_treenode_t *root, void *opaque, void (*func)(H2K_treenode_t *, void *))

	:param root: Root of the tree to consume
	:param opaque: Caller-provided callback argument
	:param func: Callback for each node

Visits and removes every node from the tree structure, calling ``func(node,
opaque)`` once per node.  The traversal does not free the nodes.


H2K_tree_min / H2K_tree_max
---------------------------

.. c:function:: static inline H2K_treenode_t *H2K_tree_min(H2K_treenode_t *root)

.. c:function:: static inline H2K_treenode_t *H2K_tree_max(H2K_treenode_t *root)

Return ``NULL`` for an empty tree or the terminal node reached by following
child links.  ``H2K_tree_min`` follows ``left`` links.  The header's current
``H2K_tree_max`` implementation follows its existing inline expression and
should be treated as an implementation detail until corrected.


H2K_tree_check
--------------

.. c:function:: void H2K_tree_check(H2K_treenode_t *root)

When ``TREE_DEBUG`` is enabled, checks for pointer corruption and enters the
failure loop if corruption is found.  Without ``TREE_DEBUG`` it is a no-op.
