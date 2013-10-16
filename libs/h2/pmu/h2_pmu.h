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

#define PMUEVTCFG (-1)			/**< Event Configuration Register */
#define PMUCNT0 0			/**< PMU Performance Counter 0 */
#define PMUCNT1 1			/**< PMU Performance Counter 1 */
#define PMUCNT2 2			/**< PMU Performance Counter 2 */
#define PMUCNT3 3			/**< PMU Performance Counter 3 */

#define H2_PMUCONFIG_SETREG 1		/**< Operation: write a register */
#define H2_PMUCONFIG_GETREG 2		/**< Operation: read a register */
#define H2_PMUCONFIG_THREADSET 0	/**< Operation: Enable PMU Counting for the specified thread */

/**
PMU Configuration Trap Interface.  Please do not use this directly, instead use the other h2_pmu functions.
@param[in] configtype		Type of operation
@param[in] val1			Generic Value
@param[in] val2			Generic Value
@param[in] val3			Generic Value
@returns 0 on success, nonzero otherwise 
*/

int h2_pmuconfig_trap(int configtype, int val1, int val2, int val3);

/**
Write PMU register
@param[in] reg			Register to write
@param[in] val			Value to write
@returns 0 on success, nonzero otherwise 
*/

static inline int h2_pmu_setreg(int reg, int val)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_SETREG, 0, reg, val);
}

/**
Read PMU register
@param[in] reg			Register to read
@returns Value read
*/

static inline int h2_pmu_getreg(int reg)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_GETREG, 0, reg, 0);
}

/**
Enable PMU monitoring for a thread
@param[in] threadid		ID of a thread to enable performance monitoring
@returns 0 on success, nonzero otherwise 
*/

static inline int h2_pmu_enable(int threadid)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_THREADSET, threadid, 1, 0);
}

/**
Disable PMU monitoring for a thread
@param[in] threadid		ID of a thread to enable performance monitoring
@returns 0 on success, nonzero otherwise 
*/

static inline int h2_pmu_disable(int threadid)
{
	return h2_pmuconfig_trap(H2_PMUCONFIG_THREADSET, threadid, 0, 0);
}

/** @} */

#endif
