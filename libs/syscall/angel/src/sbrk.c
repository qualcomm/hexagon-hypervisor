/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>

#define CHUNK_SIZE (4*1024)
#define LO_MASK(SIZ) ((SIZ)-1)
#define HI_MASK(SIZ) (-(SIZ))

#define ALIGN_UP(X,SIZ) (((X) + LO_MASK(SIZ)) & HI_MASK(SIZ))

extern char end;

extern void *heapBase __attribute__((section(".data")));
extern void *heapLimit __attribute__((section(".data")));

static pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

static unsigned long long int *heap_base = NULL;
static unsigned long long int *heap_start = NULL;

void *sys_sbrk(ptrdiff_t more)
{
	unsigned long long int *old_base, *new_base;
	pthread_init();  // because c++ mallocs before main(), and some mallocs call sys_sbrk().
	pthread_mutex_lock(&mylock);
	if (heap_base == NULL) {
		if (((unsigned int)(heapBase)) >= ((unsigned int)(&end))) {
			heap_start = heap_base = (void *)(ALIGN_UP((unsigned int)heapBase,CHUNK_SIZE));
		} else {
			heap_start = heap_base = (void *)(ALIGN_UP((unsigned int)(&end),CHUNK_SIZE));
		}
	}
	old_base = heap_base;
	if (more) {
		new_base = (void *)ALIGN_UP((unsigned int)heap_base + more,CHUNK_SIZE);
		if ((unsigned int)(new_base) < (unsigned int)(heap_start)) {
			new_base = heap_start;
		}
		//		if (heapLimit && ((unsigned int)(new_base) >= ((unsigned int)(heap_start) + heapLimit))) {
		if (heapLimit && ((unsigned int)(new_base) >= ((unsigned int)(heap_start) + (heapLimit - heapBase)))) {
			pthread_mutex_unlock(&mylock);
			return (void *)(-1);
		}
		heap_base = new_base;
	}
	pthread_mutex_unlock(&mylock);
	((unsigned int *)(old_base))[0] = more;
	((unsigned int *)(old_base))[1] = more;
	return old_base;
}

