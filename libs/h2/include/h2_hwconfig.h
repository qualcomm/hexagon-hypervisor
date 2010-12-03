/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_HWCONFIG_H
#define H2_HWCONFIG_H 1

#include <stdlib.h>

enum {
	HWCONFIG_PARTITION_D,
	HWCONFIG_PARTITION_I,
	HWCONFIG_PARTITION_L2
};

enum {
	HWCONFIG_PARTITION_SHARED,
	HWCONFIG_PARTITION_AUX_HALF,
	HWCONFIG_PARTITION_AUX_QUARTER,
	HWCONFIG_PARTITION_AUX_EIGHTH
};

void h2_hwconfig_trap(unsigned int whichtrap, void *ptr, unsigned int a, unsigned int b);

static inline void h2_hwconfig_partition(unsigned int whichcache, unsigned int partition_cfg)
{
	h2_hwconfig_trap(1,NULL,whichcache,partition_cfg);
}

static inline void h2_hwconfig_l2cache_size(unsigned int sizeval, unsigned int use_wb)
{
	h2_hwconfig_trap(0,NULL,sizeval,use_wb);
}

#endif

