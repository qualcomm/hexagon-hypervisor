/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _H2OS_MM_H
#define _H2OS_MM_H 1

#define MM_T_DEFINED 1
typedef struct mm_struct mm_t;

/* General MM structures and definitions */
#include <pagetable.h>

typedef struct mm_segment_struct {
	treenode tree;
	vaddr_t size;
	pte_t example_pte; /* permissions, cacheability, etc */
	/* for mmap'd things, file info? inode/offset? */
} mm_segment_t;

struct mm_struct {
	mm_segment_t *segments;
	pagetable_mm_info_t pagetable_info;
};

/* 
 * Some notes on how things work....
 * 
 * execve
 * 
 * This replaces the memory image with a new one from a file
 * 
 * I think this should be:
 * * Iterate over the memory space and free everything
 * * Take the ELF and mmap each ELF segment
 * * Probably set up stack segment
 * * Note which segment is heap (typically, highest writable section from ELF)
 * ... if we even allow brk/sbrk for heap.  Maybe we can push that and only do mmap?
 * 
 * Can we back sections that should be zero-initialized as MMAP'd /dev/zero?  Otherwise, 
 * can make COW with zero-page; however that makes the ref count on zero page huge, if we
 * map huge swaths of heap and stack with zero page mappings whenever they get created.
 * 
 */

/*
 * Functions to help us....
 */

/* Is there any segment in mm whose (start < base+size) AND (end > base)? If so, overlap. */
int mm_has_overlap(mm_t *mm, vaddr_t base, vaddr_t size);

/* Get segment for va */
mm_segment_t *mm_get_segment(mm_t *mm, vaddr_t va);

/* Is the segment writable? Useful for COW */
int mm_segment_iswrite(mm_t *mm, vaddr_t va);

#endif
