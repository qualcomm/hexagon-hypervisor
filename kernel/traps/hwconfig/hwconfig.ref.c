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
#include <physread.h>
#include <intcontrol.h>
#include <tmpmap.h>
#include <hvx.h>
#include <safemem.h>

typedef u32_t (*configptr_t)(u32_t, void *, u32_t, u32_t, H2K_thread_context *);

static const configptr_t H2K_hwconfigtab[HWCONFIG_MAX] IN_SECTION(".data.config.hwconfig") = {
	H2K_trap_hwconfig_l2cache,
	H2K_trap_hwconfig_partitions,
	H2K_trap_hwconfig_prefetch,
	H2K_trap_hwconfig_extbits,
	H2K_trap_hwconfig_vlength,
	H2K_trap_hwconfig_extpower,
	H2K_trap_hwconfig_getl2reg,
	H2K_trap_hwconfig_setl2reg,
	H2K_trap_hwconfig_l2locka,
	H2K_trap_hwconfig_l2unlock,
	H2K_trap_hwconfig_hwintop,
};

u32_t H2K_trap_hwconfig(hwconfig_type_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if (configtype >= HWCONFIG_MAX) return -1;
	return H2K_hwconfigtab[configtype](0, ptr, val2, val3, me);
}

u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me)
{
	u32_t cur_size;
	u32_t cur_wb;
	u32_t cur_nwa;
	u32_t cur_nra;
	u32_t syscfg;

	/* Don't need to lock here since we only proceed in ST mode */
	syscfg = H2K_get_syscfg();
	cur_size = (syscfg & SYSCFG_L2CFG) >> SYSCFG_L2CFG_BITS;
	cur_wb = syscfg & SYSCFG_L2WB;
	cur_nwa = syscfg & SYSCFG_L2NWA;
	cur_nra = syscfg & SYSCFG_L2NRA;
	size &= 0x7;
	use_wb &= 1;

	/* ST Mode */
	if (H2K_stmode_begin() != 0) return -1;

	if (size != cur_size) {
		/* write-through, no write-alloc, no read-alloc */
		syscfg &= ~SYSCFG_L2WB;
		syscfg &= ~SYSCFG_L2NWA;
		syscfg &= ~SYSCFG_L2NRA;
		H2K_set_syscfg(syscfg);
		H2K_gp->syscfg_val = syscfg;
		H2K_syncht();

		/* Clean entire cache */
#if ARCHV >= 60
		H2K_l2gcleaninv();
#else
		H2K_cache_l2_cleaninv();
#endif

		/* Set to 0 size */
		syscfg &= ~SYSCFG_L2CFG;
		H2K_set_syscfg(syscfg);
		H2K_gp->syscfg_val = syscfg;
		H2K_l2kill();
		H2K_isync();
		/* Update size, mode */
		syscfg |= ((size << SYSCFG_L2CFG_BITS) | (use_wb ? SYSCFG_L2WB : 0));
		syscfg |= cur_wb | cur_nwa | cur_nra;

	} else if (use_wb && !cur_wb) {
		syscfg |= SYSCFG_L2WB;

	} else if (!use_wb && cur_wb) {
		/* Just leave WB mode */
		/* Clean entire cache */
#if ARCHV >= 60
		H2K_l2gcleaninv();
#else
		H2K_cache_l2_cleaninv();
#endif
		/* Disable WB mode on L2$ */
		syscfg &= ~SYSCFG_L2WB;
	}

	H2K_syncht();
	H2K_set_syscfg(syscfg);
	H2K_gp->syscfg_val = syscfg;
	H2K_syncht();
	H2K_stmode_end();
	return 0;
}

