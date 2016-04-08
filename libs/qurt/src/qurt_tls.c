/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//#include "qurt_mutex.h"
#include "qurt.h"
#include <pthread.h>
#include <hexagon_protos.h>

/* EJP: FIXME: replace with static inlines */
 
/**
 * Description: Allocates a TLS key
 *
 * @param   *key  - the allocated key value
 *          *destructor - callback function when a thread exits.
 * @return  0      - SUCCESS
 *          EAGAIN - No free TLS key available
 */
int qurt_tls_create_key (int *key, void (*destructor)(void *))
{
	return pthread_key_create(key,destructor);
}

/**
 * Description: Delete a TLS key
 *
 * @param   key  - Free the allocated key
 * @return  0      - SUCCESS
 *          ENOENT - The key is not already free
 */
int qurt_tls_delete_key (int key)
{
	return pthread_key_delete(key);
}

/**
 * Description: Sets a value in TLS indexed by key
 *
 * @param   key   - Operated TLS key
 *          value - data to save at the specified key location.
 * @return  0        - SUCCESS
 *          EINVALID - Not a valid key
 */
int qurt_tls_set_specific (int key, const void *value)
{
	return pthread_setspecific(key,value);
}

/**
 * Description: Gets a value in TLS indexed by key
 *
 * @param   key   - Operated TLS key
 * @return  value saved in TLS indexed by key
 */
void *qurt_tls_get_specific (int key)
{
	return pthread_getspecific(key);
}

