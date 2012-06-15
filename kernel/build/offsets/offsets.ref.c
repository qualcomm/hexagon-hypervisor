/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <context.h>
#include <stddef.h>
#include <globals.h>

#define PRINT_CONTEXT_OFFSET(x) printf("#define CONTEXT_%s %d\n",#x,offsetof(H2K_thread_context,x))
#define PRINT_KG_OFFSET(x) printf("#define KG_%s %d\n",#x,offsetof(H2K_kg_t,x))

int main(int argc, char **argv)
{
PRINT_CONTEXT_OFFSET(next);
PRINT_CONTEXT_OFFSET(hthread);
PRINT_CONTEXT_OFFSET(status);
PRINT_CONTEXT_OFFSET(vmstatus);
PRINT_CONTEXT_OFFSET(pmu_on);
PRINT_CONTEXT_OFFSET(tid);
PRINT_CONTEXT_OFFSET(prio);
PRINT_CONTEXT_OFFSET(base_prio);
PRINT_CONTEXT_OFFSET(continuation);
PRINT_CONTEXT_OFFSET(ssrelr);
PRINT_CONTEXT_OFFSET(ssr);
PRINT_CONTEXT_OFFSET(elr);
PRINT_CONTEXT_OFFSET(r29);
PRINT_CONTEXT_OFFSET(gpugp);
PRINT_CONTEXT_OFFSET(r3130);
PRINT_CONTEXT_OFFSET(r2928);
PRINT_CONTEXT_OFFSET(r2726);
PRINT_CONTEXT_OFFSET(r2524);
PRINT_CONTEXT_OFFSET(r2322);
PRINT_CONTEXT_OFFSET(r2120);
PRINT_CONTEXT_OFFSET(r1918);
PRINT_CONTEXT_OFFSET(r1716);
PRINT_CONTEXT_OFFSET(r1514);
PRINT_CONTEXT_OFFSET(r1312);
PRINT_CONTEXT_OFFSET(r1110);
PRINT_CONTEXT_OFFSET(r0908);
PRINT_CONTEXT_OFFSET(r0706);
PRINT_CONTEXT_OFFSET(r0504);
PRINT_CONTEXT_OFFSET(r0302);
PRINT_CONTEXT_OFFSET(r0100);
PRINT_CONTEXT_OFFSET(lc0sa0);
PRINT_CONTEXT_OFFSET(lc1sa1);
PRINT_CONTEXT_OFFSET(m1m0);
PRINT_CONTEXT_OFFSET(sr_preds);
PRINT_CONTEXT_OFFSET(totalcycles);
PRINT_CONTEXT_OFFSET(id);
PRINT_CONTEXT_OFFSET(vmblock);
PRINT_CONTEXT_OFFSET(futex_ptr);
PRINT_CONTEXT_OFFSET(trapmask);
PRINT_CONTEXT_OFFSET(gevb);
PRINT_CONTEXT_OFFSET(ccr);
PRINT_CONTEXT_OFFSET(gssr_gelr);
PRINT_CONTEXT_OFFSET(gbadva_gosp);
PRINT_CONTEXT_OFFSET(cs1cs0);
PRINT_CONTEXT_OFFSET(atomic_status_word);
	printf("#define CONTEXT_SIZE %d\n",sizeof(H2K_thread_context));
	printf("#define FASTINT_CONTEXT_SIZE %d\n",sizeof(H2K_fastint_context));

PRINT_KG_OFFSET(ready_valids);
PRINT_KG_OFFSET(priomask);
PRINT_KG_OFFSET(wait_mask);
PRINT_KG_OFFSET(oncpu_start);
PRINT_KG_OFFSET(oncpu_wait);
PRINT_KG_OFFSET(waitcycles);
PRINT_KG_OFFSET(fastint_gp);
PRINT_KG_OFFSET(runlist);
PRINT_KG_OFFSET(runlist_prios);
PRINT_KG_OFFSET(ready);
PRINT_KG_OFFSET(futexhash);
PRINT_KG_OFFSET(inthandlers);
PRINT_KG_OFFSET(trace_info);
PRINT_KG_OFFSET(stacks_traptab);
PRINT_KG_OFFSET(stacks_addr);
PRINT_KG_OFFSET(traptab_addr);
PRINT_KG_OFFSET(lowprio_masks);
#ifdef H2K_L2_CONTROL
PRINT_KG_OFFSET(l2_int_base);
PRINT_KG_OFFSET(l2_ack_base);
PRINT_KG_OFFSET(l2_intinfo);
#endif

	printf("#define STATUS_DEAD       %d\n",H2K_STATUS_DEAD);
	printf("#define STATUS_READY      %d\n",H2K_STATUS_READY);
	printf("#define STATUS_RUNNING    %d\n",H2K_STATUS_RUNNING);
	printf("#define STATUS_BLOCKED    %d\n",H2K_STATUS_BLOCKED);

	return 0;
}

