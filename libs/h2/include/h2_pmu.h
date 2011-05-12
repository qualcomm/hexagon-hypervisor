/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PMU_H
#define H2_PMU_H 1

#include <stdlib.h>

#define PMUEVTCFG (-1)
#define PMUCNT0 0
#define PMUCNT1 1
#define PMUCNT2 2
#define PMUCNT3 3

#define H2_PMUCONFIG_SETREG 1
#define H2_PMUCONFIG_GETREG 2
#define H2_PMUCONFIG_THREADSET 0

int h2_pmuconfig_trap(int configtype, void *ptr, int val2, int val3);

static inline int h2_pmu_setreg(int reg, int val)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_SETREG,NULL,reg,val);
}

static inline int h2_pmu_getreg(int reg)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_GETREG,NULL,reg,0);
}

static inline int h2_pmu_enable(int threadid)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_THREADSET,(void *)threadid,1,0);
}

static inline int h2_pmu_disable(int threadid)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_THREADSET,(void *)threadid,0,0);
}

#endif
