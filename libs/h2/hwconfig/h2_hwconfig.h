/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_HWCONFIG_H
#define H2_HWCONFIG_H 1

#include <stdlib.h>

/** @file h2_hwconfig.h
 @brief Hardware Configuration Interface
*/
/** @addtogroup h2 
@{ */

#include <h2_common_hwconfig.h>

/** 
Raw interface for the hwconfig trap.  Do not use.  Instead, use an interface function such
as h2_hwconfig_partition().
@param[in] whichtrap	Which hardware configuration trap to use
@param[in] ptr		Address argument for trap
@param[in] a		First argument
@param[in] b		Second argument
@returns 0 on success, negative value on error
@dependencies None
*/

int h2_hwconfig_trap(hwconfig_type_t whichtrap, void *ptr, unsigned int a, unsigned int b);

/** 
Configure the L2 cache size.
@param[in] sizeval	Size to configure the L2 cache
@param[in] use_wb	1 enables write-back mode in the L2, 0 disables write-back mode
@returns 0 on success, negative value on error
@dependencies May return failure if more than one thread is currently running
*/

static inline int h2_hwconfig_l2cache_size(unsigned int sizeval, unsigned int use_wb)
{
	return h2_hwconfig_trap(HWCONFIG_L2CACHE, NULL, sizeval, use_wb);
}

/** 
Configure partitioning.
@param[in] whichcache	Select the cache to change partitioning on
@param[in] partition_cfg	What type of partitioning for the specified cache
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_partition(unsigned int whichcache, unsigned int prefetch_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PARTITIONS, NULL, whichcache, prefetch_cfg);
}

/** 
Configure prefetch.
@param[in] whichcache	Select the cache to change partitioning on
@param[in] prefetch_cfg Prefetch type
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_prefetch(unsigned int whichcache, unsigned int partition_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PREFETCH, NULL, whichcache, partition_cfg);
}

#ifdef HAVE_EXTENSIONS
/** 
Set XA, XE bits.
@param[in] xa XA value
@param[in] xe XE value
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_extbits(unsigned int xa, unsigned int xe)
{
	return h2_hwconfig_trap(HWCONFIG_EXTBITS, NULL, xa, xe);
}
#endif

/** @} */

#endif

