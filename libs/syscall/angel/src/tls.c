/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* dinkumware tls */

#define MAX_TLS_ENTRIES 64
typedef struct
{
	void *data;
	int used;
} tls_entry_t;

static tls_entry_t tls_entries[MAX_TLS_ENTRIES];

static int tls_init_done = 0;

static void tlsinit()
{
	int i;
	for (i = 0; i < MAX_TLS_ENTRIES; i++) {
		tls_entries[i].used = 0;
	}
	tls_init_done = 1;
}

int sys_Tlsalloc(void ** key, void (*dtor)(void *))
{
	unsigned int i;
	if (tls_init_done == 0) tlsinit();
	/* dtor is ignored unless c library need it */
	for (i = 0; i < MAX_TLS_ENTRIES; i++) {
		if (tls_entries[i].used == 0) {
			tls_entries[i].used = 1;
			*key = (void *)&tls_entries[i];
			return 0;
		}
	}
	return -1;
}

int sys_Tlsfree(void * key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (tls_init_done == 0) tlsinit();
	if (entry_ptr->used == 0) return -1;
	entry_ptr->used = 0;
	entry_ptr->data = (void *)0;
	return 0;
}

int sys_Tlsset(void * key, const void *p)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (tls_init_done == 0) tlsinit();
	if (entry_ptr->used == 0) return -1;
	entry_ptr->data = (void *)p;
	return 0;
}

void *sys_Tlsget(void *key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (tls_init_done == 0) tlsinit();
	if (entry_ptr->used == 0) return (void *)-1;
	return entry_ptr->data;
}

