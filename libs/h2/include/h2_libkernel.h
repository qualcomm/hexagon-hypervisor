/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_LIBKERNEL_H
#define H2_LIBKERNEL_H 1

#define H2_PREINIT_L2CACHE_SIZE_0K 0
#define H2_PREINIT_L2CACHE_SIZE_64K 1
#define H2_PREINIT_L2CACHE_SIZE_128K 2
#define H2_PREINIT_L2CACHE_SIZE_256K 3

void h2_preinit_hthread_startup(unsigned int mask);
void h2_preinit_l2cache_size(int size);
void h2_preinit_relocate_tcm(void *addr);
void h2_init(unsigned long long int *memmap);

#endif

