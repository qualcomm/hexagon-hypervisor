/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_tlb_imp.ref.c
 * 
 * @brief TLB Interface - Implementation
 */

#include "h2_tlb.h"

unsigned long long int h2_tlb_read(unsigned int index)
{
	return (unsigned long long)h2_tlb_trap(TLBOP_TLBREAD, index, 0);
}

void h2_tlb_write(unsigned int index, unsigned long long int entry)
{
	h2_tlb_trap(TLBOP_TLBWRITE, index, entry);
}

int h2_tlb_query(unsigned long va)
{
	return (int)h2_tlb_trap(TLBOP_TLBQUERY, va, 0);
}

int h2_tlb_alloc(unsigned long long int entry)
{
	return (int)h2_tlb_trap(TLBOP_TLBALLOC, 0, entry);
}

int h2_tlb_free(int index)
{
	return (int)h2_tlb_trap(TLBOP_TLBFREE, (unsigned int)index, 0);
}

int h2_tlb_dma_set(unsigned int index, unsigned long long int entry)
{
	return (int)h2_tlb_trap(TLBOP_DMASET, index, entry);
}
