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
Invalidate address in instruction cache
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/
void h2_icinva(void *ptr);

/**
Flush address from data cache to memory.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/
void h2_dccleana(void *ptr);

/**
Flush address from data cache to memory and invalidate.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/
void h2_dccleaninva(void *ptr);

/**
Invalidate address in data cache.
@param[in] ptr Address of word in memory
@returns Nothing
@dependencies None
*/
void h2_dcinva(void *ptr);

/**
clean range of memory from data cache hierarchy
@param[in] ptr Address of word in memory
@param[in] size number of bytes to clean
@returns Nothing
@dependencies None
*/

void h2_cache_dcclean_range(void *ptr, int bytes);

/**
clean and invalidate range of memory from data cache hierarchy
@param[in] ptr Address of word in memory
@param[in] size number of bytes to clean and invalidate
@returns Nothing
@dependencies None
*/
void h2_cache_dccleaninv_range(void *ptr, int bytes);

/**
invalidate range of memory from instruction cache
@param[in] ptr Address of word in memory
@param[in] size number of bytes to invalidate
@returns Nothing
@dependencies None
*/

void h2_cache_icinv_range(void *ptr, int bytes);

#endif
