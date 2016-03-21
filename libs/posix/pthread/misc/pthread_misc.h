/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_MISC_H
#define PTHREAD_MISC_H 1
/* MISC */

typedef unsigned int pthread_once_t;
#define PTHREAD_ONCE_INIT 0

typedef unsigned int sigset_t;

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
static inline int pthread_getconcurrency(int new_level) { return 0; }
static inline int pthread_setconcurrency(void) { return 0; }
int pthread_kill(pthread_t threawd, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);

/* Simulation HACK */
static inline unsigned long long int h2_get_elapsed_nanos(void) {
  /* One nanosecond per cycle takes too long.  Instead, do
     ~1 microsecond per cycle. */
  return h2_get_core_pcycles() << 10;
}

#endif
