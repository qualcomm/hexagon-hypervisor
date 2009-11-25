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

int sys_Tlsalloc(void ** key, void (*dtor)(void *))
{
	unsigned int cnt;
	/* dtor is ignored unless c library need it */
	for (cnt = 0; cnt < MAX_TLS_ENTRIES; cnt++) {
		if (tls_entries[cnt].used == 0) {
			tls_entries[cnt].used = 1;
			*key = (void *)&tls_entries[cnt];
			return 0;
		}
	}
	return -1;
}

int sys_Tlsfree(void * key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (entry_ptr->used == 0) return -1;
	entry_ptr->used = 0;
	entry_ptr->data = (void *)0;
	return 0;
}

int sys_Tlsset(void * key, const void *p)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (entry_ptr->used == 0) return -1;
	entry_ptr->data = (void *)p;
	return 0;
}

void *sys_Tlsget(void *key)
{
	tls_entry_t *entry_ptr = (tls_entry_t *)key;
	if (entry_ptr->used == 0) return (void *)-1;
	return entry_ptr->data;
}

