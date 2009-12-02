/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_CONTEXT_H
#define BLAST_CONTEXT_H

#include <c_std.h>

#define BLASTK_CONTEXT_ALIGN 32

typedef struct _blast_thread_context
{
	/* Kernel Variables */
	/* Make ring compatible */
	struct _blast_thread_context *next;
	struct _blast_thread_context *prev;
	// #8
	/* Other info */
	u32_t trapmask;
	u8_t prio;
	u8_t tmpprio;
	u8_t hthread;
	u8_t valid;
	// #16
	struct {
		void *event_handler;
		u32_t tid;
	};
	// 24
	/* status, etc */
	/* Context */
	/* Context required for OS calls... callee save + ugp/gp/etc */
	/* V3 callee-save is a lot bigger. :-| */
	struct {
		u32_t GBADVA;
		u32_t GSSR;
	};
	// 32
	struct {
		u32_t GOSP;
		u32_t GELR;
	};
	u64_t oncpu_start;
	u64_t totalcycles;
	u64_t pad1;
	// 64
	struct {	// OK FOR DCZEROA
		u32_t *futex_ptr;
		void *continuation;
	};
	u64_t ssrelr;
	u64_t r3130;
	u64_t r2928;
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
} __attribute__((aligned(BLASTK_CONTEXT_ALIGN))) BLASTK_thread_context;

typedef struct {
	BLASTK_thread_context context;
	unsigned long long int stack120;
	unsigned long long int stack112;
	unsigned long long int stack104;
	unsigned long long int stack096;
	unsigned long long int stack088;
	unsigned long long int stack080;
	unsigned long long int stack072;
	unsigned long long int stack064;
	unsigned long long int stack056;
	unsigned long long int stack048;
	unsigned long long int stack040;
	unsigned long long int stack032;
	unsigned long long int stack024;
	unsigned long long int stack016;
	unsigned long long int stack008;
	unsigned long long int stack000;
} BLASTK_fastint_context;

#endif
