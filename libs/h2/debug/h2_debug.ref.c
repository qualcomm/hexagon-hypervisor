/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>
	
#define PRINT_RPAIR(NAME0,NAME1) \
	printf("r"#NAME1 ":\t0x%08x\tr" #NAME0 ":\t0x%08x\n", context->r##NAME1, context->r##NAME0);

#define PRINT_CPAIR(NAME0,NAME1) \
	printf(#NAME1 ":\t0x%08x\t" #NAME0 ":\t0x%08x\n", context->NAME1, context->NAME0);

void h2_debug_context_dump(h2_context_t *context)
{
	printf("DEBUG DUMP threadid=%x\n",h2_thread_myid());
	PRINT_RPAIR(31,30);
	PRINT_RPAIR(29,28);
	PRINT_RPAIR(27,26);
	PRINT_RPAIR(25,24);
	PRINT_RPAIR(23,22);
	PRINT_RPAIR(21,20);
	PRINT_RPAIR(19,18);
	PRINT_RPAIR(17,16);
	PRINT_RPAIR(15,14);
	PRINT_RPAIR(13,12);
	PRINT_RPAIR(11,10);
	PRINT_RPAIR(09,08);
	PRINT_RPAIR(07,06);
	PRINT_RPAIR(05,04);
	PRINT_RPAIR(03,02);
	PRINT_RPAIR(01,00);
	PRINT_CPAIR(g3,g2);
	PRINT_CPAIR(g1,g0);
	PRINT_CPAIR(cs1,cs0);
	PRINT_CPAIR(gp,ugp);
	PRINT_CPAIR(m1,m0);
	PRINT_CPAIR(usr,p3_0);
	PRINT_CPAIR(lc1,sa1);
	PRINT_CPAIR(lc0,sa0);
}

