/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * 
 * rings -- Doubly-linked list rings 
 *
 */

typedef struct _BLASTK_ringnode {
	struct _BLASTK_ringnode *next;
	struct _BLASTK_ringnode *prev;
} __attribute__((aligned(8))) BLASTK_ringnode_t;

void BLASTK_ring_remove_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node);
void BLASTK_ring_insert_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node);
void BLASTK_ring_append_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node);
void BLASTK_ring_remove_append_real(BLASTK_ringnode_t **fromring, BLASTK_ringnode_t **toring, BLASTK_ringnode_t *node);

static inline void BLASTK_ring_remove(void *ring, void *node) {
	BLASTK_ring_remove_real((BLASTK_ringnode_t **)ring,(BLASTK_ringnode_t *)node);
}
static inline void BLASTK_ring_insert(void *ring, void *node) {
	BLASTK_ring_insert_real((BLASTK_ringnode_t **)ring,(BLASTK_ringnode_t *)node);
}
static inline void BLASTK_ring_append(void *ring, void *node) {
	BLASTK_ring_append_real((BLASTK_ringnode_t **)ring,(BLASTK_ringnode_t *)node);
}
static inline void BLASTK_ring_remove_append(void *fromring, void *toring, void *node) {
	BLASTK_ring_remove_append_real((BLASTK_ringnode_t **)fromring,(BLASTK_ringnode_t **)toring,(BLASTK_ringnode_t *)node);
}

