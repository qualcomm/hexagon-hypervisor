/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <stlb.h>
#include <tlbfmt.h>
#include <asid.h>
#include <alloc.h>

/*  this test can be reseeded  */
#ifndef TEST_SEED
#define TEST_SEED 2
#endif

H2K_thread_context a;

H2K_mem_stlb_asid_info_t TH_mem_stlb_asid_infos[MAX_ASIDS];   // 128*(256B+4B+4B+4B) = ~33kB
H2K_mem_tlbfmt_t TH_mem_stlb[STLB_MAX_SETS*2][STLB_MAX_WAYS]; // 2048*2*4*8B         = 128kB
H2K_mem_alloc_tag_t Heap[H2K_ALLOC_HEAP_SIZE] __attribute__((aligned(ALLOC_UNIT))) = {{{.size = 0, .free = 0}}}; //65536*4B=256kB

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

s32_t TH_mem_stlb_alloc()
{
	return H2K_mem_stlb_alloc();
}

void TH_mem_stlb_init()
{
	int i,j;
	for(i=0; i<MAX_ASIDS; i++) {
		TH_mem_stlb_asid_infos[i].baseaddr = &TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][0];
		TH_mem_stlb_asid_infos[i].waymask = rand() % (1ULL<<32);
		TH_mem_stlb_asid_infos[i].pagesize = rand() % (1ULL<<32);
		for(j=0; j<STLB_MAX_SETS/64; j++) {
			TH_mem_stlb_asid_infos[i].valids[j] = rand() % 0xffffffffffffffffULL;
		}
 		for(j=0; j<STLB_MAX_WAYS; j++) {
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].asid = rand() % MAX_ASIDS+1;
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].vpn = rand() % (1U<<20);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].global = rand() % (1U<<1);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].valid = rand() % (1U<<1);
#if ARCHV <= 3
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].ppn = rand() % (1U<<20);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].size = rand() % (1U<<4);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].part = rand() % (1U<<2);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].ccc = rand() % (1U<<3);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].xwr = rand() % (1U<<3);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].guestonly = rand() % (1U<<1);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused = rand() % (1U<<2);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused2 = rand() % (1U<<2);
#else
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].ppd = rand() % (1U<<24);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].cccc = rand() % (1U<<4);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].xwru = rand() % (1U<<4);
#if ARCHV < 73
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].abits = rand() % (1U<<2);
#else
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].pa3637 = rand() % (1U<<2);
#endif
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].pa35 = rand() % (1U<<1);
#endif
		}
	}
	H2K_kg.stlbptr = TH_mem_stlb_asid_infos;
}

void TH_compare_tlbfmt(H2K_mem_tlbfmt_t original, H2K_mem_tlbfmt_t test)
{
	if (original.raw != test.raw) {
		printf("original.raw vs test.raw:\n0x%llx vs 0x%llx\n",original.raw, test.raw);
		FAIL("mismatched tlbfmt found\n"); 
	}
}

void TH_tlbfmt_iszero(H2K_mem_tlbfmt_t test)
{
	if (test.raw !=0) {
		FAIL("Entry found when no entry should exist.\n"); 
	}
}

