/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_INFO_H
#define H2_COMMON_INFO_H 1

typedef enum {
	INFO_BUILD_ID,    /**< Build identifier */
	INFO_BOOT_FLAGS,  /**< Boot flags */
	INFO_STLB,        /**< STLB configuration */
	INFO_SYSCFG,      /**< SYSCFG register */
	INFO_MAX
} info_type;

typedef struct {
	unsigned long boot_flags;
	unsigned long boot_use_tcm:1;  /**< Hypervisor in TCM? */
	unsigned long boot_unused:31;
} info_boot_flags_type;

typedef struct {
	unsigned long stlb_config;
	unsigned long stlb_max_sets_log2:8;  /**< Sets per ASID */
	unsigned long stlb_max_ways:8;
	unsigned long stlb_size:8;           /**< Multiple of sets * ways */
	unsigned long stlb_unused:7;
	unsigned long stlb_enabled:1;
} info_stlb_type;

#endif
