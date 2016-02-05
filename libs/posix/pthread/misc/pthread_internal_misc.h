/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_INTERNAL_MISC_H
#define PTHREAD_INTERNAL_MISC_H 1

#include <pthread.h>

void pthread_safe_death(pthread_plainmutex_t *unlock, pthread_t id) __attribute__((noreturn));

#endif
