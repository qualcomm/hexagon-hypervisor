/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return ENOTSUP;
}

