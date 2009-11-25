/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
	blast_rmutex_t mutex;
	unsigned int used;
} mutex_entry;

#define MAX_LIBC_MUTEXES 64

static mutex_entry libc_mutexes[MAX_LIBC_MUTEXES];

static blast_mutex_t bigmutex = 0;

void sys_Mtxinit(void **mutex)
{
	int i;
	*mutex = NULL;
	blast_mutex_lock(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (libc_mutexes[i].used == 0) {
			blast_rmutex_init(&libc_mutexes[i].mutex);
			libc_mutexes[i].used = 1;
			*mutex = &libc_mutexes[i].mutex;
			break;
		}
	}
	blast_mutex_unlock(&bigmutex);
	assert(*mutex != NULL);
}

void sys_Mtxdst(void **mutex)
{
	int i;
	blast_mutex_lock(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (*mutex == &libc_mutexes[i].mutex) {
			libc_mutexes[i].used = 0;
			break;
		}
	}
	blast_mutex_unlock(&bigmutex);
}

void sys_Mtxlock(void **mutex)
{
	blast_rmutex_lock(*mutex);
}

void sys_Mtxunlock(void **mutex)
{
	blast_rmutex_unlock(*mutex);
}

