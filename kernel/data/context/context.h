/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_CONTEXT_H
#define BLAST_CONTEXT_H

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
			unsigned int *futex_ptr;
			unsigned int futex_val;
		};
		long long int intwait;
		unsigned long long int oncpu_start;
		struct {
			unsigned int error_badva;
			unsigned int error_cause;
		};
	};
	// #16
	/* What ring am I in? */
	//struct _blast_thread_context **queue;
	/* Callee-save context... */
	void *continuation;
	unsigned char prio;
	unsigned char hthread;
	unsigned char valid;
	// 24
	/* status, etc */
	/* Context */
	/* Context required for OS calls... callee save + ugp/gp/etc */
	/* V3 callee-save is a lot bigger. :-| */
	long long int r3130;
	// 32
	long long int ssrelr;	// OK FOR DCZEROA
	long long int r2928;
	long long int r2726;
	long long int r2524;
	// 64
	long long int r2322;	// OK FOR DCZEROA
	long long int r2120;
	long long int r1918;
	long long int r1716;
	// 96
	long long int ugpgp;	// OK FOR DCZEROA
	long long int r0100;	/* used for return value */
	/* Context required for interrupts... everything else */
	long long int r1514;
	long long int r1312;
	// 128
	long long int r0706;	// OK FOR DCZEROA
	long long int r0504;
	long long int r1110;
	long long int r0908;
	// 160
	long long int r0302;	// OK FOR DCZEROA
	long long int lc0sa0;
	long long int lc1sa1;
	long long int m0m1;
	// 192
	long long int sr_preds;	// OK FOR DCZEROA
	long long int pad0;
	long long int pad1;
	long long int pad2;
	// 224
	long long int pad3;	// NO DCZEROA -- need totalcycles
	long long int pad4;
	long long int pad5;
	unsigned long long int totalcycles;
	// 256
} __attribute__((aligned(32))) BLASTK_thread_context;

/* switch to a new thread */
/* If from is NULL, we're switching from idle */
/* If to is NULL, we're switching to idle... unlock bkl and sleep */
void BLASTK_thread_switch(BLASTK_thread_context *fromthread, BLASTK_thread_context *newthread);
/* Fake the stack on a context pointer for a new thread so that we can switch to it later */
void BLASTK_fake_stack(BLASTK_thread_context *thread);

#endif
