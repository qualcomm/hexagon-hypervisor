/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <tree.h>

void H2K_tree_remove_key(treenode **root, treenode *node, treekey key)
{
	if (*root == NULL) return; /* ERROR? */
	if (*root == node) {
		*root = node->left;
		return H2K_tree_add(root,node->right);
	}
	if (key <= (*root)->key) {
		return H2K_tree_remove_key(&(*root)->left,node,key);
	} else {
		return H2K_tree_remove_key(&(*root)->right,node,key);
	}
}

void H2K_tree_add_key(treenode **root, treenode *node, treekey key)
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

void H2K_tree_bisect(treenode **le_tree_p, treenode **gt_tree_p, treenode *root, treekey key)
{
	treenode *tmp;
	if (root == NULL) return;
	if (root->key <= key) {
		*le_tree_p = root;
		tmp = root->right;
		root->right = NULL;
		return H2K_tree_bisect(&root->right,gt_tree_p,tmp,key);
	} else {
		*gt_tree_p = root;
		tmp = root->left;
		root->left = NULL;
		return H2K_tree_bisect(le_tree_p,&root->left,tmp,key);
	}
}

void H2K_tree_destructive_iterate(treenode *root, void (*func)(treenode *))
{
	treenode *tmp;
	if (root == NULL) return;
	if (root->left == NULL) {
		tmp = root->right;
		func(root);
		return H2K_tree_destructive_iterate(tmp,func);
	} else {
		/* Rotate right and iterate */
		tmp = root->left;
		root->left = tmp->right;
		tmp->right = root;
		return H2K_tree_destructive_iterate(tmp,func);
	}
}

