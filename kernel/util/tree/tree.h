/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TREE_H
#define H2K_TREE_H 1

#include <c_std.h>
typedef u64_t H2K_treekey_t;

typedef struct treenode_struct {
	union {
		u64_t rightleft;
		struct {
			struct treenode_struct *left;
			struct treenode_struct *right;
		};
		struct treenode_struct *leaf[2];
	};
	H2K_treekey_t key;
} H2K_treenode_t;

void H2K_tree_add_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key);
void H2K_tree_remove_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key);
void H2K_tree_bisect(H2K_treenode_t **le_tree_p, H2K_treenode_t **gt_tree_p, H2K_treenode_t *root, H2K_treekey_t key);
void H2K_tree_destructive_iterate(H2K_treenode_t *root, void *opaque, void (*func)(H2K_treenode_t *, void *));
H2K_treenode_t *H2K_tree_find(H2K_treenode_t *root, H2K_treekey_t key);

static inline void H2K_tree_add(H2K_treenode_t **root, H2K_treenode_t *node) { return H2K_tree_add_key(root,node,node->key); }
static inline void H2K_tree_remove(H2K_treenode_t **root, H2K_treenode_t *node) { return H2K_tree_remove_key(root,node,node->key); }

static inline H2K_treenode_t *H2K_tree_min(H2K_treenode_t *root) { if ((root == NULL) || (root->left == NULL)) return root; return H2K_tree_min(root->left); }
static inline H2K_treenode_t *H2K_tree_max(H2K_treenode_t *root) { if ((root == NULL) || (root->right == NULL)) return root; return H2K_tree_max(root->left); }

#endif
