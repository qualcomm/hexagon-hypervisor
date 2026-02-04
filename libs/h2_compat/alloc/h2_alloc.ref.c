/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_alloc.h>
#include <h2_plainmutex.h>
#include <stdlib.h>

/*
 * h2_alloc.c
 * malloc / free / etc functions 
 * wrap the C library, but with a lock
 */

static h2_plainmutex_t memlock = H2_PLAINMUTEX_T_INIT;

void *h2_malloc(unsigned int size)
{
	void *ret;
	h2_plainmutex_lock(&memlock);
	ret = malloc(size);
	h2_plainmutex_unlock(&memlock);
	return ret;
}

void *h2_calloc(unsigned int elsize, unsigned int num)
{
	void *ret;
	h2_plainmutex_lock(&memlock);
	ret = calloc(elsize,num);
	h2_plainmutex_unlock(&memlock);
	return ret;
}

void *h2_realloc(void *ptr, int newsize)
{
	void *ret;
	h2_plainmutex_lock(&memlock);
	ret = realloc(ptr,newsize);
	h2_plainmutex_unlock(&memlock);
	return ret;
}

void *h2_memalign(size_t blocksize, size_t bytes)
{
	void *ret;
	h2_plainmutex_lock(&memlock);
	ret = memalign(blocksize,bytes);
	h2_plainmutex_unlock(&memlock);
	return ret;
}

void h2_free(void *ptr) 
{
	h2_plainmutex_lock(&memlock);
	free(ptr);
	h2_plainmutex_unlock(&memlock);
}

void h2_galloc_init(h2_galloc_t *alloc, unsigned int base, unsigned int size, h2_galloc_t *next)
{
	alloc->base = base;
	alloc->size = size;
	alloc->cur = base;
	alloc->next = next;

	h2_plainmutex_init(&alloc->lock);
}

void h2_galloc_reset(h2_galloc_t *alloc, int chain)
{
	do {
		alloc->cur = alloc->base;
		alloc = alloc->next;
	} while (chain && alloc);
}

void *h2_galloc(h2_galloc_t *alloc, unsigned int size, unsigned int align, int chain) {

	unsigned int addr;
	void *ret = NULL;

	do {
		h2_plainmutex_lock(&alloc->lock);
		addr = H2_ALIGN_UP(alloc->cur, align);

		if (addr + size <= alloc->base + alloc->size) {
			ret = (void *)addr;
			alloc->cur = addr + size;
			h2_plainmutex_unlock(&alloc->lock);
			break;
		} else {
			h2_plainmutex_unlock(&alloc->lock);
			alloc = alloc->next;
		}
	} while (chain && alloc);

	return ret;
}
