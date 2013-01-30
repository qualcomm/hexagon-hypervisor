/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tree.h>
#include <globals.h>

#ifdef TREE_DEBUG
__attribute__((noinline)) void H2K_tree_fail() 
{
	while (1) /* SPIN FOREVER */;
}

static int H2K_tree_check_search(H2K_treenode_t *root)
{
	int ret = 0;
	H2K_treenode_t *left = root->left;
	H2K_treenode_t *right = root->right;
	if ((long)(left) & 1) return 1;
	if ((long)(right) & 1) return 1;
	root->left = (void *)(((long)left)+1);
	root->right = (void *)(((long)right)+1);
	if (left) ret |= H2K_tree_check_search(left);
	if (right) ret |= H2K_tree_check_search(right);
	root->left = left;
	root->right = right;
	return ret;
}

void H2K_tree_check(H2K_treenode_t *root)
{
	if (root == NULL) return;
	if (H2K_tree_check_search(root)) H2K_tree_fail();
}
#else
#define H2K_tree_check(...) do {} while (0) /* NOTHING */
#endif

void H2K_tree_remove_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key)
{
	if (*root == NULL) return; /* ERROR? */
	if (*root == node) {
		*root = node->left;
		if (node->right == NULL) {
			return;
		} else {
			return H2K_tree_add(root,node->right);
		}
	}
	if (key <= (*root)->key) {
		return H2K_tree_remove_key(&(*root)->left,node,key);
	} else {
		return H2K_tree_remove_key(&(*root)->right,node,key);
	}
}

void H2K_tree_add_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key)
{
	if (*root == NULL) {
		*root = node;
		return;
	}
	if (key <= (*root)->key) {
		return H2K_tree_add_key(&(*root)->left,node,key);
	} else {
		return H2K_tree_add_key(&(*root)->right,node,key);
	}
}

void H2K_tree_bisect(H2K_treenode_t **le_tree_p, H2K_treenode_t **gt_tree_p, H2K_treenode_t *root, H2K_treekey_t key)
{
	H2K_treenode_t *tmp;
	if (root == NULL) return;
	H2K_tree_check(root);
	if (root->key <= key) {
		*le_tree_p = root;
		*gt_tree_p = tmp = root->right;
		root->right = NULL;
		return H2K_tree_bisect(&root->right,gt_tree_p,tmp,key);
	} else {
		*gt_tree_p = root;
		*le_tree_p = tmp = root->left;
		root->left = NULL;
		return H2K_tree_bisect(le_tree_p,&root->left,tmp,key);
	}
}

void H2K_tree_destructive_iterate(H2K_treenode_t *root, void *opaque, void (*func)(H2K_treenode_t *, void *))
{
	H2K_treenode_t *tmp;
	H2K_tree_check(root);
	if (root == NULL) return;
	if (root->left == NULL) {
		tmp = root->right;
		func(root,opaque);
		return H2K_tree_destructive_iterate(tmp,opaque,func);
	} else {
		/* Rotate right and iterate */
		tmp = root->left;
		root->left = tmp->right;
		tmp->right = root;
		return H2K_tree_destructive_iterate(tmp,opaque,func);
	}
}

H2K_treenode_t *H2K_tree_find(H2K_treenode_t *root, H2K_treekey_t key)
{
	if (root == NULL) { return NULL; }
	if (root->key == key) {
		return root;
	}
	else if (root->key > key) {
		return H2K_tree_find(root->left, key);
	}
	else {
		return H2K_tree_find(root->right, key);
	}
}
