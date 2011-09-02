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

/** 
Constants for choosing a cache to change partitioning on
*/
enum {
	HWCONFIG_PARTITION_D,	/**< Select the Data Cache */
	HWCONFIG_PARTITION_I,	/**< Select the Instruction Cache */
	HWCONFIG_PARTITION_L2	/**< Select the L2 Cache */
};

/** 
Constants for choosing a partitioning type
*/
enum {
	HWCONFIG_PARTITION_SHARED,	/**< Select fully shared */
	HWCONFIG_PARTITION_AUX_HALF,	/**< Select Half and Half partition */
	HWCONFIG_PARTITION_AUX_QUARTER,	/**< Select 1/4 and 3/4 partition */
	HWCONFIG_PARTITION_AUX_EIGHTH	/**< Select 1/8 and 7/8 partition */
};

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

int h2_hwconfig_trap(unsigned int whichtrap, void *ptr, unsigned int a, unsigned int b);

/** 
Configure partitioning.
@param[in] whichcache	Select the cache to change partitioning on
@param[in] partition_cfg	What type of partitioning for the specified cache
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_partition(unsigned int whichcache, unsigned int partition_cfg)
{
	return h2_hwconfig_trap(1,NULL,whichcache,partition_cfg);
}

/** 
Configure the L2 cache size.
@param[in] sizeval	Size to configure the L2 cache
@param[in] use_wb	1 enables write-back mode in the L2, 0 disables write-back mode
@returns 0 on success, negative value on error
@dependencies May return failure if more than one thread is currently running
*/

static inline int h2_hwconfig_l2cache_size(unsigned int sizeval, unsigned int use_wb)
{
	return h2_hwconfig_trap(0,NULL,sizeval,use_wb);
}

/** @} */

#endif

