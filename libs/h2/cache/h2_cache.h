/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CACHE_H
#define H2_CACHE_H 1

/** @file h2_cache.h
 * 
 * @brief Cache operations
 */

/** @addtogroup h2 
@{ */

/**
Flush address from data cache to memory.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/

static inline void h2_dccleana(void *ptr)
{
	asm volatile (" dccleana(%0) " : :"r"(ptr) : "memory");
}

/**
Flush address from data cache to memory and invalidate.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/

static inline void h2_dccleaninva(void *ptr)
{
	asm volatile (" dccleaninva(%0) " : :"r"(ptr) : "memory");
}

/**
Invalidate address in data cache.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/

static inline void h2_dcinva(void *ptr)
{
	asm volatile (" dcinva(%0) " : :"r"(ptr) : "memory");
}

#endif
