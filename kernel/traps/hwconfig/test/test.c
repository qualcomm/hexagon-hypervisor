/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <hwconfig.h>
#include <fatal.h>
#include <globals.h>
#include <cache.h>
#include <hw.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t TH_saw_l2_cleaninv;

H2K_thread_context a;

void H2K_cache_l2_cleaninv()
{
	TH_saw_l2_cleaninv = 1;
}

#if ARCHV < 56
void TH_test_l2locka() {}
#else

int H2K_safemem_check_and_lock(void *addr, int type, pa_t *pa, H2K_thread_context *me)
{
	if (pa) *pa = (pa_t)addr;
	return 1;
}

char x[128] __attribute__((aligned(64)));

int TH_test_l2locka(int len) {
	u32_t ret;
	if ((ret = H2K_trap_hwconfig(HWCONFIG_L2LOCKA,&x,len,0,&a)) != 0) FAIL("returned fail");
	if (H2K_trap_hwconfig(HWCONFIG_L2UNLOCK,NULL,0,0,&a) != 0) FAIL("unlock returned fail");
	return 0;
}

#endif

#if ARCHV <= 3
void TH_test_pf()
{
	u32_t checkval = 0;
	a.ssr = 0;
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_I,1,&a);
	checkval |= 1<<22;
	if (a.ssr != checkval) FAIL("HF_I");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_D,1,&a);
	checkval |= 1<<23;
	if (a.ssr != checkval) FAIL("HF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_SF_D,1,&a);
	checkval |= 1<<24;
	if (a.ssr != checkval) FAIL("SF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_I,0,&a);
	checkval ^= 1<<22;
	if (a.ssr != checkval) FAIL("!HF_I");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_D,0,&a);
	checkval ^= 1<<23;
	if (a.ssr != checkval) FAIL("!HF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_SF_D,0,&a);
	checkval ^= 1<<24;
	if (a.ssr != checkval) FAIL("!SF_D");
}
#else
void TH_test_pf()
{
	u32_t checkval = 0;
	a.ccr = 0;
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_I,1,&a);
	checkval |= 1<<16;
	if (a.ccr != checkval) FAIL("HF_I");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_D,1,&a);
	checkval |= 1<<17;
	if (a.ccr != checkval) FAIL("HF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_SF_D,1,&a);
	checkval |= 1<<20;
	if (a.ccr != checkval) FAIL("SF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_I,0,&a);
	checkval ^= 1<<16;
	if (a.ccr != checkval) FAIL("!HF_I");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_HF_D,0,&a);
	checkval ^= 1<<17;
	if (a.ccr != checkval) FAIL("!HF_D");
	H2K_trap_hwconfig(2,NULL,HWCONFIG_PREFETCH_SF_D,0,&a);
	checkval ^= 1<<20;
	if (a.ccr != checkval) FAIL("!SF_D");
}
#endif

int main()
{
	u32_t syscfg,cur_size,cur_wb,i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	__asm__ __volatile(" imask = %0 " : : "r"(-1));
	H2K_set_syscfg(0x7f);

	/* Check base functionality for l2cache hwconfig */
	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,1,0,&a);
	// We always clean now
	// if (TH_saw_l2_cleaninv) FAIL("Clean cache shouldn't cause cleaninv 1");
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 1) FAIL("Invalid cache size");
	if (cur_wb != 0) FAIL("Invalid mode");

	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,1,1,&a);
	if (TH_saw_l2_cleaninv) FAIL("Clean cache shouldn't cause cleaninv 2");
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 1) FAIL("Invalid cache size");
	if (cur_wb != 1) FAIL("Invalid mode");

	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,1,0,&a);
#if ARCHV < 60
	// FIXME: Find a way to check the inline l2gcleaninv
	if (!TH_saw_l2_cleaninv) FAIL("Should have cleaned cache 1");
#endif
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 1) FAIL("Invalid cache size");
	if (cur_wb != 0) FAIL("Invalid mode");

	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,2,1,&a);
	// We always clean now
	//	if (TH_saw_l2_cleaninv) FAIL("Clean cache shouldn't cause cleaninv 3");
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 2) FAIL("Invalid cache size 1");
	if (cur_wb != 1) FAIL("Invalid mode");

	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,2,1,&a);
	if (TH_saw_l2_cleaninv) FAIL("no change");
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 2) FAIL("Invalid cache size 2");
	if (cur_wb != 1) FAIL("Invalid mode");

	TH_saw_l2_cleaninv = 0;
	H2K_trap_hwconfig(0,NULL,3,1,&a);
#if ARCHV < 60
	// FIXME: Find a way to check the inline l2gcleaninv
	if (!TH_saw_l2_cleaninv) FAIL("Should have cleaned cache 2");
#endif
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	if (cur_size != 3) FAIL("Invalid cache size 3");
	if (cur_wb != 1) FAIL("Invalid mode");

	/* Check for partitioning */
	for (i = 0; i < 4; i++) {
		H2K_trap_hwconfig(1,NULL,HWCONFIG_PARTITION_I,i,&a);
		syscfg = H2K_get_syscfg();
		if (((syscfg >> 27) & 3) != i) FAIL("Bad partition config (I)");
	}
	for (i = 0; i < 4; i++) {
		H2K_trap_hwconfig(1,NULL,HWCONFIG_PARTITION_D,i,&a);
		syscfg = H2K_get_syscfg();
		if (((syscfg >> 25) & 3) != i) FAIL("Bad partition config (D)");
	}
#if ARCHV >= 4
	for (i = 0; i < 4; i++) {
		H2K_trap_hwconfig(1,NULL,HWCONFIG_PARTITION_L2,i,&a);
		syscfg = H2K_get_syscfg();
		if (((syscfg >> 29) & 3) != i) FAIL("Bad partition config (L2)");
	}
#endif

	/* Check for PF bits */
	TH_test_pf();

	TH_test_l2locka(64);
	TH_test_l2locka(128);

	puts("TEST PASSED\n");
	return 0;
}

