/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return ENOTSUP;
}

int pthread_getconcurrency(int new_level) { (void)new_level; return 0; }
int pthread_setconcurrency(void) { return 0; }
unsigned long long int h2_get_elapsed_nanos(void) {
  /* One nanosecond per cycle takes too long.  Instead, do
     ~1 microsecond per cycle. */
  return h2_get_core_pcycles() << 10;
}
