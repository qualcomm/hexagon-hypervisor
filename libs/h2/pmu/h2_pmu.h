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

#define H2_PMUEVTCFG 8			/**< Event Configuration Register */
#define H2_PMUCFG 10			/**< Event Configuration Register */
#define H2_PMUCNT0 0	          /**< PMU Performance Counter 0 */
#define H2_PMUCNT1 1			      /**< PMU Performance Counter 1 */
#define H2_PMUCNT2 2			      /**< PMU Performance Counter 2 */
#define H2_PMUCNT3 3			      /**< PMU Performance Counter 3 */

/* soft counters */
#define H2_TLBMISSX_LO (-2)    /**< TLB execute miss low word */
#define H2_TLBMISSX_HI (-3)    /**< TLB execute miss high word */
#define H2_TLBMISSRW_LO (-4)   /**< TLB read/write miss low word */
#define H2_TLBMISSRW_HI (-5)   /**< TLB read/write miss high word */
#define H2_STLBMISS_LO (-6)    /**< STLB miss low word */
#define H2_STLBMISS_HI (-7)    /**< STLB miss high word */

/**
PMU Configuration Trap Interface.  Please do not use this directly, instead use the other h2_pmu functions.
@param[in] configtype		Type of operation
@param[in] val1			Generic Value
@param[in] val2			Generic Value
@param[in] val3			Generic Value
@returns 0 on success, nonzero otherwise 
*/

int h2_pmuctrl_trap(int configtype, int val1, int val2, int val3);

/**
Write PMU register
@param[in] reg			Register to write
@param[in] val			Value to write
@returns 0 on success, nonzero otherwise 
*/

static inline int h2_pmu_setreg(int reg, int val)
{
	return h2_pmuctrl_trap(PMUCTRL_SETREG, 0, reg, val);
}

/**
Read PMU register
@param[in] reg			Register to read
@returns Value read
*/

static inline int h2_pmu_getreg(int reg)
{
	return h2_pmuctrl_trap(PMUCTRL_GETREG, 0, reg, 0);
}

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
