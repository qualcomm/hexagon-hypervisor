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
	INFO_LIVELOCK,    /**< LIVELOCK register */
	INFO_REV,         /**< REV register */
	INFO_SSBASE,      /**< subsystem base from cfg_table */
	INFO_TLB_FREE,    /**< Number of replaceable TLB entries */
	INFO_TLB_SIZE,    /**< Number of TLB entries */
	INFO_PHYSADDR,    /**< Kernel physical address */
	INFO_TCM_BASE,    /**< TCM base address */
	INFO_L2MEM_SIZE,  /**< L2 cache array size */
	INFO_TCM_SIZE,    /**< TCM size */
	INFO_H2K_PGSIZE,  /**< Kernel page size */
	INFO_H2K_NPAGES,  /**< Number of kernel pages */
	INFO_L2VIC_BASE,  /**< Interrupt controller physical address */
	INFO_TIMER_BASE,  /**< Timer physical address */
	INFO_TIMER_INT,   /**< Timer interrupt number */
	INFO_ERROR,       /**< Kernel error */
	INFO_HTHREADS,    /**< Running hw threads mask */
	INFO_L2TAG_SIZE,  /**< L2 cache tag size */
	INFO_L2CFG_BASE,  /**< L2 regs base */
	INFO_CLADE_BASE,  /**< CLADE regs base */
	INFO_MAX
} info_type;

typedef union {
	struct {
		unsigned long boot_use_tcm:1;  /**< Hypervisor in TCM? */
		unsigned long boot_have_hvx:1;
		unsigned long boot_unused:30;
	};
	unsigned long raw;
} info_boot_flags_type;

typedef union {
	struct {
		unsigned long stlb_max_sets_log2:8;  /**< Sets per ASID */
		unsigned long stlb_max_ways:8;
		unsigned long stlb_size:8;           /**< Multiple of sets * ways */
		unsigned long stlb_unused:7;
		unsigned long stlb_enabled:1;
	};
	unsigned long raw;
} info_stlb_type;

#endif
