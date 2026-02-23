/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ALLOC_H
#define H2_ALLOC_H 1

/** @file h2_alloc.h
 * @brief malloc / free / etc functions wrap the C library, but with a lock
 */

#include <h2_plainmutex.h>
#include <h2_common_c_std.h>

void *h2_malloc(unsigned int size);
void *h2_calloc(unsigned int elsize, unsigned int num);
void *h2_realloc(void *ptr, int newsize);
void *h2_memalign(unsigned int blocksize, unsigned int bytes);
void h2_free(void *ptr);

/**
@brief Generic allocator. No direct access.
*/
typedef struct h2_galloc {
	h2_plainmutex_t lock;
	unsigned int base;
	unsigned int size;
	unsigned int cur;
	struct h2_galloc *next;
} h2_galloc_t;

/**
Initialize generic allocator.
@param[in] alloc  Address of allocator
@param[in] base  Base address of allocation region
@param[in] size  Size in bytes of allocation region
@param[in] next  Pointer to next allocator; may be NULL
@dependencies None
*/
void h2_galloc_init(h2_galloc_t *alloc, unsigned int base, unsigned int size, h2_galloc_t *next);

/**
Reset generic allocator.
@param[in] alloc  Address of allocator
@param[in] chain  Flag: Reset all allocators in chain
@dependencies None
*/
void h2_galloc_reset(h2_galloc_t *alloc, int chain);

/**
Allocate memory; optionally try chained allocators if preceding ones fail.
@param[in] alloc  Address of allocator
@param[in] size   Size in bytes to allocate
@param[in] align  Minimum alignment (byte address)
@param[in] chain  Flag: Try all allocators in chain
@returns  Pointer to allocated memory. NULL on failure.
@dependencies None
*/
void *h2_galloc(h2_galloc_t *alloc, unsigned int size, unsigned int align, int chain);

#endif
