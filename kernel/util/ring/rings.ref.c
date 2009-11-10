/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * Rings -- doubly-linked list rings 
 */

#include <rings.h>
#include <std.h>

void BLASTK_ring_remove_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	if (*ring == node) {
		*ring = node->next;
		if (*ring == node) *ring = NULL;
	}
}

void BLASTK_ring_insert_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)
{
	if (*ring == NULL) {
		node->prev = node->next = node;
	} else {
		node->next = *ring;
		node->prev = (*ring)->prev;
		node->prev->next = node;
		node->next->prev = node;
	}
	*ring = node;
}

void BLASTK_ring_append_real(BLASTK_ringnode_t **ring, BLASTK_ringnode_t *node)
{
	if (*ring == NULL) {
		node->prev = node->next = node;
		*ring = node;
	} else {
		node->next = *ring;
		node->prev = (*ring)->prev;
		node->prev->next = node;
		node->next->prev = node;
	}
}

void BLASTK_ring_remove_append_real(BLASTK_ringnode_t **fromring, BLASTK_ringnode_t **toring, BLASTK_ringnode_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	if (*fromring == node) {
		if (node == node->next) *ring = NULL;
		else *ring = node->next;
	}
	if (*toring == NULL) {
		node->prev = node->next = node;
		*ring = node;
	} else {
		node->next = *ring;
		node->prev = (*ring)->prev;
		node->prev->next = node;
		node->next->prev = node;
	}
}

