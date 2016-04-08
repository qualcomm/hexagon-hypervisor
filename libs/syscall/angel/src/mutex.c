/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
	pthread_mutex_t mutex;
	unsigned int used;
} mutex_entry;

#define MAX_LIBC_MUTEXES 64

static mutex_entry libc_mutexes[MAX_LIBC_MUTEXES];

static pthread_plainmutex_t bigmutex = PTHREAD_PLAINMUTEX_INITIALIZER_NP;
extern void pthread_init();
void sys_Mtxinit(void **mutex)
{
	int i;
	*mutex = NULL;
	pthread_init();
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	pthread_plainmutex_lock_np(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (libc_mutexes[i].used == 0) {
			pthread_mutex_init(&libc_mutexes[i].mutex,&attr);
			libc_mutexes[i].used = 1;
			*mutex = &libc_mutexes[i].mutex;
			break;
		}
	}
	pthread_plainmutex_unlock_np(&bigmutex);
	//	assert(*mutex != NULL);
}

void sys_Mtxdst(void **mutex)
{
	int i;
	pthread_plainmutex_lock_np(&bigmutex);
	for (i = 0; i < MAX_LIBC_MUTEXES; i++) {
		if (*mutex == &libc_mutexes[i].mutex) {
			libc_mutexes[i].used = 0;
			break;
		}
	}
	pthread_plainmutex_unlock_np(&bigmutex);
}

void sys_Mtxlock(void **mutex)
{
	pthread_mutex_lock(*mutex);
}

void sys_Mtxunlock(void **mutex)
{
	pthread_mutex_unlock(*mutex);
}

