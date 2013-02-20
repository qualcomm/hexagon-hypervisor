/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <hwconfig.h>
#include <asm_offsets.h>
#include <thread.h>
#include <fatal.h>
#include <globals.h>
#include <stmode.h>
#include <hw.h>
#include <cache.h>
#include <hexagon_protos.h>

typedef u32_t (*configptr_t)(u32_t, void *, u32_t, u32_t, H2K_thread_context *);

#define MAX_CONFIGS 3

static const configptr_t H2K_hwconfigtab[MAX_CONFIGS] IN_SECTION(".data.config.hwconfig") = {
	H2K_trap_hwconfig_l2cache,
	H2K_trap_hwconfig_partitions,
	H2K_trap_hwconfig_prefetch,
};

u32_t H2K_trap_hwconfig(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if (configtype >= MAX_CONFIGS) return 0;
	return H2K_hwconfigtab[configtype](0,ptr,val2,val3,me);
}

u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me)
{
	u32_t cur_size;
	u32_t cur_wb;
	u32_t syscfg;
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg >> 16) & 0x7;
	cur_wb = (syscfg >> 23) & 1;
	size &= 0x7;
	use_wb &= 1;

	/* ST Mode */
	if (H2K_stmode_begin() != 0) return -1;

	if (size != cur_size) {
		if (cur_wb) {
			/* Clean entire cache */
			H2K_cache_l2_cleaninv();
		}
		/* Set to 0 size */
		syscfg &= 0xfff8ffff;
		H2K_set_syscfg(syscfg);
		/* L2kill */
		asm volatile (" l2kill ; isync ");
		/* Update size, mode */
		syscfg |= ((size << 16) | (use_wb << 23));
	} else if (use_wb && !cur_wb) {
		syscfg |= (use_wb<<23);
	} else if (!use_wb && cur_wb) {
		/* Just leave WB mode */
		/* Clean entire cache */
		H2K_cache_l2_cleaninv();
		/* Disable WB mode on L2$ */
		syscfg = Q6_R_clrbit_RR(syscfg,23);
	}
	H2K_set_syscfg(syscfg);
	H2K_stmode_end();
	return 0;
}

u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)
{
	u32_t syscfg;
	u64_t insertval;
	insertval = (2ULL << 32) | (25+(2*whatcache));
	syscfg = H2K_get_syscfg();
	syscfg = Q6_R_insert_RP(syscfg,configval,insertval);
	H2K_set_syscfg(syscfg);
	return 0;
}

u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)
{
	/* SSR/CCR gets saved/restored at trap time.  If that changes to switch
	 * time, modify SSR/CCR directly. */
#if ARCHV <= 3
	switch (whatcache) {
		case HWCONFIG_PREFETCH_HF_I: me->ssr = Q6_R_insert_RII(me->ssr,configval,1,22); break;
		case HWCONFIG_PREFETCH_HF_D: me->ssr = Q6_R_insert_RII(me->ssr,configval,1,23); break;
		case HWCONFIG_PREFETCH_SF_D: me->ssr = Q6_R_insert_RII(me->ssr,configval,1,24); break;
	}
#else
	switch (whatcache) {
		case HWCONFIG_PREFETCH_HF_I: me->ccr = Q6_R_insert_RII(me->ccr,configval,1,16); break;
		case HWCONFIG_PREFETCH_HF_D: me->ccr = Q6_R_insert_RII(me->ccr,configval,1,17); break;
		case HWCONFIG_PREFETCH_SF_D: me->ccr = Q6_R_insert_RII(me->ccr,configval,1,20); break;
		case HWCONFIG_PREFETCH_HF_I_L2: me->ccr = Q6_R_insert_RII(me->ccr,configval,1,18); break;
		case HWCONFIG_PREFETCH_SF_D_L2: me->ccr = Q6_R_insert_RII(me->ccr,configval,1,19); break;
	}
#endif
	return 0;
}