int main()
{
	int i,j;
	/* Debug stuff */
	/* int count,total; */
	H2K_mem_tlbfmt_t entry, test;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	srand(TEST_SEED);
#if ARCHV <= 3
	entry.ppn = 0;
	entry.ccc = 0;
	entry.xwr = 0x7;
	entry.size = 0;
	entry.valid = 1;
	entry.asid = 0;
	entry.vpn = 0;
#else
	entry.ppd = 1;
	entry.cccc = 0;
	entry.xwru = 0xf;
	entry.valid = 1;
	entry.asid = 0;
	entry.vpn = 0;
#endif

        /* No storage */
	H2K_asid_table_init();
	H2K_mem_stlb_invalidate_va(0,1,0,&a);
	H2K_mem_stlb_invalidate_asid(0);
	H2K_mem_stlb_add(0,0,entry,&a);
	if (H2K_mem_stlb_lookup(0,0,&a).raw != 0) {
		FAIL("found entry with no storage");
	}

	/* Use Test Harness storage */
	TH_mem_stlb_init();

	/* Invalidate already invalidated asid */
	H2K_mem_stlb_invalidate_asid(0);
	H2K_mem_stlb_invalidate_asid(0);
	test = H2K_mem_stlb_lookup(0,0,&a);
	TH_tlbfmt_iszero(test);

	/* Invalidate already invalidated va */
	H2K_mem_stlb_invalidate_va(0,1,0,&a);
	test = H2K_mem_stlb_lookup(0,0,&a);
	TH_tlbfmt_iszero(test);

	for(i=0; i<MAX_ASIDS; i++) {
		//printf("Testing asid=%x\n",i);
		entry.asid=i;
		for(j=0; j<(1ULL<<22); j+=0x1000) {
			//printf("addr=%08x\n",j);
			entry.vpn=j>>PAGE_BITS;
			H2K_mem_stlb_add(j,i,entry,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			//printf("add/lookup\n");
			TH_compare_tlbfmt(entry, test);
			test = H2K_mem_stlb_lookup(j+(rand() & 0xfff),i,&a);
			TH_compare_tlbfmt(entry, test);
			//printf("add/lookup other in page\n");
			//Invalidate entry.
			H2K_mem_stlb_invalidate_va(j,1,i,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			//printf("lookup after invalidate\n");
			TH_tlbfmt_iszero(test);
		}

		if (0 == i % 2) {
			H2K_mem_stlb_add(0,i,entry,&a);
			H2K_mem_stlb_invalidate_asid(i);
			test = H2K_mem_stlb_lookup(0,i,&a);
			TH_tlbfmt_iszero(test);
		}

		for(j=0xfff; j<(1ULL<<20); j+=0x10000) {
			entry.vpn=j>>PAGE_BITS;
			H2K_mem_stlb_add(j,i,entry,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			TH_compare_tlbfmt(entry, test);
			test = H2K_mem_stlb_lookup(j-(rand() & 0xfff),i,&a);
			TH_compare_tlbfmt(entry, test);
			H2K_mem_stlb_invalidate_va(j,1,i,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			TH_tlbfmt_iszero(test);
		}

	}

	/* Use Test Harness alloc - Note: Overwrites H2K stlbptr in its updated init of STLB */
	H2K_mem_alloc_init(Heap, (MAX_ASIDS)*sizeof(H2K_mem_stlb_asid_info_t));
	if (TH_mem_stlb_alloc() > 0) FAIL("allocation of stlb with small heap");
	H2K_mem_alloc_free(Heap);
	H2K_mem_alloc_init(Heap, H2K_ALLOC_HEAP_SIZE);
	if (TH_mem_stlb_alloc() <= 0) FAIL("non-allocation of stlb with large heap");

	/* Reinitialize */
	TH_mem_stlb_init();
	H2K_mem_stlb_invalidate_va(0,1,0,&a);
	H2K_mem_stlb_invalidate_va(0x1000,1,0,&a);
	entry.raw = 0;
	H2K_mem_stlb_add(0,0,entry,&a);
	test = H2K_mem_stlb_lookup(0,0,&a);
	TH_tlbfmt_iszero(test);

	/* DEBUG STUFF */

	/* //Printouts for the TH stlb table */
	/* count = 0; */
	/* total = 0; */
	/* for(i=0; i<2*STLB_MAX_SETS; i++) { */
	/* 	for(j=0; j<STLB_MAX_WAYS; j++) { */
	/* 		//Show the raw data */
	/* 		if (TH_mem_stlb[i][j].raw != 0) {  */
	/* 			printf("%lld\n", TH_mem_stlb[i][j].raw); */
	/* 			count++; */
	/* 		} */
	/* 	} */
	/* 	if (count != 0) { */
	/* 		//Show anything that has something in it */
        /*                 printf("\n set %d: %d\n",i ,count); */
	/* 	} */
	/* 	total += count; */
	/* 	count = 0; */
	/* } */
	/* //Show how many actual entries have something. */
	/* printf("\nTotal of %d entries\n", total); */

	/* //Prints valid arrays for each ASID */
	/* for(i=0; i<MAX_ASIDS; i++) { */
	/* 	if(H2K_mem_stlb_asid_infos[i].valids[0] || */
	/* 	   H2K_mem_stlb_asid_infos[i].valids[1] || */
	/* 	   H2K_mem_stlb_asid_infos[i].valids[2] || */
	/* 	   H2K_mem_stlb_asid_infos[i].valids[3] ) { */
	/* 		printf("ASID %d: valids %llx %llx %llx %llx\n",i, */
	/* 		       H2K_mem_stlb_asid_infos[i].valids[0], */
	/* 		       H2K_mem_stlb_asid_infos[i].valids[1], */
	/* 		       H2K_mem_stlb_asid_infos[i].valids[2], */
	/* 		       H2K_mem_stlb_asid_infos[i].valids[3]); */
	/* 	} */
	/* } */

	puts("TEST PASSED");
	return 0;
}

