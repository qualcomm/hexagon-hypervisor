/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RINGS_H
#define RINGS_H 1

/*
 * 
 * rings -- Doubly-linked list rings 
 *
 */

#include <c_std.h>

typedef struct _h2_ringnode {
	struct _h2_ringnode *next;
	struct _h2_ringnode *prev;
} __attribute__((aligned(8))) h2_ringnode_t;

void h2_ring_remove_real(h2_ringnode_t **ring, h2_ringnode_t *node) IN_SECTION(".text.core.ring");
void h2_ring_insert_real(h2_ringnode_t **ring, h2_ringnode_t *node) IN_SECTION(".text.core.ring");
void h2_ring_append_real(h2_ringnode_t **ring, h2_ringnode_t *node) IN_SECTION(".text.core.ring");
void h2_ring_remove_append_real(h2_ringnode_t **fromring, h2_ringnode_t **toring, h2_ringnode_t *node) IN_SECTION(".text.core.ring");

static inline void h2_ring_remove(void *ring, void *node) {
	h2_ring_remove_real((h2_ringnode_t **)ring,(h2_ringnode_t *)node);
}
static inline void h2_ring_insert(void *ring, void *node) {
	h2_ring_insert_real((h2_ringnode_t **)ring,(h2_ringnode_t *)node);
}
static inline void h2_ring_append(void *ring, void *node) {
	h2_ring_append_real((h2_ringnode_t **)ring,(h2_ringnode_t *)node);
}
static inline void h2_ring_remove_append(void *fromring, void *toring, void *node) {
	h2_ring_remove_append_real((h2_ringnode_t **)fromring,(h2_ringnode_t **)toring,(h2_ringnode_t *)node);
}

#ifdef DEBUG
#include <stdio.h>
static inline void h2_ring_dump_real(h2_ringnode_t **ring)
{
	h2_ringnode_t *tmp;
	if (*ring == NULL) {
		printf("NULL\n");
		return;
	}
	tmp = *ring;
	do {
		printf("%p ",tmp);
		tmp = tmp->next;
	} while (tmp != *ring);
	printf("\n");
}
static inline void h2_ring_dump(void *ring)
{
	h2_ring_dump_real((h2_ringnode_t **)ring);
}
#endif

#endif