u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)
{
	u32_t syscfg;
	u64_t insertval;
	insertval = (2ULL << 32) | (25+(2*whatcache));
	BKL_LOCK();
	syscfg = H2K_get_syscfg();
	syscfg = Q6_R_insert_RP(syscfg,configval,insertval);
	H2K_set_syscfg(syscfg);
	H2K_gp->syscfg_val = syscfg;
	BKL_UNLOCK();
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

u32_t H2K_trap_hwconfig_extbits(u32_t unused, void *unusedp, u32_t xa, u32_t xe, H2K_thread_context *me) {
	/* FIXME: should check for allowed XA values here (maybe?) */
	/* EJP: Always allow XE/XA to be set if only for silver tests working also */
	me->ssr = Q6_R_insert_RII(me->ssr, xa, SSR_XA_NBITS, SSR_XA_BITS);
	me->ssr = Q6_R_insert_RII(me->ssr, xe, 1, SSR_XE_BIT);
#ifdef HAVE_EXTENSIONS
	if (xe) {
		H2K_hvx_poweron(); // make sure the lights are on
	}

#endif
	return 0;
}

u32_t H2K_trap_hwconfig_vlength(u32_t unused, void *unusedp, u32_t vlength, u32_t unused3, H2K_thread_context *me) {

#ifdef HAVE_EXTENSIONS

	u32_t cur, new;
	
/* FIXME: This needs to be virtualized */

	BKL_LOCK();
	cur = H2K_get_syscfg();
	if (vlength >= V2X_LENGTH) {  // turn on long vectors
		new = cur | SYSCFG_V2X;
	} else {
		new = cur & ~SYSCFG_V2X;
	}
	if (new != cur) {
		H2K_set_syscfg(new);
		H2K_gp->syscfg_val = new;
		H2K_isync();
		cur = H2K_get_syscfg();
		if (cur != new) {  // failed
			BKL_UNLOCK();
			return -1;
		}
	}

	BKL_UNLOCK();
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_extpower(u32_t unused, void *unusedp, u32_t state, u32_t unused3, H2K_thread_context *me) {

#ifdef HAVE_EXTENSIONS

	/* Just HVX for now */
	if (state) {
		H2K_hvx_poweron();
	} else {
		H2K_hvx_poweroff();
	}
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_getl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)
{
	u32_t va;
	u32_t cfg;
	u32_t l2_cfg_base;
	u32_t volatile *reg;
	u32_t ret;

	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return -1;
	}

	asm volatile
		(
		 " %0 = cfgbase \n"
		 : "=r" (cfg)
		 );

	l2_cfg_base = H2K_mem_physread_word((cfg << 16) + CFG_TABLE_L2REGS) << 16;

	va = H2K_tmpmap_add_and_lock(l2_cfg_base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

u32_t H2K_trap_hwconfig_setl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)
{
	u32_t va;
	u32_t cfg;
	u32_t l2_cfg_base;
	u32_t volatile *reg;
	u32_t ret;

	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return -1;
	}

	asm volatile
		(
		 " %0 = cfgbase \n"
		 : "=r" (cfg)
		 );

	l2_cfg_base = H2K_mem_physread_word((cfg << 16) + CFG_TABLE_L2REGS) << 16;

	va = H2K_tmpmap_add_and_lock(l2_cfg_base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	*reg = val;
	H2K_dccleana((void *)reg);
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

u32_t H2K_trap_hwconfig_l2locka(u32_t unused, void *addr, u32_t len, u32_t unused3, H2K_thread_context *me)
{
#if ARCHV >= 56
#if ARCHV >= 60
#define L2LINESIZE 64
#else
#define L2LINESIZE 32
#endif
	pa_t pa;
	char *caddr = addr;
	u32_t off;
	u32_t count;
	u32_t ret = 1;
	if (!H2K_safemem_check_and_lock(addr,SAFEMEM_RW,&pa,me)) return 1;
	/* EJP: FIXME: need to check for every page */
	for (off = 0; off < len; off += L2LINESIZE) {
		count = 0;
		while (H2K_l2locka(caddr+off) == 0) {
			if (++count > (1024*1024)) goto fail;
		}
	}
	ret = 0;
fail:
	H2K_safemem_unlock();
	return ret;
#else
	return 1;
#endif
}

u32_t H2K_trap_hwconfig_l2unlock(u32_t unused, void *addr, u32_t len, u32_t unused3, H2K_thread_context *me)
{
#if ARCHV >= 56
	/* EJP: for small ones, just unlock a range? Indicate all with addr=NULL? */
	H2K_l2unlock();
	return 0;
#else
	return 1;
#endif
}

u32_t H2K_trap_hwconfig_hwintop(u32_t unused, void *unusedptr, u32_t op_and_int, u32_t val, H2K_thread_context *me)
{
	u32_t op = op_and_int >> 16;
	u32_t intno = op_and_int & 0xFFFF;
	if (intno >= MAX_INTERRUPTS) return 1;
	if (op >= HWCONFIG_HWINTOP_XXX_LAST) return 1;
	switch (op) {
	case HWCONFIG_HWINTOP_ENABLE: H2K_intcontrol_enable(intno); break;
	case HWCONFIG_HWINTOP_DISABLE: H2K_intcontrol_disable(intno); break;
	case HWCONFIG_HWINTOP_RAISE: H2K_intcontrol_raise(intno); break;
	}
	return 0;
}
