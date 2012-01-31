/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TREE_H
#define H2K_TREE_H 1

#include <c_std.h>
typedef u64_t treekey;

typedef struct treenode_struct {
	union {
		u64_t rightleft;
		struct {
			struct treenode_struct *left;
			struct treenode_struct *right;
		};
		struct treenode_struct *leaf[2];
	};
	treekey key;
} treenode;

void H2K_tree_add_key(treenode **root, treenode *node, treekey key);
void H2K_tree_remove_key(treenode **root, treenode *node, treekey key);
void H2K_tree_bisect(treenode **le_tree_p, treenode **gt_tree_p, treenode *root, treekey key);
void H2K_tree_destructive_iterate(treenode *root, void (*func)(treenode *));

static inline void H2K_tree_add(treenode **root, treenode *node) { return H2K_tree_add_key(root,node,node->key); }
static inline void H2K_tree_remove(treenode **root, treenode *node) { return H2K_tree_remove_key(root,node,node->key); }

static inline treenode *H2K_tree_min(treenode *root) { if ((root == NULL) || (root->left == NULL)) return root; return H2K_tree_min(root->left); }
static inline treenode *H2K_tree_max(treenode *root) { if ((root == NULL) || (root->right == NULL)) return root; return H2K_tree_max(root->left); }

#endif
