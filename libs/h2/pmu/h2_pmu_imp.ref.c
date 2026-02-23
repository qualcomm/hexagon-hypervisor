/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_pmu_imp.ref.c
 * 
 * @brief Interaction with the Performance Monitoring Unit - Implementation
 */

#include "h2_pmu.h"
#include "h2_hwconfig.h"

int h2_pmu_setreg(int reg, int val)
{
	return (int)h2_pmuctrl_trap(PMUCTRL_SETREG, 0, reg, val);
}

unsigned int h2_pmu_getreg(int reg)
{
	unsigned int ret;

	switch (reg) {
	case H2_PMUCNT0:
		asm volatile (" %0 = g26 " : "=r"(ret));
		return ret;
	case H2_PMUCNT1:
		asm volatile (" %0 = g27 " : "=r"(ret));
		return ret;
	case H2_PMUCNT2:
		asm volatile (" %0 = g28 " : "=r"(ret));
		return ret;
	case H2_PMUCNT3:
		asm volatile (" %0 = g29 " : "=r"(ret));
		return ret;
	case H2_PMUCNT4:
		asm volatile (" %0 = g16 " : "=r"(ret));
		return ret;
	case H2_PMUCNT5:
		asm volatile (" %0 = g17 " : "=r"(ret));
		return ret;
	case H2_PMUCNT6:
		asm volatile (" %0 = g18 " : "=r"(ret));
		return ret;
	case H2_PMUCNT7:
		asm volatile (" %0 = g19 " : "=r"(ret));
		return ret;
	default:
		return h2_pmuctrl_trap(PMUCTRL_GETREG, 0, reg, 0);
	}
}

int h2_pmu_count_clear() {
	int i;
	int ret;
	
	for (i = 0; i < 8; i++) {
		if (0 != (ret = h2_pmu_setreg(H2_PMUCNT0 + i, 0))) return ret;
	}

	return 0;
}

int h2_pmu_reset() {
	int ret;
	
	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG, 0))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG1, 0))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUCFG, 0))) return ret;

	return h2_pmu_count_clear();
}

int h2_pmu_enable() {
	int ret;
	
	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG, __h2_pmu_evtcfg__))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG1, __h2_pmu_evtcfg1__))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUCFG, __h2_pmu_cfg__))) return ret;

	if (__h2_gpio_toggle__) {
		if (0 != (ret = h2_hwconfig_gpio_toggle(1))) return ret;
	}

	return 0;
}

int h2_pmu_disable() {
	int ret;

	if (__h2_gpio_toggle__) {
		if (0 != (ret = h2_hwconfig_gpio_toggle(0))) return ret;
	}

	/* save */
	__h2_pmu_evtcfg__ = h2_pmu_getreg(H2_PMUEVTCFG);
	__h2_pmu_evtcfg1__ = h2_pmu_getreg(H2_PMUEVTCFG1);
	__h2_pmu_cfg__ = h2_pmu_getreg(H2_PMUCFG);

	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG, 0))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUEVTCFG1, 0))) return ret;
	if (0 != (ret = h2_pmu_setreg(H2_PMUCFG, 0))) return ret;

	return 0;
}
