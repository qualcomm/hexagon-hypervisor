/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast_alloc.h>
#include <blast_mutex.h>
#include <stdlib.h>

/*
 * blast_alloc.c
 * malloc / free / etc functions 
 * wrap the C library, but with a lock
 */

static unsigned int memlock = 0;

void *blast_malloc(unsigned int size)
{
	void *ret;
	blast_mutex_lock(&memlock);
	ret = malloc(size);
	blast_mutex_unlock(&memlock);
	return ret;
}

void *blast_calloc(unsigned int elsize, unsigned int num)
{
	void *ret;
	blast_mutex_lock(&memlock);
	ret = calloc(elsize,num);
	blast_mutex_unlock(&memlock);
	return ret;
}

void *blast_realloc(void *ptr, int newsize)
{
	void *ret;
	blast_mutex_lock(&memlock);
	ret = realloc(ptr,newsize);
	blast_mutex_unlock(&memlock);
	return ret;
}

void blast_free(void *ptr) 
{
	blast_mutex_lock(&memlock);
	free(ptr);
	blast_mutex_unlock(&memlock);
}

