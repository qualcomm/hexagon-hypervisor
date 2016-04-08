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
	CREGPAIR(lc0,sa0);
	CREGPAIR(lc1,sa1);
	CREGPAIR(usr,p3_0);
	CREGPAIR(m1,m0);
	/* OFFSET 32 */
	CREGPAIR(gp,ugp);
	CREGPAIR(cs1,cs0);
	CREGPAIR(g1,g0);
	CREGPAIR(g3,g2);
	/* OFFSET 64 */
	REGPAIR(01,00);
	REGPAIR(03,02);
	REGPAIR(05,04);
	REGPAIR(07,06);
	/* OFFSET 96 */
	REGPAIR(09,08);
	REGPAIR(11,10);
	REGPAIR(13,12);
	REGPAIR(15,14);
	/* OFFSET 128 */
	REGPAIR(17,16);
	REGPAIR(19,18);
	REGPAIR(21,20);
	REGPAIR(23,22);
	/* OFFSET 160 */
	REGPAIR(25,24);
	REGPAIR(27,26);
	REGPAIR(29,28);
	REGPAIR(31,30);
	/* SIZE: 192 */
} h2_context_t;

#endif
