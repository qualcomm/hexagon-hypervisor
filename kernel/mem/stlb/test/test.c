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

/*  this test can be reseeded  */

#ifndef TEST_SEED
#define TEST_SEED 2
#endif

H2K_thread_context a;

H2K_mem_stlb_asid_info_t TH_mem_stlb_asid_infos[MAX_ASIDS];
H2K_mem_tlbfmt_t TH_mem_stlb[STLB_MAX_SETS*2][STLB_MAX_WAYS];

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
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
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused = rand() % (1U<<3);
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
	H2K_asid_table_init();

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
		entry.asid=i;
		for(j=0; j<(1ULL<<20); j+=0x10000) {
			entry.vpn=j>>PAGE_BITS;
			H2K_mem_stlb_add(j,i,entry,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			TH_compare_tlbfmt(entry, test);
			test = H2K_mem_stlb_lookup(j+rand() % 0xfff,i,&a);
			TH_compare_tlbfmt(entry, test);
			//Invalidate entry.
			H2K_mem_stlb_invalidate_va(j,1,i,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
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
			test = H2K_mem_stlb_lookup(j-rand() % 0xfff,i,&a);
			TH_compare_tlbfmt(entry, test);
			H2K_mem_stlb_invalidate_va(j,1,i,&a);
			test = H2K_mem_stlb_lookup(j,i,&a);
			TH_tlbfmt_iszero(test);
		}

	}

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

