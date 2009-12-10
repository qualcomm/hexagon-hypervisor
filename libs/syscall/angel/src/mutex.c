/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
	h2_rmutex_t mutex;
	unsigned int used;
} mutex_entry;

#define MAX_LIBC_MUTEXES 64

static mutex_entry libc_mutexes[MAX_LIBC_MUTEXES];

static h2_mutex_t bigmutex = 0;

void sys_Mtxinit(void **mutex)
{
	int i;
	*mutex = NULL;
	h2_mutex_lock(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (libc_mutexes[i].used == 0) {
			h2_rmutex_init(&libc_mutexes[i].mutex);
			libc_mutexes[i].used = 1;
			*mutex = &libc_mutexes[i].mutex;
			break;
		}
	}
	h2_mutex_unlock(&bigmutex);
	assert(*mutex != NULL);
}

void sys_Mtxdst(void **mutex)
{
	int i;
	h2_mutex_lock(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (*mutex == &libc_mutexes[i].mutex) {
			libc_mutexes[i].used = 0;
			break;
		}
	}
	h2_mutex_unlock(&bigmutex);
}

void sys_Mtxlock(void **mutex)
{
	h2_rmutex_lock(*mutex);
}

void sys_Mtxunlock(void **mutex)
{
	h2_rmutex_unlock(*mutex);
}

