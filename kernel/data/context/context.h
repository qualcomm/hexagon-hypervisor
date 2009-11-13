/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_CONTEXT_H
#define BLAST_CONTEXT_H

#include <c_std.h>

typedef struct _blast_thread_context
{
	/* Kernel Variables */
	/* Make ring compatible */
	struct _blast_thread_context *next;
	struct _blast_thread_context *prev;
	// #8
	/* Other info */
	union {
		struct {
			u32_t *futex_ptr;
			u32_t futex_val;
		};
	};
	// #16
	/* What ring am I in? */
	//struct _blast_thread_context **queue;
	/* Callee-save context... */
	void *continuation;
	u8_t prio;
	u8_t hthread;
	u8_t valid;
	// 24
	/* status, etc */
	/* Context */
	/* Context required for OS calls... callee save + ugp/gp/etc */
	/* V3 callee-save is a lot bigger. :-| */
	u64_t r3130;
	// 32
	u64_t ssrelr;	// OK FOR DCZEROA
	u64_t r2928;
	u64_t r2726;
	u64_t r2524;
	// 64
	u64_t r2322;	// OK FOR DCZEROA
	u64_t r2120;
	u64_t r1918;
	u64_t r1716;
	// 96
	u64_t ugpgp;	// OK FOR DCZEROA
	u64_t r0100;	/* used for return value */
	/* Context required for interrupts... everything else */
	u64_t r1514;
	u64_t r1312;
	// 128
	u64_t r0706;	// OK FOR DCZEROA
	u64_t r0504;
	u64_t r1110;
	u64_t r0908;
	// 160
	u64_t r0302;	// OK FOR DCZEROA
	u64_t lc0sa0;
	u64_t lc1sa1;
	u64_t m1m0;
	// 192
	u64_t sr_preds;	// OK FOR DCZEROA
	u64_t pad0;
	u64_t pad1;
	u64_t pad2;
	// 224
	u64_t pad3;	// NO DCZEROA -- need totalcycles
	struct {
		void *event_handler;
		u32_t pad4;
	}
	u64_t oncpu_start;
	u64_t totalcycles;
	// 256
} __attribute__((aligned(32))) BLASTK_thread_context;

#endif
