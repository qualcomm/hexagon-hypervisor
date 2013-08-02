/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_HWCONFIG_H
#define H2_COMMON_HWCONFIG_H 1

typedef enum {
	HWCONFIG_L2CACHE,
	HWCONFIG_PARTITIONS,
	HWCONFIG_PREFETCH,
	HWCONFIG_EXTBITS,
	HWCONFIG_MAX
} hwconfig_type_t;

enum {
HWCONFIG_PREFETCH_HF_I,
HWCONFIG_PREFETCH_HF_D,
HWCONFIG_PREFETCH_SF_D,
HWCONFIG_PREFETCH_HF_I_L2,
HWCONFIG_PREFETCH_SF_D_L2
};

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

#endif
