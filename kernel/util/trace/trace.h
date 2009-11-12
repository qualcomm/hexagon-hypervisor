/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TRACE_H
#define TRACE_H 1

typedef struct {
	unsigned int word0;
	unsigned int word1;
	unsigned int word2;
	unsigned int word3;
} __attribute__((aligned(16))) BLASTK_trace_entry_t;

extern unsigned int BLASTK_trace_index;
extern BLASTK_trace_entry_t BLASTK_trace[MAX_TRACE_ENTRIES];

static inline void BLASTK_trace(short type, unsigned int arg1, unsigned int arg2, unsigned int arg3, unsigned int hwtnum)
{
	/* EJP: Fixme: non-atomic update of trace_index */
	unsigned int word0;
	int oldidx = BLASTK_trace_index;
	BLASTK_trace_index = oldidx + 1;
	if (BLASTK_trace_index >= MAX_TRACE_ENTRIES) BLASTK_trace_index = 0;
	word0 = Q6_R_combine_RlRl(hwtnum,type);
	BLASTK_trace[oldidx].word0 = word0;
	BLASTK_trace[oldidx].word1 = arg1;
	BLASTK_trace[oldidx].word2 = arg2;
	BLASTK_trace[oldidx].word3 = arg3;
}

#endif

