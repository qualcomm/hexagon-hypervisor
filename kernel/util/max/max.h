/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#if __QDSP6_ARCH__ <= 3
#define MAX_HTHREADS 6
#elif __QDSP6_ARCH__ == 4
#define MAX_HTHREADS 3
#endif

#define MAX_THREADS 16

#define KERNEL_STACK_SIZE (8*16)

#define MAX_PRIOS 32

#define RESCHED_INT 3

#if __QDSP6_ARCH__ <= 3
#define RESCHED_INT_INTMASK (0x80000000 >> RESCHED_INT)
#else
#define RESCHED_INT_INTMASK (0x00000001 << RESCHED_INT)
#endif

#define SSR_DEFAULT 0x1c60000

