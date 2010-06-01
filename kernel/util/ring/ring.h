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

typedef struct _H2K_ringnode {
	struct _H2K_ringnode *next;
	struct _H2K_ringnode *prev;
} __attribute__((aligned(8))) H2K_ringnode_t;

void H2K_ring_remove_real(H2K_ringnode_t **ring, H2K_ringnode_t *node) IN_SECTION(".text.core.ring");
void H2K_ring_insert_real(H2K_ringnode_t **ring, H2K_ringnode_t *node) IN_SECTION(".text.core.ring");
void H2K_ring_append_real(H2K_ringnode_t **ring, H2K_ringnode_t *node) IN_SECTION(".text.core.ring");
void H2K_ring_remove_append_real(H2K_ringnode_t **fromring, H2K_ringnode_t **toring, H2K_ringnode_t *node) IN_SECTION(".text.core.ring");

static inline void H2K_ring_remove(void *ring, void *node) {
	H2K_ring_remove_real((H2K_ringnode_t **)ring,(H2K_ringnode_t *)node);
}
static inline void H2K_ring_insert(void *ring, void *node) {
	H2K_ring_insert_real((H2K_ringnode_t **)ring,(H2K_ringnode_t *)node);
}
static inline void H2K_ring_append(void *ring, void *node) {
	H2K_ring_append_real((H2K_ringnode_t **)ring,(H2K_ringnode_t *)node);
}
static inline void H2K_ring_remove_append(void *fromring, void *toring, void *node) {
	H2K_ring_remove_append_real((H2K_ringnode_t **)fromring,(H2K_ringnode_t **)toring,(H2K_ringnode_t *)node);
}

#endif

