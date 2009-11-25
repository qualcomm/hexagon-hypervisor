/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_LIBKERNEL_H
#define BLAST_LIBKERNEL_H 1

#define BLAST_PREINIT_L2CACHE_SIZE_0K 0
#define BLAST_PREINIT_L2CACHE_SIZE_64K 1
#define BLAST_PREINIT_L2CACHE_SIZE_128K 2
#define BLAST_PREINIT_L2CACHE_SIZE_256K 3

void blast_preinit_hthread_startup(unsigned int mask);
void blast_preinit_l2cache_size(int size);
void blast_preinit_relocate_tcm(void *addr);
void blast_init(unsigned long long int *memmap);

#endif

