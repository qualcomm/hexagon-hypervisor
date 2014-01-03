/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_PMU_H
#define H2_COMMON_PMU_H 1

typedef enum {
	PMUCTRL_THREADSET,  /**< Operation: Enable PMU Counting for the specified thread */
	PMUCTRL_SETREG,     /**< Operation: write a register */
	PMUCTRL_GETREG,     /**< Operation: read a register */
	PMUCTRL_MAX
} pmuop_type;

#endif
