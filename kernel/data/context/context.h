/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONTEXT_H
#define H2K_CONTEXT_H

#include <c_std.h>

#define H2K_CONTEXT_ALIGN 32

enum {
	H2K_STATUS_DEAD = 0,
	H2K_STATUS_RUNNING,
	H2K_STATUS_READY,
	H2K_STATUS_BLOCKED,
};

#define H2K_VMSTATUS_VMWORK_BIT  0
#define H2K_VMSTATUS_KILL_BIT   1
#define H2K_VMSTATUS_IE_BIT     7
#define H2K_VMSTATUS_VMWORK	(0x01 << (H2K_VMSTATUS_VMWORK_BIT))
#define H2K_VMSTATUS_KILL	(0x01 << (H2K_VMSTATUS_KILL_BIT))
#define H2K_VMSTATUS_IE		(0x01 << (H2K_VMSTATUS_IE_BIT))

typedef struct _h2_thread_context
{
	/* Kernel Variables */
	/* Make ring compatible */
	struct _h2_thread_context *next;
	struct _h2_thread_context *prev;
	// #8
	/* Other info */
	u8_t prio;			// could be 5 bits
	u8_t hthread;
	u8_t tid;
	u8_t status;
	// #12
	union {				// must be updated with LL/SC?
		u32_t atomic_status_word;
		struct {
			u8_t vmstatus;
			u8_t tmpprio;
			u8_t vmcpu;
			u8_t u8pad0;
		};
	};
	// #16
	struct {
		void *gevb;
		u32_t trapmask;
	};
	// 24
	/* status, etc */
	/* Context */
	/* Context required for OS calls... callee save + ugp/gp/etc */
	/* V3 callee-save is a lot bigger. :-| */
	union {
		u64_t gelr_gbadva;
		struct {
			u32_t gbadva;
			u32_t gelr;
		};
	};
	// 32
	union {
		u64_t gssr_gosp;
		struct {
			u32_t gosp;
			u32_t gssr;
		};
	};
	u64_t oncpu_start;	/* Could be unioned for use only while running... */
	u64_t totalcycles;
	struct {
		u32_t ccr;	/* Could be moved to zeroed area */
		// u32_t gptb;	/* can look it up from asid table... */
		struct H2K_vmblock_struct *vmblock;
	};
	// 64
	struct {	// OK FOR DCZEROA
		u32_t *futex_ptr;
		void *continuation;
	};
	union {
		u64_t ssrelr;
		struct {
			u32_t elr;
			union {
				u32_t ssr;
				struct {
					u8_t ssr_cause;
					u8_t ssr_asid;
					u8_t ssr_um:1;
					u8_t ssr_ex:1;
					u8_t ssr_ie:1;
					u8_t ssr_incompat:5;
					u8_t ssr_highbyte;
				};
			};
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
	u64_t r2726;	// OK FOR DCZEROA
	u64_t r2524;
	u64_t r2322;
	u64_t r2120;
	// 128
	u64_t r1918;	// OK FOR DCZEROA
	u64_t r1716;
	u64_t ugpgp;
	u64_t r0100;	/* used for return value */
	/* Context required for interrupts... everything else */
	/* Note: Fast Interrupt contexts don't need these (can't be interrupted) */
	// 160
	u64_t r1514;	// OK FOR DCZEROA
	u64_t r1312;
	u64_t r1110;
	u64_t r0908;
	// 192
	u64_t r0706;	// OK FOR DCZEROA
	u64_t r0504;
	u64_t r0302;
	u64_t lc0sa0;
	// 224
	u64_t lc1sa1;	// OK FOR DCZEROA
	u64_t m1m0;
	u64_t sr_preds;
	u64_t cs1cs0;	// V4 regs
	// 256
} __attribute__((aligned(H2K_CONTEXT_ALIGN))) H2K_thread_context;

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
