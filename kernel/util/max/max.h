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

