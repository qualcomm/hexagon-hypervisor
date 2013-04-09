/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * Rings -- doubly-linked list rings 
 */

#include <ring.h>
#include <c_std.h>

void h2_ring_remove_real(h2_ringnode_t **ring, h2_ringnode_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	if (*ring == node) {
		*ring = node->next;
		if (*ring == node) *ring = NULL;
	}
}

void h2_ring_insert_real(h2_ringnode_t **ring, h2_ringnode_t *node)
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

void h2_ring_append_real(h2_ringnode_t **ring, h2_ringnode_t *node)
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

void h2_ring_remove_append_real(h2_ringnode_t **fromring, h2_ringnode_t **toring, h2_ringnode_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	if (*fromring == node) {
		if (node == node->next) {
			*fromring = NULL;
		} else {
			*fromring = node->next;
		}
	}
	if (*toring == NULL) {
		node->prev = node->next = node;
		*toring = node;
	} else {
		node->next = *toring;
		node->prev = (*toring)->prev;
		node->prev->next = node;
		node->next->prev = node;
	}
}

