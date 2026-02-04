/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CONTEXT_H
#define H2_CONTEXT_H 1

#include <h2_context_defs.h>

#define REGPAIR(ODD,EVEN) \
	union { \
		unsigned long long int r##ODD##EVEN; \
		struct { \
			unsigned int r##EVEN; \
			unsigned int r##ODD; \
		}; \
	};

#define CREGPAIR(ODD,EVEN) \
	union { \
		unsigned long long int ODD##EVEN; \
		struct { \
			unsigned int EVEN; \
			unsigned int ODD; \
		}; \
	};

typedef struct {
	/* OFFSET 0 */
	CREGPAIR(lc0,sa0);             // 0
	CREGPAIR(lc1,sa1);             // 8
	CREGPAIR(usr,p3_0);            // 16
	CREGPAIR(m1,m0);               // 24
	/* OFFSET 32 */
	CREGPAIR(gp,ugp);              // 32
	CREGPAIR(cs1,cs0);             // 40
	CREGPAIR(g1,g0);               // 48
	CREGPAIR(g3,g2);               // 56
	/* OFFSET 64 */
	CREGPAIR(framekey,framelimit); // 64
	REGPAIR(01,00);                // 72
	REGPAIR(03,02);                // 80
	REGPAIR(05,04);                // 88
	/* OFFSET 96 */
	REGPAIR(07,06);                // 96
	REGPAIR(09,08);                // 104
	REGPAIR(11,10);                // 112
	REGPAIR(13,12);                // 120
	/* OFFSET 128 */
	REGPAIR(15,14);                // 128
	REGPAIR(17,16);                // 136
	REGPAIR(19,18);                // 144
	REGPAIR(21,20);                // 152
	/* OFFSET 160 */
	REGPAIR(23,22);                // 160
	REGPAIR(25,24);                // 168
	REGPAIR(27,26);                // 176
	REGPAIR(29,28);                // 184
	/* OFFSET 192 */
	REGPAIR(31,30);                // 192
	/* SIZE: 200 */
} h2_context_t;

#endif
