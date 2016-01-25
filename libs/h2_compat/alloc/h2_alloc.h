/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ALLOC_H
#define H2_ALLOC_H 1

/** @file h2_alloc.h
 * @brief malloc / free / etc functions wrap the C library, but with a lock
 */

void *h2_malloc(unsigned int size);
void *h2_calloc(unsigned int elsize, unsigned int num);
void *h2_realloc(void *ptr, int newsize);
void *h2_memalign(unsigned int blocksize, unsigned int bytes);
void h2_free(void *ptr);

#endif

