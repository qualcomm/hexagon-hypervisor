/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <context.h>
#include <stddef.h>

#define PRINT_OFFSET(x) printf("#define CONTEXT_%s %d\n",#x,offsetof(H2K_thread_context,x))

int main(int argc, char **argv)
{
PRINT_OFFSET(hthread);
PRINT_OFFSET(status);
PRINT_OFFSET(vmstatus);
PRINT_OFFSET(tid);
PRINT_OFFSET(prio);
PRINT_OFFSET(baseprio);
PRINT_OFFSET(schedprio);
PRINT_OFFSET(continuation);
PRINT_OFFSET(ssrelr);
PRINT_OFFSET(ugpgp);
PRINT_OFFSET(r3130);
PRINT_OFFSET(r2928);
PRINT_OFFSET(r2726);
PRINT_OFFSET(r2524);
PRINT_OFFSET(r2322);
PRINT_OFFSET(r2120);
PRINT_OFFSET(r1918);
PRINT_OFFSET(r1716);
PRINT_OFFSET(r1514);
PRINT_OFFSET(r1312);
PRINT_OFFSET(r1110);
PRINT_OFFSET(r0908);
PRINT_OFFSET(r0706);
PRINT_OFFSET(r0504);
PRINT_OFFSET(r0302);
PRINT_OFFSET(r0100);
PRINT_OFFSET(lc0sa0);
PRINT_OFFSET(lc1sa1);
PRINT_OFFSET(m1m0);
PRINT_OFFSET(sr_preds);
PRINT_OFFSET(totalcycles);
PRINT_OFFSET(oncpu_start);
PRINT_OFFSET(futex_ptr);
PRINT_OFFSET(trapmask);
PRINT_OFFSET(gevb);
PRINT_OFFSET(gelr_gbadva);
PRINT_OFFSET(gssr_gosp);
	printf("#define CONTEXT_SIZE %d\n",sizeof(H2K_thread_context));
	printf("#define FASTINT_CONTEXT_SIZE %d\n",sizeof(H2K_fastint_context));
	return 0;
}

