/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_TLB_H
#define H2_TLB_H 1

#include <stdlib.h>

/** @file h2_hwconfig.h
 @brief Hardware Configuration Interface
*/
/** @addtogroup h2 
@{ */

#include <h2_common_tlb.h>

/**
Raw interface for the tlb trap.  Do not use.  Instead, use an interface function such
as h2_tlb_alloc().
@param[in] whichtrap	Which hardware configuration trap to use
@param[in] val32	32-bit value
@param[in] val64	64-bit value
@returns various values
@dependencies None
*/

long long int h2_tlb_trap(unsigned int whichtrap, unsigned int val32, unsigned long long int val64);

/**
Read TLB entry
@param[in] index  Index of TLB entry
@returns TLB entry
*/

static inline unsigned long long int h2_tlb_read(unsigned int index)
{
	return (unsigned long long)h2_tlb_trap(TLBOP_TLBREAD, index, 0);
}

/**
Write TLB entry
@param[in] index  Index of TLB entry
@param[in] entry  new TLB entry value
*/

static inline void h2_tlb_write(unsigned int index, unsigned long long int entry)
{
	h2_tlb_trap(TLBOP_TLBWRITE, index, entry);
}

/**
Search for TLB entry
@param[in] va  Virtual Address
@returns index on success, negative value if not found
*/

static inline int h2_tlb_query(unsigned long va)
{
	return (int)h2_tlb_trap(TLBOP_TLBQUERY, va, 0);
}

/**
Allocate TLB entry
@param[in] entry  TLB entry
@returns index on success, negative value if failure
*/

static inline int h2_tlb_alloc(unsigned long long int entry)
{
	return (int)h2_tlb_trap(TLBOP_TLBALLOC, 0, entry);
}

/**
Free TLB entry
@param[in] index  entry index
@returns zero on success, negative value if failure
*/

static inline int h2_tlb_free(int index)
{
	return (int)h2_tlb_trap(TLBOP_TLBFREE, (unsigned int)index, 0);
}

/**
Set DMA TLB entry
@param[in] index  index relative to start of DMA TLB
@param[in] entry  TLB entry
@returns absolute index on success, negative value if failure
*/

static inline int h2_tlb_dma_set(unsigned int index, unsigned long long int entry)
{
#if ARCHV >= 73
	return (int)h2_tlb_trap(TLBOP_DMASET, index, entry);
#else
	return -1;
#endif
}

#endif

/** @} */
