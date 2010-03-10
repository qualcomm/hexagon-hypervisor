/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CYCLES_H
#define H2_CYCLES_H 1

unsigned long long int h2_get_pcycles(void);

#if __QDSP6_ARCH__ <= 3
#define H2_CYCLES__PER_THREAD 6
#elif __QDSP6_ARCH__ == 4
#define H2_CYCLES__PER_THREAD 3
#else
#error define cycles per thread
#endif

static inline unsigned long long int h2_get_tcycles(void) { return h2_get_pcycles()/H2_CYCLES__PER_THREAD; }

unsigned long long int h2_get_core_pcycles(void);

#endif

