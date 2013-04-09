/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "ramfs.h"

inode_t root;

static inline inode_t *get_inode(u32_t index)
{
	if (index >= (INODES_PER_PAGE * POINTERS_PER_PAGE)) return NULL;
	return inodes[index / INODES_PER_PAGE][index % INODES_PER_PAGE];
}

static inline void freelist_inode(u32_t index)
{
	inode_t *inode = get_inode(index);
	h2_mutex_lock(&fslock);
	inode->freelist_next = freelist;
	freelist = inode->freelist_next;
	h2_mutex_unlock(&fslock);
}

void ramfs_dir_create(inode_t *inode, inode_t *parent)
{
	u32_t *data;
	*inode = {
		.type_perm = TYPE_DIRECTORY | 0777;
		.unused_short = 0;
		.refcount = 1;
		.size = 12*2;
		.mutex = 0;
		.block0 = page_new_zero();
		.block1 = NULL;
		.indirect1 = NULL;
		.indirect2 = NULL;
	};
	data = inode->block0;
	data[0] = (u32_t)(inode);
	data[1] = (u32_t)(&data[3]);
	data[2] = 0x0000002E;		// "."
	data[3] = (u32_t)(parent);
	data[4] = NULL;
	data[5] = 0x00002E2E;		// ".."
}

void newnode()
{
	
}

void ramfs_init()
{
	inode_init(&root);
	ramfs_dir_create(&root,&root);
}

