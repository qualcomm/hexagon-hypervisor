/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_PMU_H
#define H2_COMMON_PMU_H 1

typedef enum {
	PMUCONFIG_THREADSET,  /**< Operation: Enable PMU Counting for the specified thread */
	PMUCONFIG_SETREG,     /**< Operation: write a register */
	PMUCONFIG_GETREG,     /**< Operation: read a register */
	PMUCONFIG_MAX
} pmuop_t;

#endif
