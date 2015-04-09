/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dinkumware tls */

#include <h2.h>

#define mutex_type		h2_mutex_t
#define mutex_lock(lock)	h2_mutex_lock(lock)
#define mutex_unlock(lock)	h2_mutex_unlock(lock)
#define mutex_init(lock)	h2_mutex_init(lock)

#define MAX_TLS_ENTRIES 64
typedef struct
{
	void *data;
	int used;
	mutex_type lock;  //  this is going to be pretty onerous
} tls_entry_t;

static tls_entry_t tls_entries[MAX_TLS_ENTRIES];

/*
Checking for this at every tls alloc operation seems wasteful, and
even more so when you have to contend for a lock in order to check it.

And that's assuming that the user alloc's before trying to get, set, or free.

We really just need an __init section.
*/

//static int tls_init_done = 0;
h2_mutex_t tls_init_done=H2_MUTEX_T_INIT;  //  this needs to change if the mutex type changes

static void tlsinit()
{
	int i;

	if (h2_mutex_trylock(&tls_init_done) != 0) return;  //  trylock returns 0 on success

	for (i = 0; i < MAX_TLS_ENTRIES; i++) {
		tls_entries[i].used = 0;
		mutex_init(&tls_entries[i].lock);
	}
	//  leave mutex locked, indicating init has been performed.

	//  problem:  if this happens during multithreaded operation, the
	//  threads that returned immediately might execute faster than the one
	//  doing the initialization.
}

int sys_Tlsalloc(void ** key, void (*dtor)(void *))
{
	unsigned int i;
	/* dtor is ignored unless c library need it */
	tlsinit();  //  need an __init like section for h2
	for (i = 0; i < MAX_TLS_ENTRIES; i++) {
		mutex_lock(&tls_entries[i].lock);
		if (tls_entries[i].used == 0) {
			tls_entries[i].used = 1;
			*key = (void *)&tls_entries[i];
			mutex_unlock(&tls_entries[i].lock);
			return 0;
		}
		mutex_unlock(&tls_entries[i].lock);
	}
	return -1;
}

int sys_Tlsfree(void * key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	mutex_lock(&entry_ptr->lock);
	if (entry_ptr->used == 0) {
		mutex_unlock(&entry_ptr->lock);
		return -1;
	}
	entry_ptr->used = 0;
	entry_ptr->data = (void *)0;
	mutex_unlock(&entry_ptr->lock);
	return 0;
}

int sys_Tlsset(void * key, const void *p)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	mutex_lock(&entry_ptr->lock);
	if (entry_ptr->used == 0) {
		mutex_unlock(&entry_ptr->lock);
		return -1;
	}
	entry_ptr->data = (void *)p;
	mutex_unlock(&entry_ptr->lock);
	return 0;
}

void *sys_Tlsget(void *key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	mutex_lock(&entry_ptr->lock);
	if (entry_ptr->used == 0) {
		mutex_unlock(&entry_ptr->lock);
		return (void *)-1;
	}
	mutex_unlock(&entry_ptr->lock);
	return entry_ptr->data;
}

