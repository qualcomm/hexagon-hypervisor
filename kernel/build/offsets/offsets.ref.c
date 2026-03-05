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
#define PRINT_KG_SUBSTRUCT_OFFSET(x,y) fprintf(outfile, "#define KG_%s_%s %d\n",#x,#y,offsetof(H2K_kg_t,x.y))
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
	PRINT_CONTEXT_OFFSET(usrp30);
	PRINT_CONTEXT_OFFSET(totalcycles);
	PRINT_CONTEXT_OFFSET(id);
	PRINT_CONTEXT_OFFSET(vmblock);
	PRINT_CONTEXT_OFFSET(futex_ptr_64);
	PRINT_CONTEXT_OFFSET(futex_ptr_lo);
	PRINT_CONTEXT_OFFSET(futex_ptr_hi);
	PRINT_CONTEXT_OFFSET(pktcount);
	PRINT_CONTEXT_OFFSET(framekey_framelimit);
	PRINT_CONTEXT_OFFSET(trapmask);
	PRINT_CONTEXT_OFFSET(gevb);
	PRINT_CONTEXT_OFFSET(ccr);
	PRINT_CONTEXT_OFFSET(gssr_gelr);
	PRINT_CONTEXT_OFFSET(gbadva_gosp);
	PRINT_CONTEXT_OFFSET(cs1cs0);
	PRINT_CONTEXT_OFFSET(atomic_status_word);
	PRINT_CONTEXT_OFFSET(cpuint_enabled_pending);
	PRINT_CONTEXT_OFFSET(cpuint_enabled);
	PRINT_CONTEXT_OFFSET(cpuint_pending);
	PRINT_CONTEXT_OFFSET(tree);
	PRINT_CONTEXT_OFFSET(rightleft);
	PRINT_CONTEXT_OFFSET(timeout);
#if ARCHV >= 68
	PRINT_CONTEXT_OFFSET(dm0);
#endif
#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	PRINT_CONTEXT_OFFSET(vwctrl);
#endif
	fprintf(outfile, "#define CONTEXT_PSEUDO_SGP0 0xFFFFFFFF\n");
	fprintf(outfile, "#define CONTEXT_PSEUDO_IMASK 0xFFFFFFFE\n");
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
	PRINT_KG_OFFSET(fastint_gp_ssr);
	PRINT_KG_OFFSET(runlist);
	PRINT_KG_OFFSET(runlist_prios);
	PRINT_KG_OFFSET(ready);
	PRINT_KG_OFFSET(futexhash);
	PRINT_KG_OFFSET(inthandlers);
	PRINT_KG_OFFSET(lowprio_masks);
	PRINT_KG_OFFSET(stlbptr);
	PRINT_KG_OFFSET(vmblocks);
	PRINT_KG_OFFSET(phys_offset);
	PRINT_KG_OFFSET(core_rev);
	PRINT_KG_OFFSET(timer_intnum);
	PRINT_KG_OFFSET(hthreads_mask);
	PRINT_KG_OFFSET(hthreads);

#ifdef CLUSTER_SCHED
	PRINT_KG_OFFSET(cluster_clusters);
	PRINT_KG_OFFSET(cluster_hthreads);
	PRINT_KG_OFFSET(cluster_mask);
	PRINT_KG_OFFSET(cluster_sched);
	PRINT_KG_OFFSET(coproc_max);
	PRINT_KG_OFFSET(coproc_count);
#endif

#ifdef CRASH_DEBUG
	PRINT_KG_OFFSET(crash_contexts);
	PRINT_KG_OFFSET(crash_tlb);
#endif

	PRINT_KG_OFFSET(syscfg_val);
	PRINT_KG_OFFSET(fatal_hook_and_arg);
	PRINT_KG_OFFSET(fatal_hook_gp_ssr);
	PRINT_KG_OFFSET(is_nmi_soft);
#ifdef H2K_L2_CONTROL
	PRINT_KG_OFFSET(l2_int_base);
	PRINT_KG_OFFSET(l2_ack_base);
	PRINT_KG_OFFSET(l2_intinfo);
#endif
	PRINT_KG_OFFSET(angel_lock);
	PRINT_KG_OFFSET(info_boot_flags);
	PRINT_KG_OFFSET(dma_version);
	PRINT_KG_OFFSET(core_id);
	PRINT_KG_OFFSET(core_count);

	PRINT_KG_SUBSTRUCT_OFFSET(time,next_ticks);
	PRINT_KG_SUBSTRUCT_OFFSET(time,last_ticks);
	PRINT_KG_SUBSTRUCT_OFFSET(time,last_pcycles);
	PRINT_KG_SUBSTRUCT_OFFSET(time,timeouts);
	PRINT_KG_SUBSTRUCT_OFFSET(time,devptr);
	PRINT_VMBLOCK_OFFSET(waiting_cpus);
	PRINT_VMBLOCK_OFFSET(max_cpus);
	PRINT_VMBLOCK_OFFSET(contexts);
	PRINT_VMBLOCK_OFFSET(intinfo);
	PRINT_VMBLOCK_OFFSET(parent);
	PRINT_VMBLOCK_OFFSET(waiting_cpus);
	PRINT_VMBLOCK_OFFSET(pending);
	PRINT_VMBLOCK_OFFSET(enable);
	PRINT_VMBLOCK_OFFSET(percpu_mask);
	PRINT_VMBLOCK_OFFSET(int_v2p);
	PRINT_VMBLOCK_OFFSET(max_cpus);
	PRINT_VMBLOCK_OFFSET(num_cpus);
	PRINT_VMBLOCK_OFFSET(contexts);
#ifdef DO_EXT_SWITCH
	PRINT_VMBLOCK_OFFSET(ext_contexts);
#endif
	PRINT_VMBLOCK_OFFSET(flags);

#ifdef COUNT_TLB_EVENTS
	PRINT_VMBLOCK_OFFSET(tlbmissx);
	PRINT_VMBLOCK_OFFSET(tlbmissrw);
#endif
	fprintf(outfile, "#define VMBLOCK_TOTALSIZE %d\n",sizeof(H2K_vmblock_t));

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

