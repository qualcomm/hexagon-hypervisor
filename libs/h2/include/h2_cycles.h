/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CYCLES_H
#define H2_CYCLES_H 1

unsigned long long int h2_get_pcycles(void);

#if __QDSP6_ARCH__ <= 3
#define H2_CYCLES__PER_THREAD 6
#elif __QDSP6_ARCH__ >= 4
#define H2_CYCLES__PER_THREAD 3
#else
#error define cycles per thread
#endif

static inline unsigned long long int h2_get_tcycles(void) { return h2_get_pcycles()/H2_CYCLES__PER_THREAD; }

unsigned long long int h2_get_core_pcycles(void);

static inline void h2_profile_enable(int enable) { return; }

static inline void h2_profile_reset_idle_pcycles(void) { return; }
static inline void h2_profile_reset_thread_pcycles(int thread_id) { return; }

static inline void h2_profile_get_idle_pcycles(unsigned long long *pcycles) { return; }
static inline void h2_profile_get_thread_pcycles(int thread_id, unsigned long long  *pcycles) { *pcycles = h2_get_tcycles(); return; }

#endif

