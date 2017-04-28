
:mod:`tree` -- binary search tree
=================================

.. module:: tree


SUMMARY
-------

Binary Search trees support O(log(n)) insertion, removal, searching.


H2K_tree_add
------------

.. c:function:: void H2K_tree_add(treenode **root, treenode *node)

	:param tree: Pointer to the tree root
	:param node: Node to add to the tree

H2K_tree_remove
---------------

.. c:function:: void H2K_tree_remove(treenode **root, treenode *node)

	:param tree: Pointer to the tree root
	:param node: Node to remove from the tree

H2K_tree_find
-------------

.. c:function:: treenode *H2K_tree_find(treenode **root, treekey key)

	:param tree: Pointer to the tree root
	:param key: Value to search for
	:returns: Pointer to node if found, NULL if not found

H2K_tree_bisect
---------------

.. c:function:: void H2K_tree_bisect(treenode **le_tree, treenode **gt_tree, treenode *root, treekey key)

	:param le_tree: Tree holding output nodes less than or equal to key
	:param gt_tree: Tree holding output nodes greater than key
	:param root: Tree root node.  Note that the tree root after bisecting will be either in le_tree or gt_tree output
	:param key: Value to split tree with

H2K_tree_destructive_iterate
----------------------------

.. c:function:: void H2K_tree_destructive_iterate(treenode *root, void (*func)(treenode *))

	:param root: Tree root node
	:param func: Function to call for each tree node



