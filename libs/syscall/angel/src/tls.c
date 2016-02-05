/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dinkumware tls */

#include <pthread.h>

/*
Checking for this at every tls alloc operation seems wasteful, and
even more so when you have to contend for a lock in order to check it.

And that's assuming that the user alloc's before trying to get, set, or free.

We really just need an __init section.
*/

//static int tls_init_done = 0;

int sys_Tlsalloc(pthread_key_t *key, void (*dtor)(void *))
{
	return pthread_key_create(key,dtor);
}

int sys_Tlsfree(pthread_key_t key)
{
	return pthread_key_delete(key);
}

int sys_Tlsset(pthread_key_t key, void *p)
{
	return pthread_setspecific(key,p);
}

void *sys_Tlsget(pthread_key_t key)
{
	return pthread_getspecific(key);
}

