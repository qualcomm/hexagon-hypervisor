/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * Page Allocator
 * 
 * Used to allocate PA ranges for pools as well as VA ranges for allocations
 * 
 * We strongly want to favor larger pages where possible
 * 
 * Here are some ideas:
 * 0) Do normal allocator: keep track of (start, length) freelist tuples and first fit
 * 1) Buddy allocator: power of two and aggregate back. Storage for VA space?
 * 2) Binary search tree: keep allocated areas.  Not too hard to iterate over.
 * 3) pseudo-pagetables: easy to find power-of-four areas. Easy to iterate over. 
 *
 * Not a lot of information on things in qurt like: if you ask for a non-power-of-two size, 
 * is the kernel free to overallocate?
 *
 * OK, let's just do normal allocator and see how it does!
 *
 */

/* Find a good chunk for allocation */
/* We might have a mandated address within the free region, or 
 * we might just have an alignment hint.  Either way, try and
 * find a suitable chunk.  It might take the whole thing, it 
 * might be right along the left or right sides, or in the middle.  
 * Handle these cases and manipulate the free list appropriately.
 * 
 * Returns the address allocated.
 */
#include <qurt.h>

#ifdef DEBUG_BIST
#define qurt_malloc malloc
#define qurt_calloc calloc
#define qurt_free free
#include <stdio.h>
#define fatal() do { printf("OOPS: fatal\n"); exit(1); } while (0)
#endif

#ifndef fatal
#define fatal qurt_exception_raise_fatal
#endif

static inline unsigned int log2_rounddown(unsigned int x)
{
	return (31-__builtin_clz(x|1));
}

struct qurt_freelist_node {
	struct qurt_freelist_node *next;
	unsigned int base;
	unsigned int size;
};

static unsigned int rechunk(struct qurt_freelist_node **ptr,unsigned int addr,unsigned int size,unsigned int alignhint)
{
	struct qurt_freelist_node *tmp = *ptr;
	struct qurt_freelist_node *right, *left;
	unsigned int max_align;
	unsigned int alignbase;
	unsigned int oldbase = tmp->base;
	unsigned int oldsize = tmp->size;
	unsigned int offset;
	right = left = NULL;
	max_align = log2_rounddown(size);
	if (addr != 0) offset = addr-oldbase;
	else do {
		// qurt_printf("... max_align=%x alignhint=%x...",max_align,alignhint);
		alignhint &= ((1<<max_align)-1);
		alignbase = (oldbase & ((1<<max_align)-1));
		if (alignhint >= alignbase) offset = alignhint - alignbase;
		else offset = (1<<max_align) + alignhint - alignbase;
		// qurt_printf("... alignhint=%x alignbase=%x offset=%x\n",alignhint,alignbase,offset);
		max_align--;
	} while ((offset+size) > oldsize);
	/* OK, we've found out where we want this. */
	/* We could be ending up with 0, 1, or 2 nodes left over:
	 * * 0 nodes: fill up entire space
	 * * 1 node: aligned to left or right
	 * * 2 nodes: in the middle
	 */
	addr = oldbase + offset;
	if (offset != 0) {
		/* Need left side */
		left = tmp;
		left->size = offset;
	}
	if ((offset+size) < oldsize) {
		if (left == NULL) right = tmp;
		else {
			/* Two nodes */
			if ((right = qurt_malloc(sizeof(*right))) == NULL) fatal();
			right->next = left->next;
			left->next = right;
		}
		right->base = oldbase + offset + size;
		right->size = oldsize - (offset+size);
	}
	if ((left == NULL) && (right == NULL)) {
		/* zero nodes */
		*ptr = tmp->next;
		qurt_free(tmp);
	}
	return addr;
}

unsigned int qurt_pgalloc(struct qurt_freelist_node **ptr,unsigned int addr,unsigned int size,unsigned int alignhint)
{
	struct qurt_freelist_node *tmp;
	for (; (*ptr) != NULL; ptr = &((*ptr)->next)) {
		tmp = *ptr;
		if (tmp->size < size) continue;
		if (addr) {
			if (addr < tmp->base) continue;
			if ((addr+size) > (tmp->base+tmp->size)) continue;
		}
		/* We've found a chunk that we can allocate in.  Try and make it a good one. */
		return rechunk(ptr,addr,size,alignhint);
	}
	/* Couldn't find something suitable. */
	return 0;
}

void try_coalesce(struct qurt_freelist_node *left)
{
	struct qurt_freelist_node *right = left->next;
	if (right == NULL) return;
	if ((left->base+left->size) == (right->base)) {
		left->size += right->size;
		left->next = right->next;
		qurt_free(right);
	}
}

void qurt_pgfree(struct qurt_freelist_node **ptr,unsigned int addr,unsigned int size)
{
	struct qurt_freelist_node *tmp;
	struct qurt_freelist_node *newnode;
	for (; (*ptr) != NULL; ptr = &((*ptr)->next)) {
		tmp = *ptr;
		/* Is it lower and discontiguous? */
		if ((tmp->base + tmp->size) < addr) continue;
		/* Is it contiguous on the left? */
		if ((tmp->base + tmp->size) == addr) {
			tmp->size += size;
			try_coalesce(tmp);
			return;
		}
		/* Is it contiguous on the right? */
		if ((addr+size) == tmp->base) {
			tmp->base = addr;
			tmp->size += size;
			return;
		}
		/* If not, we'll need to make a spot for it at beginning of list */
		break;
	}
	/* Need to make a new node, possibly because we're at the end. */
	if ((newnode = qurt_malloc(sizeof(*newnode))) == NULL) fatal();
	newnode->base = addr;
	newnode->size = size;
	newnode->next = *ptr;
	*ptr = newnode;
}

void qurt_pgalloc_print_freelist(struct qurt_freelist_node *head)
{
	if (head == NULL) qurt_printf("END\n");
	else {
		qurt_printf("..base=%08x size=%08x\n",head->base,head->size);
		qurt_pgalloc_print_freelist(head->next);
	}
}

#ifdef DEBUG_BIST

#include <stdio.h>

static void FAIL(const char *msg)
{
	printf("oops: %s\n",msg);
	exit(1);
}

static void print_freelist(struct qurt_freelist_node *head)
{
	qurt_pgalloc_print_freelist(head);
}

int main() {
	struct qurt_freelist_node *list = NULL;
	unsigned int ret;
	printf("Empty list: ");
	print_freelist(list);
	qurt_pgfree(&list,0x100,0x100);
	printf("initial list: ");
	print_freelist(list);
	if (qurt_pgalloc(&list,0x180,1,0) != 0x180) FAIL("bad alloc");
	printf("after alloc in middle: ");
	print_freelist(list);
	if ((ret=qurt_pgalloc(&list,0,0x10,0x10)) != 0x100) {
		printf("got ret=%08x\n",ret);
		print_freelist(list);
		FAIL("unexpected");
	}
	printf("after alloc in start: ");
	print_freelist(list);
	if (qurt_pgalloc(&list,0,0x40,0x40) != 0x140) FAIL("alignment");
	printf("after alloc in middle 2: ");
	print_freelist(list);
	qurt_pgfree(&list,0x140,0x40);
	qurt_pgfree(&list,0x180,1);
	qurt_pgfree(&list,0x100,0x10);
	printf("all freed: ");
	print_freelist(list);
	if (qurt_pgalloc(&list,0,5,0x3) != 0x103) FAIL("alignment2");
	print_freelist(list);
	return 0;
}
#endif

