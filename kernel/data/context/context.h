/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONTEXT_H
#define H2K_CONTEXT_H

#include <c_std.h>
#include <tree.h>
#include <idtype.h>
#include <max.h>

#define H2K_CONTEXT_ALIGN 32

enum {
	H2K_STATUS_DEAD = 0,
	H2K_STATUS_RUNNING,
	H2K_STATUS_READY,
	H2K_STATUS_BLOCKED,
	H2K_STATUS_VMWAIT,
	H2K_STATUS_INTBLOCKED,
};

typedef struct _h2_thread_context
{
	/* Kernel Variables */
	/* Make ring compatible */
	struct _h2_thread_context *next;
	struct _h2_thread_context *prev;
	// #8
	/* Other info */
	union {
		struct {
			u8_t tid;
			u8_t hthread;			// could be < 8 bits
			u8_t prio;
			u8_t status;			// could be < 8 bits, combined with vmstatus?
		};
		u32_t status_prio_hthread_tid;
	};
	// #12
	union {				// must be updated with LL/SC?
		u32_t atomic_status_word;
		struct {
			u8_t vmstatus;
			u8_t base_prio;	// Does it need to be atomic?
			u8_t unusedab;	// use for i/o byte?  Add read/write valid bits below?
			u8_t pmu_on;	// Does it need to be atomic?  1 bit
		};
	};
	// #16
	union {
		u64_t vmblock_id;
		struct {
			H2K_id_t id;
			struct H2K_vmblock_struct *vmblock;	// could look up from GP + high bits of id
		};
	};
	// 24
	struct {
		void *gevb;		// isn't necessarily the same for all CPUs in a guest.  Also, we want it quickly.
		u32_t trapmask;		// Alread in VMblock?  Maybe move it?  All threads in a VM have same trap mask?
	};
	/* status, etc */
	/* Context */
	/* Context required for OS calls... callee save + ugp/gp/etc */
	/* Need to add per-cpu interrupts in here */
	// 32
	union {
		struct {
			u64_t rightleft;
			u64_t timeout;
		};
		H2K_treenode_t tree;
	};
	union {
		u64_t cpuint_enabled_pending;
		struct {
			u32_t cpuint_pending;
			u32_t cpuint_enabled;
		};
	};
	u64_t totalcycles;
	// 64
	struct {	// OK FOR DCZEROA
		u32_t futex_ptr;		// Probably not needed if interrupted; only on trap; could be unioned below?
		// needs to be pa_t, but is word aligned.  For 36 bits pa, can be 34 bits... 
		void *continuation;		// probably can be 30 bits.  34 bits for futex_ptr plus 30 bits for continuation fits.
	};
	union {
		u64_t ccrssr;
		struct {
			union {
				u32_t ssr;
				struct {
					u8_t ssr_cause;
#if ARCHV <= 3
					u8_t ssr_asid:5;
					u8_t ssr_guest:1; /* Fake guest bit using a bit of ASID */
					u8_t ssr_asid_byte_unused:2;
#else
					u8_t ssr_asid:7;
					u8_t ssr_asid_byte_unused:1;
#endif
					u8_t ssr_um:1;
					u8_t ssr_ex:1;
					u8_t ssr_ie:1;
#if ARCHV <= 3
					u8_t ssr_tnum:3;
					u8_t ssr_hfi:1;
					u8_t ssr_hfd:1;
					u8_t ssr_sfd:1;
					u8_t ssr_hi_rsvd:7;
#else
					u8_t ssr_guest:1;
					u8_t ssr_badva_v0:1;
					u8_t ssr_badva_v1:1;
					u8_t ssr_badva_bvs:1;
					u8_t ssr_badva_ce:1;
					u8_t ssr_badva_pe:1;
#ifdef HAVE_EXTENSIONS
					u8_t ssr_hi_rsvd:2;
					u8_t ssr_xa:3;
#else
					u8_t ssr_hi_rsvd:5;
#endif
					u8_t ssr_ss:1;
#ifdef HAVE_EXTENSIONS
					u8_t ssr_xe:1;
#else
					u8_t ssr_unused:1;
#endif

#endif
				};
			};
			u32_t ccr;
		};
	};
	u64_t r3130;
	union {
		u64_t r2928;
		struct {
			u32_t r28;
			u32_t r29;
		};
	};
	// 96
	u64_t r1918;	// OK FOR DCZEROA
	u64_t r1716;
	union {
		u64_t usrp30;
		struct {
			u32_t p30;
			u32_t usr;
		};
	};
	union {
		u64_t r0100;	/* used for return value */
		struct {
			u32_t r00;
			u32_t r01;
		};
	};
	/* Context required for interrupts... everything else */
	/* Note: SR really needed for any context switch.  */
	/* Note: Fast Interrupt contexts don't need these (can't be interrupted) */
	// 128
	u64_t r1514;	// OK FOR DCZEROA
	u64_t r1312;
	u64_t r1110;
	u64_t r0908;
	// 160
	u64_t r0706;	// OK FOR DCZEROA
	u64_t r0504;
	union {
		u64_t r0302;
		struct {
			u32_t r02;
			u32_t r03;
		};
	};
	u64_t lc0sa0;
	// 192
	union {
		u64_t gpugp;
		struct {
			u32_t ugp;
			u32_t gp;
		};
	};
	u64_t lc1sa1;	// OK FOR DCZEROA
	u64_t m1m0;
	union {
		u64_t reserved_elr;		// OK to move to cleared area?
		struct {
			u32_t elr;
			u32_t reserved_u32;
		};
	};
	// 224
	u64_t r2726;	// OK for dczeroa
	u64_t r2524;
	u64_t r2322;
	u64_t r2120;
	// 256
	union {
		u64_t gssr_gelr;	// OK to move to cleared area?
		struct {
			u32_t gelr;
			u32_t gssr;
		};
	};
	union {
		u64_t gbadva_gosp;	// OK to move to cleared area?
		struct {
			u32_t gosp;
			u32_t gbadva;
		};
	};
	u64_t cs1cs0;	// V4 regs
	u64_t reserved_u64_3;
	// 288
} __attribute__((aligned(H2K_CONTEXT_ALIGN))) H2K_thread_context;

typedef struct {
	u32_t vregs[32][8];
	u32_t qregs[8];
} __attribute__((aligned(H2K_CONTEXT_ALIGN))) H2K_ext_context;

typedef struct {
	H2K_thread_context context;
	u64_t stack120;
	u64_t stack112;
	u64_t stack104;
	u64_t stack096;
	u64_t stack088;
	u64_t stack080;
	u64_t stack072;
	u64_t stack064;
	u64_t stack056;
	u64_t stack048;
	u64_t stack040;
	u64_t stack032;
	u64_t stack024;
	u64_t stack016;
	u64_t stack008;
	u64_t stack000;
} H2K_fastint_context;

#endif
