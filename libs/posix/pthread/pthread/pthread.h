/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _H2_PTHREAD_H
#define _H2_PTHREAD_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* EJP: Where we need to point to somewhere else, point to h2.
 * But other environments should point to here for primitives.
 */

#define _PROVIDE_POSIX_TIME_DECLS 1

#include <h2if.h>
#include <errno.h>
#include <sched.h>

#ifndef likely
#define PTHREAD_SELF_DEF_LIKELY 1
#define likely(x) (__builtin_expect(!!(x), 1))
#endif

#ifndef unlikely
#define PTHREAD_SELF_DEF_UNLIKELY 1
#define unlikely(x) (__builtin_expect(!!(x), 0))
#endif

static inline int pthread_unsup() { return ENOTSUP; }

#include <pthread_thread.h>
#include <pthread_plainmutex.h>
#include <pthread_mutex.h>
#include <pthread_cond.h>
#include <pthread_barrier.h>
#include <pthread_sem.h>
#include <pthread_misc.h>
#include <pthread_tls.h>

#ifdef PTHREAD_SELF_DEF_LIKELY
#undef PTHREAD_SELF_DEF_LIKELY
#undef likely
#endif

#ifdef PTHREAD_SELF_DEF_UNLIKELY
#undef PTHREAD_SELF_DEF_UNLIKELY
#undef unlikely
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

