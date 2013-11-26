/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <stddef.h>
#include <globals.h>
#include <stlb.h>
#include <vm.h>

#define PRINT_CONTEXT_OFFSET(x) fprintf(outfile, "#define CONTEXT_%s %d\n",#x,offsetof(H2K_thread_context,x))
#define PRINT_KG_OFFSET(x) fprintf(outfile, "#define KG_%s %d\n",#x,offsetof(H2K_kg_t,x))
#define PRINT_VMBLOCK_OFFSET(x) fprintf(outfile, "#define VMBLOCK_%s %d\n",#x,offsetof(H2K_vmblock_t,x))
#define PRINT_STLB_OFFSET(x) fprintf(outfile, "#define STLB_INFO_%s %d\n",#x,offsetof(H2K_mem_stlb_asid_info_t,x))

int main(int argc, char **argv)
{
	FILE *outfile = fopen(argv[1], "w");
	
	if (outfile == NULL) {
		perror(argv[1]);
		exit(1);
	}

	PRINT_CONTEXT_OFFSET(next);
	PRINT_CONTEXT_OFFSET(prev);
	PRINT_CONTEXT_OFFSET(hthread);
	PRINT_CONTEXT_OFFSET(status);
	PRINT_CONTEXT_OFFSET(vmstatus);
	PRINT_CONTEXT_OFFSET(pmu_on);
	PRINT_CONTEXT_OFFSET(tid);
	PRINT_CONTEXT_OFFSET(status_prio_hthread_tid);
	PRINT_CONTEXT_OFFSET(prio);
	PRINT_CONTEXT_OFFSET(base_prio);
	PRINT_CONTEXT_OFFSET(continuation);
	PRINT_CONTEXT_OFFSET(ccrssr);
	PRINT_CONTEXT_OFFSET(ssr);
	PRINT_CONTEXT_OFFSET(elr);
	PRINT_CONTEXT_OFFSET(r29);
	PRINT_CONTEXT_OFFSET(r00);
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
	fprintf(outfile, "#define CONTEXT_SIZE %d\n",sizeof(H2K_thread_context));
#ifdef DO_EXT_SWITCH
	fprintf(outfile, "#define EXT_CONTEXT_SIZE %d\n",sizeof(H2K_ext_context));
#endif
	fprintf(outfile, "#define FASTINT_CONTEXT_SIZE %d\n",sizeof(H2K_fastint_context));

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
	PRINT_KG_OFFSET(stacks_traptab);
	PRINT_KG_OFFSET(stacks_addr);
	PRINT_KG_OFFSET(traptab_addr);
	PRINT_KG_OFFSET(lowprio_masks);
	PRINT_KG_OFFSET(trace_info_entries_buf);
	PRINT_KG_OFFSET(trace_info_buf);
	PRINT_KG_OFFSET(trace_info_entries);
	PRINT_KG_OFFSET(trace_info_max_level_index);
	PRINT_KG_OFFSET(trace_info_index);
	PRINT_KG_OFFSET(trace_info_max_trace_level);
	PRINT_KG_OFFSET(trace_buf_default);
	PRINT_KG_OFFSET(stlbptr);
	PRINT_KG_OFFSET(vmblocks);
#ifdef H2K_L2_CONTROL
	PRINT_KG_OFFSET(l2_int_base);
	PRINT_KG_OFFSET(l2_ack_base);
	PRINT_KG_OFFSET(l2_intinfo);
#endif
	PRINT_VMBLOCK_OFFSET(waiting_cpus);
	PRINT_VMBLOCK_OFFSET(max_cpus);
	PRINT_VMBLOCK_OFFSET(contexts);
#ifdef DO_EXT_SWITCH
	PRINT_VMBLOCK_OFFSET(ext_contexts);
#endif
	PRINT_VMBLOCK_OFFSET(flags);

#ifdef COUNT_TLB_EVENTS
	PRINT_VMBLOCK_OFFSET(tlbmissx);
	PRINT_VMBLOCK_OFFSET(tlbmissrw);
#endif

	PRINT_STLB_OFFSET(valids);
	PRINT_STLB_OFFSET(pagesize);
	PRINT_STLB_OFFSET(waymask);
	PRINT_STLB_OFFSET(baseaddr);
	fprintf(outfile, "#define STLB_INFO_SIZE %d\n",sizeof(H2K_mem_stlb_asid_info_t));

	fprintf(outfile, "#define STATUS_DEAD       %d\n",H2K_STATUS_DEAD);
	fprintf(outfile, "#define STATUS_READY      %d\n",H2K_STATUS_READY);
	fprintf(outfile, "#define STATUS_RUNNING    %d\n",H2K_STATUS_RUNNING);
	fprintf(outfile, "#define STATUS_BLOCKED    %d\n",H2K_STATUS_BLOCKED);
	fprintf(outfile, "#define STATUS_INTBLOCKED %d\n",H2K_STATUS_INTBLOCKED);
	fprintf(outfile, "#define STATUS_VMWAIT     %d\n",H2K_STATUS_VMWAIT);

	return 0;
}

