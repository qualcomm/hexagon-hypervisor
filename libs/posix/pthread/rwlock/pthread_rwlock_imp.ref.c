/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <pthread_rwlock.h>

int pthread_condattr_init(pthread_condattr_t *attr) { attr->unused = 0; return 0; }
int pthread_condattr_destroy(pthread_condattr_t *attr) { return 0; }
int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) { return 0; }

int pthread_rwlock_init(pthread_rwlock_t *lock, pthread_rwlock_attr_t *attr) { lock->raw = 0; return 0; }