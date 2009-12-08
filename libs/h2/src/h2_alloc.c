/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_alloc.h>
#include <h2_mutex.h>
#include <stdlib.h>

/*
 * h2_alloc.c
 * malloc / free / etc functions 
 * wrap the C library, but with a lock
 */

static unsigned int memlock = 0;

void *h2_malloc(unsigned int size)
{
	void *ret;
	h2_mutex_lock(&memlock);
	ret = malloc(size);
	h2_mutex_unlock(&memlock);
	return ret;
}

void *h2_calloc(unsigned int elsize, unsigned int num)
{
	void *ret;
	h2_mutex_lock(&memlock);
	ret = calloc(elsize,num);
	h2_mutex_unlock(&memlock);
	return ret;
}

void *h2_realloc(void *ptr, int newsize)
{
	void *ret;
	h2_mutex_lock(&memlock);
	ret = realloc(ptr,newsize);
	h2_mutex_unlock(&memlock);
	return ret;
}

void h2_free(void *ptr) 
{
	h2_mutex_lock(&memlock);
	free(ptr);
	h2_mutex_unlock(&memlock);
}

