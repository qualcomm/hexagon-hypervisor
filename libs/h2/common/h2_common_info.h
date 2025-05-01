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
	INFO_CFGBASE,     /**< cfgbase register */
	INFO_HVX_VLENGTH, /**< HVX native (no v2x) vector length, in bytes */
	INFO_COPROC_CONTEXTS,/**< Number of HVX/SILVER contexts (no v2x) */
	INFO_HVX_SWITCH,  /**< HVX context switch in kernel active */
	INFO_VTCM_BASE,   /**< VTCM base address */
	INFO_VTCM_SIZE,   /**< VTCM size */
	INFO_ECC_BASE,    /**< ECC base */
	INFO_L2_LINE_SZ,  /**< L2 cache line size */
	INFO_AUDIO_EXT,   /**< Audio extension type */
	INFO_VTCM_BANK_WIDTH, /**< VTCM bank width */
	INFO_L1D_SIZE,    /**< L1data cache size */
	INFO_MAX_CLUSTER_COPROC, /**< Max coprocessor threads per cluster */
	INFO_HMX_INSTANCES, /**< Number of HMX instances */
	INFO_CORECFG_BASE,/**< Core regs base */
	INFO_HLX_CONTEXTS, /**< Number of HLX instances */
	INFO_UNIT_START,  /**< Start of unit config list */
	INFO_UNIT_ENTRY,  /**< Unit entry */
	INFO_CORE_ID,     /**< Multicore core ID */
	INFO_CORE_COUNT,  /**< Multicore core count */
	INFO_SHIFT,       /**< Multicore shift - offset translation for boot VM */
	INFO_TCM_OFFSET,  /**< Multicore TCM base offset */
	INFO_MAX
} info_type;

#define INFO_HVX_CONTEXTS (INFO_COPROC_CONTEXTS)  // compat
#define INFO_MAX_CLUSTER_HVX (INFO_MAX_CLUSTER_COPROC)  // compat

typedef union {
	struct {
		unsigned long boot_use_tcm:1;     /**< Hypervisor in TCM? */
		unsigned long boot_have_hvx:1;    /**< Core has HVX? */
		unsigned long boot_have_sample:1; /**< PC sampling enabled ? */
		unsigned long boot_ext_ok:1;      /**< Kernel can switch extended contexts? */
		unsigned long boot_have_dma:1;    /**< Core has user-mode DMA? */
		unsigned long boot_have_hmx:1;    /**< Core has HMX? */
		unsigned long boot_have_silver:1; /**< Core has SILVER? */
		unsigned long boot_have_hlx:1;    /**< Core has HLX? */
		unsigned long boot_unused:24;
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
