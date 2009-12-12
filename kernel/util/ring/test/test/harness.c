/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>

#include <checker_ring.h>
#include <ringtests.h>

static H2K_ringnode_t adata, *a = &adata;
static H2K_ringnode_t bdata, *b = &bdata;
static H2K_ringnode_t cdata, *c = &cdata;
static H2K_ringnode_t ddata, *d = &ddata;

void test_ring_remove();
void test_ring_append();
void test_ring_insert();
void test_ring_remove_append();

void FAIL(const char *str)
{
	puts(str);
	fflush(stdout);
	fflush(stderr);
	exit(1);
}

H2K_ringnode_t *getnode(const char ch)
{
	switch (ch) {
		case 'a': return a;
		case 'b': return b;
		case 'c': return c;
		case 'd': return d;
		default: return NULL;
	}
}

void checkring(H2K_ringnode_t *head, const char *str)
{
	int i = 0;
	H2K_ringnode_t *tmp = head;
	if (*str == '\0') {
		if (head == NULL) return;
		else FAIL("Empty string, non-empty ringnode");
	}
	checker_ring(head);
	do {
		if (tmp != getnode(str[i])) {
			FAIL("Ring Check Fails");
		}
		tmp = tmp->next;
		i++;
	} while (tmp != head);
	if (str[i] != '\0') {
		FAIL("Ring ends before string");
	}
}

H2K_ringnode_t *makering(const char *str)
{
	int i;
	H2K_ringnode_t *head, *tail, *tmp;
	if (*str == '\0') return NULL;
	head = tail = NULL;
	for (i = 0; str[i] != '\0'; i++) {
		tmp = getnode(str[i]);
		if (head == NULL) {
			head = tmp;
			tail = head;
		} else {
			tail->next = tmp;
			tmp->prev = tail;
			tail = tmp;
		}
	}
	tail->next = head;
	head->prev = tail;
	checkring(head, str);
	return head;
}

int main()
{
	test_ring_remove();
	test_ring_insert();
	test_ring_append();
	test_ring_remove_append();
	puts("PASS");
	fflush(stdout);
	return 0;
}

