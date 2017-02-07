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
#include <cfg_table.h>
#include <atomic.h>

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
	H2K_trap_hwconfig_getcladereg,
	H2K_trap_hwconfig_setcladereg
};

typedef struct {
	u32_t regbit:5;
	u32_t addrbit:5;
	u32_t unused:22;
} clade_reg_t;

static const clade_reg_t clade_regs[] = {
	{ 0, 29},  // region
	{12, 16},  // comp
	{ 8, 12},  // ex_hi
	{ 0,  0},  // ex_lo_small
	{ 8, 12}   // ex_lo_large
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
		/* EJP: FIXME: in ARCHV >= 60 H2K_cache_l2_cleaninv() should just be H2K_l2gclenainv() */
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

	H2K_gp->l2tags = size;
	H2K_gp->tcm_size = H2K_gp->l2size - (H2K_gp->l2tags > 0 ? (1 << H2K_gp->l2tags) * L2_TAG_CHUNK : 0);

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

	if ((xa < EXT_HVX_XA_START || xa >= EXT_HVX_XA_START + H2K_gp->hvx_contexts)  // not in HVX range
#ifdef DO_EXT_SWITCH
			|| (!(me->vmblock->do_ext))
#endif
			) {
		me->ssr = Q6_R_insert_RII(me->ssr, xa, SSR_XA_NBITS, SSR_XA_BITS);
		me->ssr = Q6_R_insert_RII(me->ssr, xe, 1, SSR_XE_BIT);
		H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_SAVEXT_BIT);
	}
	/* else (when in hvx range and do_ext) kernel is managing xa/xe, so do nothing here */
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
	if (vlength > Q6_R_ct0_R(H2K_gp->hvx_vlength)) {  // turn on long vectors
		new = cur | SYSCFG_V2X;
	} else {
		new = cur & ~SYSCFG_V2X;
	}
	if (new != cur) {
		H2K_set_syscfg(new);
		H2K_isync();
		cur = H2K_get_syscfg();
		H2K_gp->syscfg_val = cur;

#ifdef DO_EXT_SWITCH
		H2K_gp->info_boot_flags.boot_ext_ok = H2K_gp->info_boot_flags.boot_have_hvx && (!(H2K_gp->syscfg_val & SYSCFG_V2X)) && (H2K_gp->hthreads <= H2K_gp->hvx_contexts);
		if (H2K_gp->info_boot_flags.boot_ext_ok) {
			if (me->vmblock->use_ext) {
				me->vmblock->do_ext = 1;
				H2K_hvx_poweron();
			}
		} else {
			me->vmblock->do_ext = 0;
			/* Forget about live HVX regs when enabling long vectors */
			H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_SAVEXT_BIT);
		}
#endif

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

static u32_t getxreg (u32_t cfg_offset, u32_t offset) {
	u32_t va;
	pa_t base;
	u32_t volatile *reg;
	u32_t ret;

	base = H2K_cfg_table(cfg_offset) << CFG_TABLE_SHIFT;

	va = H2K_tmpmap_add_and_lock(base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

u32_t H2K_trap_hwconfig_getl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return -1;
	}

	/* FIXME: This could return -1 */
	return getxreg(CFG_TABLE_L2REGS, offset);
}

u32_t H2K_trap_hwconfig_getcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	u32_t val;
	u32_t idx = (0 == offset ? 0 : ((offset % CLADE_REG_PD_CHUNK) / 4) + 1);

	if (offset > CLADEREGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_CLADEREG_RANGE;
		return -1;
	}

	/* FIXME: This could return -1 */
	val = getxreg(CFG_TABLE_CLADEREGS, offset);

	return val << (clade_regs[idx].addrbit - clade_regs[idx].regbit);
}

static u32_t setxreg(u32_t cfg_offset, u32_t offset, u32_t val) {
	u32_t va;
	pa_t base;
	u32_t volatile *reg;
	u32_t ret;

	base = H2K_cfg_table(cfg_offset) << CFG_TABLE_SHIFT;

	va = H2K_tmpmap_add_and_lock(base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	*reg = val;
	H2K_dccleana((void *)reg);
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

u32_t H2K_trap_hwconfig_setl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return -1;
	}

	/* FIXME: This could return -1 */
	return setxreg(CFG_TABLE_L2REGS, offset, val);
}

u32_t H2K_trap_hwconfig_setcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
	u32_t idx = (0 == offset ? 0 : ((offset % CLADE_REG_PD_CHUNK) / 4) + 1);
	u32_t ret;

	if (offset > CLADEREGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_CLADEREG_RANGE;
		return -1;
	}

	/* FIXME: This could return -1 */
	ret = setxreg(CFG_TABLE_CLADEREGS, offset, val >> (clade_regs[idx].addrbit - clade_regs[idx].regbit));
	return ret << (clade_regs[idx].addrbit - clade_regs[idx].regbit);
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
