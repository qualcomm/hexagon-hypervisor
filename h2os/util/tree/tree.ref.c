/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tree.h>

#ifdef TREE_DEBUG
__attribute__((noinline)) void h2_tree_fail() 
{
	while (1) /* SPIN FOREVER */;
}

static int h2_tree_check_search(h2_treenode_t *root)
{
	int ret = 0;
	h2_treenode_t *left = root->left;
	h2_treenode_t *right = root->right;
	if ((long)(left) & 1) return 1;
	if ((long)(right) & 1) return 1;
	root->left = (void *)(((long)left)+1);
	root->right = (void *)(((long)right)+1);
	if (left) ret |= h2_tree_check_search(left);
	if (right) ret |= h2_tree_check_search(right);
	root->left = left;
	root->right = right;
	return ret;
}

void h2_tree_check(h2_treenode_t *root)
{
	if (root == NULL) return;
	if (h2_tree_check_search(root)) h2_tree_fail();
}
#else
#define h2_tree_check(...) do {} while (0) /* NOTHING */
#endif

void h2_tree_remove_key(h2_treenode_t **root, h2_treenode_t *node, h2_treekey_t key)
{
	if (*root == NULL) return; /* ERROR? */
	if (*root == node) {
		*root = node->left;
		if (node->right == NULL) {
			return;
		} else {
			return h2_tree_add(root,node->right);
		}
	}
	if (key <= (*root)->key) {
		return h2_tree_remove_key(&(*root)->left,node,key);
	} else {
		return h2_tree_remove_key(&(*root)->right,node,key);
	}
}

void h2_tree_add_key(h2_treenode_t **root, h2_treenode_t *node, h2_treekey_t key)
{
	if (*root == NULL) {
		*root = node;
		return;
	}
	if (key <= (*root)->key) {
		return h2_tree_add_key(&(*root)->left,node,key);
	} else {
		return h2_tree_add_key(&(*root)->right,node,key);
	}
}

void h2_tree_bisect(h2_treenode_t **le_tree_p, h2_treenode_t **gt_tree_p, h2_treenode_t *root, h2_treekey_t key)
{
	h2_treenode_t *tmp;
	if (root == NULL) return;
	h2_tree_check(root);
	if (root->key <= key) {
		*le_tree_p = root;
		*gt_tree_p = tmp = root->right;
		root->right = NULL;
		return h2_tree_bisect(&root->right,gt_tree_p,tmp,key);
	} else {
		*gt_tree_p = root;
		*le_tree_p = tmp = root->left;
		root->left = NULL;
		return h2_tree_bisect(le_tree_p,&root->left,tmp,key);
	}
}

void h2_tree_destructive_iterate(h2_treenode_t *root, void *opaque, void (*func)(h2_treenode_t *, void *))
{
	h2_treenode_t *tmp;
	h2_tree_check(root);
	if (root == NULL) return;
	if (root->left == NULL) {
		tmp = root->right;
		func(root,opaque);
		return h2_tree_destructive_iterate(tmp,opaque,func);
	} else {
		/* Rotate right and iterate */
		tmp = root->left;
		root->left = tmp->right;
		tmp->right = root;
		return h2_tree_destructive_iterate(tmp,opaque,func);
	}
}

h2_treenode_t *h2_tree_find(h2_treenode_t *root, h2_treekey_t key)
{
	if (root == NULL) { return NULL; }
	if (root->key == key) {
		return root;
	}
	else if (root->key > key) {
		return h2_tree_find(root->left, key);
	}
	else {
		return h2_tree_find(root->right, key);
	}
}
