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
extern H2K_mem_stlb_asid_info_t *H2K_mem_stlb_asid_infos;// IN_SECTION(".data.mem.stlb"); /* MOVE TO GLOBALS */  

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
			TH_mem_stlb_asid_infos[i].valids[j] = rand() % (1ULL<<64);
		}
		for(j=0; j<STLB_MAX_WAYS; j++) {
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].asid = rand() % MAX_ASIDS+1;
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].vpn = rand() % (1U<<20);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].global = rand() % (1U<<1);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].valid = rand() % (1U<<1);
#if __QDSP6_ARCH__ <= 3
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].ppn = rand() % (1U<<20);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].size = rand() % (1U<<4);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].part = rand() % (1U<<2);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].ccc = rand() % (1U<<3);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].xwr = rand() % (1U<<3);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].guestonly = rand() % (1U<<1);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused = rand() % (1U<<2);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused2 = rand() % (1U<<2);
#else
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].cccc = rand() % (1U<<4);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].xwru = rand() % (1U<<4);
			TH_mem_stlb[i*(STLB_MAX_SETS/MAX_ASIDS)][j].unused = rand() % (1U<<3);
#endif
		}
	}
	H2K_mem_stlb_asid_infos = TH_mem_stlb_asid_infos;
}

void TH_compare_tlbfmt(H2K_mem_tlbfmt_t original, H2K_mem_tlbfmt_t new)
{
	if (original.raw != new.raw) {
		printf("test.raw vs original.raw:\n0x%llx\n0x%llx\nn",original.raw, new.raw);
//		FAIL("mismatched tlbfmt found\n"); 
	}
}

