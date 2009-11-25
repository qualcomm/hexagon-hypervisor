/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_alloc.h
 * malloc / free / etc functions 
 * wrap the C library, but with a lock
 */

#ifndef BLAST_ALLOC_H
#define BLAST_ALLOC_H 1

void *blast_malloc(unsigned int size);
void *blast_calloc(unsigned int elsize, unsigned int num);
void *blast_realloc(void *ptr, int newsize);
void blast_free(void *ptr);

#endif

