/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <hexagon_protos.h>

typedef void (*pthread_tls_destructor_t)(void *);

static pthread_tls_destructor_t pthread_tls_dtors[PTHREAD_KEYS_MAX];
unsigned long long int pthread_tls_key_valid = 0ULL;
pthread_plainmutex_t mutex = PTHREAD_PLAINMUTEX_INITIALIZER_NP;

static inline void ***get_pthread_tls_ptr()
{
	unsigned long ugp_val;
	asm volatile (" %0 = ugp " : "=r"(ugp_val) );
	return (void ***)(ugp_val + 4);
}

static inline void **get_pthread_tls()
{
	return *(get_pthread_tls_ptr());
}

static inline void set_pthread_tls(void **x)
{
	*(get_pthread_tls_ptr()) = x;
}

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	unsigned int i;
	int ret;
	pthread_plainmutex_lock_np(&mutex);
	i = Q6_R_ct1_P(pthread_tls_key_valid);
	if (i < 64) {
		pthread_tls_key_valid |= (1ULL << i);
		pthread_tls_dtors[i] = destructor;
		*key = i;
		ret = 0;
	} else {
		ret = EAGAIN;
	}
	pthread_plainmutex_unlock_np(&mutex);
	return ret;
}

int pthread_key_delete(pthread_key_t key)
{
	pthread_plainmutex_lock_np(&mutex);
	pthread_tls_dtors[key] = NULL;
	pthread_tls_key_valid &= ~(1ULL << key);
	pthread_plainmutex_unlock_np(&mutex);
	return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
	void **pthread_tls_ptr = get_pthread_tls();
	if (pthread_tls_ptr == NULL) return NULL;
	return pthread_tls_ptr[key];
}

int pthread_setspecific(pthread_key_t key, void *value)
{
	void **pthread_tls_ptr = get_pthread_tls();
	if (pthread_tls_ptr == NULL) {
		if ((pthread_tls_ptr = calloc(PTHREAD_KEYS_MAX,sizeof(*pthread_tls_ptr))) == NULL) {
			return ENOMEM;
		}
		set_pthread_tls(pthread_tls_ptr);
	}
	pthread_tls_ptr[key] = value;
	return 0;
}

/* This is called at the end of a thread to call destructors and free the tls data (if applicable) */
void pthread_tls_teardown()
{
	int i,j;
	void *old;
	void **pthread_tls_ptr = get_pthread_tls();
	if (pthread_tls_ptr == NULL) return;					/* No TLS data at all */
	for (j = 0; j < PTHREAD_DESTRUCTOR_ITERATIONS; j++) {
		for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
			if (pthread_tls_ptr[i] == NULL) continue;		/* No data */
			if ((pthread_tls_key_valid >> i) == 0) break;		/* Nothing in the future */
			if (((pthread_tls_key_valid >> i) & 1) == 0) continue;	/* Nothing at this location */
			if (pthread_tls_dtors[i] == NULL) continue;		/* No destructor */
			old = pthread_tls_ptr[i];
			pthread_tls_ptr[i] = NULL;
			pthread_tls_dtors[i](old);
		}
	}
	free(pthread_tls_ptr);
	pthread_tls_ptr = NULL;
	set_pthread_tls(pthread_tls_ptr);
}

