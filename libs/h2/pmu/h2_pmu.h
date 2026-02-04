/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PMU_H
#define H2_PMU_H 1

/** @file h2_pmu.h
 @brief Interaction with the Performance Monitoring Unit
*/
/** @addtogroup h2 
@{ */

#include <stdlib.h>
#include <h2_common_pmu.h>

#define H2_PMUEVTCFG 8            /**< PMU Event Configuration Register */
#define H2_PMUEVTCFG1 9           /**< PMU Event Configuration Register 1 */
#define H2_PMUCFG   10            /**< PMU Configuration Register */
#define H2_PMUSTID0 11            /**< PMU STID0 Configuration Register */
#define H2_PMUSTID1 12            /**< PMU STID1 Configuration Register */
#define H2_PMUCNT0   0            /**< PMU Performance Counter 0 */
#define H2_PMUCNT1   1            /**< PMU Performance Counter 1 */
#define H2_PMUCNT2   2            /**< PMU Performance Counter 2 */
#define H2_PMUCNT3   3            /**< PMU Performance Counter 3 */
#define H2_PMUCNT4   4            /**< PMU Performance Counter 4 */
#define H2_PMUCNT5   5            /**< PMU Performance Counter 5 */
#define H2_PMUCNT6   6            /**< PMU Performance Counter 6 */
#define H2_PMUCNT7   7            /**< PMU Performance Counter 7 */

/* soft counters */
#define H2_TLBMISSX_LO  (-2)    /**< TLB execute miss low word */
#define H2_TLBMISSX_HI  (-3)    /**< TLB execute miss high word */
#define H2_TLBMISSRW_LO (-4)    /**< TLB read/write miss low word */
#define H2_TLBMISSRW_HI (-5)    /**< TLB read/write miss high word */
#define H2_STLBMISS_LO  (-6)    /**< STLB miss low word */
#define H2_STLBMISS_HI  (-7)    /**< STLB miss high word */

extern unsigned int __h2_pmu_evtcfg__;
extern unsigned int __h2_pmu_evtcfg1__;
extern unsigned int __h2_pmu_cfg__;
extern unsigned int __h2_gpio_toggle__;

/**
PMU Configuration Trap Interface.  Please do not use this directly, instead use the other h2_pmu functions.
@param[in] configtype		Type of operation
@param[in] val1			Generic Value
@param[in] val2			Generic Value
@param[in] val3			Generic Value
@returns 0 on success, nonzero otherwise 
*/

unsigned int h2_pmuctrl_trap(int configtype, int val1, int val2, int val3);

/**
Write PMU register
@param[in] reg			Register to write
@param[in] val			Value to write
@returns 0 on success, nonzero otherwise 
*/
int h2_pmu_setreg(int reg, int val);

/**
Read PMU register
@param[in] reg			Register to read
@returns Value read
*/
unsigned int h2_pmu_getreg(int reg);

int h2_pmu_count_clear(void);

int h2_pmu_reset(void);

int h2_pmu_enable(void);

int h2_pmu_disable(void);

/* /\** */
/* REMOVED: Enable PMU monitoring for a thread */
/* @param[in] threadid		ID of a thread to enable performance monitoring */
/* @returns 0 on success, nonzero otherwise  */
/* *\/ */

/* static inline int h2_pmu_enable(int threadid) */
/* { */
/* 	return -1; */
/* 	//return h2_pmuctrl_trap(PMUCTRL_THREADSET, threadid, 1, 0); */
/* } */

/* /\** */
/* REMOVED: Disable PMU monitoring for a thread */
/* @param[in] threadid		ID of a thread to enable performance monitoring */
/* @returns 0 on success, nonzero otherwise  */
/* *\/ */

/* static inline int h2_pmu_disable(int threadid) */
/* { */
/* 	return -1; */
/* 	// return h2_pmuctrl_trap(PMUCTRL_THREADSET, threadid, 0, 0); */
/* } */

/** @} */

#endif
