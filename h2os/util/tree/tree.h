/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef h2_TREE_H
#define h2_TREE_H 1

#include <c_std.h>
typedef u64_t h2_treekey_t;

typedef struct treenode_struct {
	union {
		u64_t rightleft;
		struct {
			struct treenode_struct *left;
			struct treenode_struct *right;
		};
		struct treenode_struct *leaf[2];
	};
	h2_treekey_t key;
} h2_treenode_t;

void h2_tree_add_key(h2_treenode_t **root, h2_treenode_t *node, h2_treekey_t key);
void h2_tree_remove_key(h2_treenode_t **root, h2_treenode_t *node, h2_treekey_t key);
void h2_tree_bisect(h2_treenode_t **le_tree_p, h2_treenode_t **gt_tree_p, h2_treenode_t *root, h2_treekey_t key);
void h2_tree_destructive_iterate(h2_treenode_t *root, void *opaque, void (*func)(h2_treenode_t *, void *));
h2_treenode_t *h2_tree_find(h2_treenode_t *root, h2_treekey_t key);

static inline void h2_tree_add(h2_treenode_t **root, h2_treenode_t *node) { return h2_tree_add_key(root,node,node->key); }
static inline void h2_tree_remove(h2_treenode_t **root, h2_treenode_t *node) { return h2_tree_remove_key(root,node,node->key); }

static inline h2_treenode_t *h2_tree_min(h2_treenode_t *root) { if ((root == NULL) || (root->left == NULL)) return root; return h2_tree_min(root->left); }
static inline h2_treenode_t *h2_tree_max(h2_treenode_t *root) { if ((root == NULL) || (root->right == NULL)) return root; return h2_tree_max(root->left); }

#endif