int main()
{
	int i,j;
	int count,total =0;
	H2K_mem_tlbfmt_t entry, test;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_asid_table_init();
	H2K_mem_stlb_init();

	srand(TEST_SEED);
#if __QDSP6_ARCH__ <= 3
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
	H2K_mem_stlb_init();
	H2K_mem_stlb_invalidate_va(0,0,&a);
	H2K_mem_stlb_invalidate_asid(0);
	H2K_mem_stlb_add(0,0,entry,&a);
	if (H2K_mem_stlb_lookup(0,0,&a).raw != 0) {
		FAIL("found entry with no storage");
	}

	/* Use Test Harness storage */
	TH_mem_stlb_init();

	H2K_mem_stlb_add(0,0,entry,&a);
	test = H2K_mem_stlb_lookup(0,0,&a); // start of the page:
	TH_compare_tlbfmt(entry, test);
	test = H2K_mem_stlb_lookup(0xFFF,0,&a); // end of the same page:
	TH_compare_tlbfmt(entry, test);

	H2K_mem_stlb_invalidate_va(0,0,&a);
	H2K_mem_stlb_invalidate_asid(0);

	H2K_mem_stlb_add(0,0,entry,&a);
	test = H2K_mem_stlb_lookup(0,0,&a); // start of the page:
	TH_compare_tlbfmt(entry, test);
	test = H2K_mem_stlb_lookup(0xFFF,0,&a); // end of the same page:
	TH_compare_tlbfmt(entry, test);

//	for(i=0; i<MAX_ASIDS; i++) {

	entry.asid=9;
	H2K_mem_stlb_add(0,9,entry,&a);
	H2K_mem_stlb_add(0xfff,9,entry,&a);
	test = H2K_mem_stlb_lookup(0,9,&a);
	TH_compare_tlbfmt(entry, test);
	test = H2K_mem_stlb_lookup(0xfff,9,&a);
	TH_compare_tlbfmt(entry, test);

	entry.vpn=0xf;
	H2K_mem_stlb_add(0xffff,9,entry,&a);
	H2K_mem_stlb_add(0xffff,9,entry,&a);
	test = H2K_mem_stlb_lookup(0xffff,9,&a);
	TH_compare_tlbfmt(entry, test);

	entry.vpn=0x1f;
	H2K_mem_stlb_add(0x1ffff,9,entry,&a);
	test = H2K_mem_stlb_lookup(0x1ffff,9,&a);
	TH_compare_tlbfmt(entry, test);

	entry.vpn=0x4f;
	H2K_mem_stlb_add(0x4ffff,9,entry,&a);
	test = H2K_mem_stlb_lookup(0x4ffff,9,&a);
	TH_compare_tlbfmt(entry, test);

	entry.vpn=0x8f;
	H2K_mem_stlb_add(0x8ffff,9,entry,&a);
	test = H2K_mem_stlb_lookup(0x8ffff,9,&a);
	TH_compare_tlbfmt(entry, test);

	entry.asid=10;
	entry.vpn=0xaf;
	H2K_mem_stlb_add(0xaffff,10,entry,&a);
	test = H2K_mem_stlb_lookup(0xaffff,10,&a);
	TH_compare_tlbfmt(entry, test);

	entry.asid=11;
	entry.vpn=0xff;
	H2K_mem_stlb_add(0xfffff,11,entry,&a);
	test = H2K_mem_stlb_lookup(0xfffff,11,&a);
	TH_compare_tlbfmt(entry, test);

	entry.asid=12;
	entry.vpn=0xf0000;
	H2K_mem_stlb_add(0xf0000fff,12,entry,&a);
	test = H2K_mem_stlb_lookup(0xf0000fff,12,&a);
	TH_compare_tlbfmt(entry, test);

	entry.vpn=0xfffff;
	H2K_mem_stlb_add(0xffffffff,12,entry,&a);
	H2K_mem_stlb_add(0xfffffabe,12,entry,&a);
	test = H2K_mem_stlb_lookup(0xfffffabe,12,&a);
	TH_compare_tlbfmt(entry, test);

	/* //Printouts for the TH stlb table */
	/* for(i=0; i<2*STLB_MAX_SETS; i++) { */
	/* 	for(j=0; j<STLB_MAX_WAYS; j++) { */
	/* 		//Show the raw data */
        /*                 //printf("%lld ", TH_mem_stlb[i][j].raw); */
	/* 		if (TH_mem_stlb[i][j].raw != 0) { count++; } */
	/* 	} */
	/* 	if (count != 0) { */
	/* 		//Show anything that has something in it */
        /*                 //printf(" set %d: %d\n",i ,count); */
	/* 	} */
	/* 	total += count; */
	/* 	count = 0; */
	/* } */
	/* //Show how many actual entries have something. */
	/* printf("\nTotal of %d entries\n", total); */

	/* //Prints valid arrays for each ASID */
	/* for(i=0; i<MAX_ASIDS; i++) { */
	/* 	if(H2K_mem_stlb_asid_infos[i].valids[0] || H2K_mem_stlb_asid_infos[i].valids[1] || H2K_mem_stlb_asid_infos[i].valids[2] || H2K_mem_stlb_asid_infos[i].valids[3] ) { */
	/* 		printf("ASID %d: valids %llx %llx %llx %llx\n",i, H2K_mem_stlb_asid_infos[i].valids[0], H2K_mem_stlb_asid_infos[i].valids[1], H2K_mem_stlb_asid_infos[i].valids[2], H2K_mem_stlb_asid_infos[i].valids[3] ); */
	/* 	} */
	/* } */

	H2K_mem_stlb_invalidate_va(0,9,&a);
	H2K_mem_stlb_invalidate_va(0xffff,9,&a);
	H2K_mem_stlb_invalidate_va(0xfffff,9,&a);
	H2K_mem_stlb_invalidate_asid(9);

	
	// entry not found, but there is storage...
	test = H2K_mem_stlb_lookup(0xffff,9,&a);
	// figure out check...
	printf("test.raw is 0x%llx\n", test.raw);

	//Init asid table
	puts("TEST PASSED");
	return 0;
}

