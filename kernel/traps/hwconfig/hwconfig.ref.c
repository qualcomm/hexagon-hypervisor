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
#include <hlx.h>
#include <hmx.h>
#include <safemem.h>
#include <cfg_table.h>
#include <atomic.h>
#include <tlb.h>

#ifdef CLUSTER_SCHED
#include <readylist.h>
#include <runlist.h>
#include <dosched.h>
#endif

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
	H2K_trap_hwconfig_setcladereg,
	H2K_trap_hwconfig_hwthreads_num,
	H2K_trap_hwconfig_hwthreads_mask,
	H2K_trap_hwconfig_ecc,
	H2K_trap_hwconfig_hmxbits,
	H2K_trap_hwconfig_getdmacfg,
	H2K_trap_hwconfig_setdmacfg,
	H2K_trap_hwconfig_l2gclean,
	H2K_trap_hwconfig_getstrideprefetcherreg,
	H2K_trap_hwconfig_setstrideprefetcherreg,
	H2K_trap_hwconfig_set_hmx_power_on_start_addr,
	H2K_trap_hwconfig_set_hmx_power_off_start_addr,
	H2K_trap_hwconfig_gpio_toggle,
	H2K_trap_hwconfig_set_gpio_addr,
	H2K_trap_hwconfig_l2cp,
	H2K_trap_hwconfig_geteccreg,
	H2K_trap_hwconfig_getvwctrl,
	H2K_trap_hwconfig_setvwctrl,
	H2K_trap_hwconfig_get_dpm_voltlimitmgmt_reg,
	H2K_trap_hwconfig_set_dpm_voltlimitmgmt_reg,
	H2K_trap_hwconfig_hlxbits
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
	if (configtype >= HWCONFIG_MAX) 
	{
		return -1;
	}
	else
	{
		return H2K_hwconfigtab[configtype](0, ptr, val2, val3, me);
	}
	
}

static u32_t getxreg (u32_t cfg_offset, u32_t offset) {
	u32_t va;
	pa_t base;
	u32_t volatile *reg;
	u32_t ret;

	offset &= -4;

	base = H2K_cfg_table(cfg_offset) << CFG_TABLE_SHIFT;

	va = H2K_tmpmap_add_and_lock(base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

static u32_t setxreg(u32_t cfg_offset, u32_t offset, u32_t val) {
	u32_t va;
	pa_t base;
	u32_t volatile *reg;
	u32_t ret;

	offset &= -4;

	base = H2K_cfg_table(cfg_offset) << CFG_TABLE_SHIFT;

	va = H2K_tmpmap_add_and_lock(base, UNCACHED);
	reg = (u32_t *) (va + offset);
	ret = *reg;
	*reg = val;
	H2K_dccleana((void *)reg);
	H2K_tmpmap_remove_and_unlock();

	return ret;
}

u32_t H2K_trap_do_hwconfig_l2cache(u32_t unused, u32_t ecc_enable, u32_t size, u32_t use_wb, H2K_thread_context *me)
{
	u32_t cur_nwa;
	u32_t cur_nra;
	u32_t syscfg;
	u32_t tmp;
	u32_t i;

	/* Don't need to lock here since we only proceed in ST mode */
	syscfg = H2K_get_syscfg();
	cur_nwa = syscfg & SYSCFG_L2NWA;
	cur_nra = syscfg & SYSCFG_L2NRA;
	size &= 0x7;
	use_wb &= 1;

	/* ST Mode */
	if (H2K_stmode_begin() != 0) return -1;

	/* write-through, no write-alloc, no read-alloc */
	syscfg &= ~SYSCFG_L2WB;
	syscfg &= ~SYSCFG_L2NWA;
	syscfg &= ~SYSCFG_L2NRA;
	H2K_set_syscfg(syscfg);
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
	H2K_l2kill();
	H2K_isync();

	syscfg |= size << SYSCFG_L2CFG_BITS;
	syscfg |= use_wb << SYSCFG_L2WB_BIT;
	syscfg |= cur_nwa | cur_nra;
	
	/* ECC */
	if (ecc_enable != H2K_gp->ecc_enable) {
		for (i = 0; i < ECCREGS_NREGS; i++) {
			tmp = getxreg(CFG_TABLE_ECC_BASE, ECCREGS_STRIDE * i);
			tmp = Q6_R_insert_RII(tmp, ((ecc_enable & (1 << i)) ? ECCREGS_ENABLE : ECCREGS_DISABLE), ECCREGS_BITS, 0);
			setxreg(CFG_TABLE_ECC_BASE, ECCREGS_STRIDE * i, tmp);
		}
		H2K_gp->ecc_enable = ecc_enable;
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

u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me) {

	/* ecc_enable unchanged */
	return H2K_trap_do_hwconfig_l2cache(unused, H2K_gp->ecc_enable, size, use_wb, me);
}

u32_t H2K_trap_hwconfig_ecc(u32_t unused, void *unusedp, u32_t ecc_enable, u32_t unused3, H2K_thread_context *me) {
	u32_t syscfg;
	u32_t tmp, i;

	if (ecc_enable >= 0x1 << ECCREGS_NREGS) {  // out of range
		return -1;
	}

	/* get ECC enable state */
	for (i = 0; i < ECCREGS_NREGS; i++) {
		tmp = getxreg(CFG_TABLE_ECC_BASE, ECCREGS_STRIDE * i);
		if (tmp & ECCREGS_ENABLE) {
			H2K_gp->ecc_enable |= (1 << i);
		}
	}

	syscfg = H2K_get_syscfg();
	/* size and use_wb unchanged */
	return H2K_trap_do_hwconfig_l2cache(unused, ecc_enable, (syscfg & SYSCFG_L2CFG) >> SYSCFG_L2CFG_BITS, (syscfg & SYSCFG_L2WB) >> SYSCFG_L2WB_BIT, me);
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

u32_t H2K_trap_hwconfig_hlxbits(u32_t unused, void *unusedp,  u32_t xa3, u32_t xe3, H2K_thread_context *me) {
#ifdef HAVE_HLX
# ifdef CLUSTER_SCHED
	if (H2K_gp->cluster_sched) {
		BKL_LOCK();
		if (xe3 && !(me->ccr & CCR_XE3_BIT_MASK)) {  // turning xe3 on
			// block as if we got resched interrupt
			H2K_log("hthread %d  hlxbits: task 0x%08x  setting xe3\n", me->hthread, me);
			me->ccr = Q6_R_insert_RII(me->ccr, xa3, CCR_XA3_NBITS, CCR_XA3_BITS);
			me->ccr = Q6_R_insert_RII(me->ccr, xe3, 1, CCR_XE3_BIT);
			H2K_runlist_remove(me);
			H2K_ready_append(me);
			H2K_dosched(me, me->hthread);
		}
		if (!xe3 && (me->ccr & CCR_XE3_BIT_MASK)) {  // turning xe3 off
			xex_set_clr(me->hthread, 0, 0, 1);
			H2K_log("hthread %d  hmxbits: task 0x%08x  clearing xe3\n", me->hthread, me);
		}
		BKL_UNLOCK();
	}
# endif
	me->ccr = Q6_R_insert_RII(me->ccr, xa3, CCR_XA3_NBITS, CCR_XA3_BITS);
	me->ccr = Q6_R_insert_RII(me->ccr, xe3, 1, CCR_XE3_BIT);

	if (xe3) {
		H2K_hlx_poweron(); // make sure the lights are on
	}
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_hmxbits(u32_t unused, void *unusedp, u32_t xe2, u32_t xa2, H2K_thread_context *me) {
#if ARCHV >= 68
	if (0 < H2K_gp->hmx_units) {  // exists
#ifdef CLUSTER_SCHED
		if (H2K_gp->cluster_sched) {
			BKL_LOCK();
			if (xe2 && !(me->ssr & SSR_XE2_BIT_MASK)) {  // turning xe2 on
				// block as if we got resched interrupt
				H2K_log("hthread %d  hmxbits: task 0x%08x  setting xe2\n", me->hthread, me);
				me->ccr = Q6_R_insert_RII(me->ccr, xa2, CCR_XA2_NBITS, CCR_XA2_BITS);
				me->ssr = Q6_R_insert_RII(me->ssr, xe2, 1, SSR_XE2_BIT);
				H2K_runlist_remove(me);
				H2K_ready_append(me);
				H2K_dosched(me, me->hthread);
			}
			if (!xe2 && (me->ssr & SSR_XE2_BIT_MASK)) {  // turning xe2 off
# ifdef HAVE_HLX
				xex_set_clr(me->hthread, 0, 1, 0);
# else
				xex_set_clr(me->hthread, 0, 1);
# endif
				H2K_log("hthread %d  hmxbits: task 0x%08x  clearing xe2\n", me->hthread, me);
			}
			BKL_UNLOCK();
		}
#endif
		me->ccr = Q6_R_insert_RII(me->ccr, xa2, CCR_XA2_NBITS, CCR_XA2_BITS);
		me->ssr = Q6_R_insert_RII(me->ssr, xe2, 1, SSR_XE2_BIT);

		if (xe2) {
			H2K_hmx_poweron(); // make sure the lights are on
		}
		return 0;
	}
	return -1;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_extbits(u32_t unused, void *unusedp, u32_t xa, u32_t xe, H2K_thread_context *me) {
	/* FIXME: should check for allowed XA values here (maybe?) */
	/* EJP: Always allow XE/XA to be set if only for silver tests working also */

#ifdef CLUSTER_SCHED
	if (H2K_gp->cluster_sched) {
		BKL_LOCK();
		if (xe && !(me->ssr & SSR_XE_BIT_MASK)) {  // turning xe on
			// block as if we got resched interrupt
			H2K_log("hthread %d  extbits:  task 0x%08x  setting xe\n", me->hthread, me);

			if ((xa < EXT_HVX_XA_START || xa >= EXT_HVX_XA_START + H2K_gp->coproc_contexts)  // not in HVX range
#ifdef DO_EXT_SWITCH
					|| (!(me->vmblock->do_ext))
#endif
					) {
				me->ssr = Q6_R_insert_RII(me->ssr, xa, SSR_XA_NBITS, SSR_XA_BITS);
				me->ssr = Q6_R_insert_RII(me->ssr, xe, 1, SSR_XE_BIT);
				H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_SAVEXT_BIT);
			}
			/* else (when in hvx range and do_ext) kernel is managing xa/xe, so do nothing here */
			H2K_runlist_remove(me);
			H2K_ready_append(me);
			H2K_dosched(me, me->hthread);
		}
		if (!xe && (me->ssr & SSR_XE_BIT_MASK)) {  // turning xe off
# ifdef HAVE_HLX
				xex_set_clr(me->hthread, 1, 0, 0);
# else
				xex_set_clr(me->hthread, 1, 0);
# endif
			H2K_log("hthread %d  extbits: task 0x%08x  clearing xe\n", me->hthread, me);
		}
		BKL_UNLOCK();
	}
#endif

	if ((xa < EXT_HVX_XA_START || xa >= EXT_HVX_XA_START + H2K_gp->coproc_contexts)  // not in HVX range
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

u32_t H2K_trap_hwconfig2_hlxbits(u32_t unused, void *unusedp, u32_t xa3, u32_t xe3, H2K_thread_context *me) {
	/* FIXME: should check for allowed XA3 values here (maybe?) */
	/* EJP: Always allow XE3/XA3 to be set if only for silver tests working also */
	
#ifdef CLUSTER_SCHED
	if (H2K_gp->cluster_sched) {
		BKL_LOCK();
		if (xe3 && !(me->ccr & CCR_XE3_BIT_MASK)) {  // turning xe3 on
			// block as if we got resched interrupt
			H2K_log("hthread %d  extbits:  task 0x%08x  setting xe3\n", me->hthread, me);

			if ((xa3 < EXT_HLX_XA3_START || xa3 >= EXT_HLX_XA3_START + H2K_gp->hlx_contexts)  // not in HLX range //TODO: Do we need to do this for HLX
#ifdef DO_EXT_SWITCH
					|| (!(me->vmblock->do_ext))//TODO: Do we need to do this for HLX
#endif
					) {
				me->ccr = Q6_R_insert_RII(me->ccr, xa3, CCR_XA3_NBITS, CCR_XA3_BITS);
				me->ccr = Q6_R_insert_RII(me->ccr, xe3, 1, CCR_XE3_BIT);
				H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_SAVEXT_BIT); //TODO: Do we need to do this for HLX
			}
			/* else (when in hlx range and do_ext) kernel is managing xa/xe, so do nothing here */
			H2K_runlist_remove(me);
			H2K_ready_append(me);
			H2K_dosched(me, me->hthread);
		}
		if (!xe3 && (me->ccr & CCR_XE3_BIT_MASK)) {  // turning xe off
			xex_set_clr(me->hthread, 0, 0, 1);
			H2K_log("hthread %d  extbits: task 0x%08x  clearing xe3\n", me->hthread, me);
		}
		BKL_UNLOCK();
	}
#endif

	if ((xa3 < EXT_HLX_XA3_START || xa3 >= EXT_HLX_XA3_START + H2K_gp->hlx_contexts)  // not in HLX range
#ifdef DO_EXT_SWITCH
			|| (!(me->vmblock->do_ext))
#endif
			) {
		me->ccr = Q6_R_insert_RII(me->ccr, xa3, CCR_XA3_NBITS, CCR_XA3_BITS);
		me->ccr = Q6_R_insert_RII(me->ccr, xe3, 1, CCR_XE3_BIT);
		H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_SAVEXT_BIT); //TODO: Do we need to do this for HLX
	}
	/* else (when in hlx range and do_ext) kernel is managing xa3/xe3, so do nothing here */
#ifdef HAVE_HLX
	if (xe3) {
		H2K_hlx_poweron(); // make sure the lights are on
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
		H2K_gp->info_boot_flags.boot_ext_ok = H2K_gp->info_boot_flags.boot_have_hvx && (!(H2K_gp->syscfg_val & SYSCFG_V2X)) && (H2K_gp->hthreads <= H2K_gp->coproc_contexts);
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

	/* Both HVX HMX now */
	if (state) {
		H2K_hvx_poweron();
# ifdef HAVE_HLX
		H2K_hlx_poweron();
# endif
		H2K_hmx_poweron();
	} else {
		H2K_hvx_poweroff();
# ifdef HAVE_HLX
		H2K_hlx_poweroff();
# endif
		H2K_hmx_poweroff();
	}
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_getl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return 0;
	}

	return getxreg(CFG_TABLE_L2REGS, offset);
}

u32_t H2K_trap_hwconfig_getcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	u32_t val;
	u32_t idx = (0 == offset ? 0 : ((offset % CLADE_REG_PD_CHUNK) / 4) + 1);

	if (offset > CLADEREGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_CLADEREG_RANGE;
		return 0;
	}

	val = getxreg(CFG_TABLE_CLADEREGS, offset);

	return val << (clade_regs[idx].addrbit - clade_regs[idx].regbit);
}

u32_t H2K_trap_hwconfig_setl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
	if (offset > L2REGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_L2REG_RANGE;
		return 0;
	}

	return setxreg(CFG_TABLE_L2REGS, offset, val);
}

u32_t H2K_trap_hwconfig_setcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
	u32_t idx = (0 == offset ? 0 : ((offset % CLADE_REG_PD_CHUNK) / 4) + 1);
	u32_t ret;

	if (offset > CLADEREGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_CLADEREG_RANGE;
		return 0;
	}

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

u32_t H2K_trap_hwconfig_hwintop(u32_t unused, void *unusedp, u32_t op_and_int, u32_t val, H2K_thread_context *me)
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

u32_t H2K_trap_hwconfig_hwthreads_mask(u32_t unused, void *unusedp, u32_t mask, u32_t unused3, H2K_thread_context *me) {

	mask |= 0x1;  // thread 0 stays on
	H2K_start_threads(mask);
	H2K_isync();

	asm ( " %0 = modectl " :"=r"(H2K_gp->hthreads_mask));
	H2K_gp->hthreads_mask &= 0xffff;
	H2K_gp->hthreads = Q6_R_popcount_P(H2K_gp->hthreads_mask);

#ifdef CLUSTER_SCHED
	H2K_cluster_config();
#endif
	return H2K_gp->hthreads_mask;
}

u32_t H2K_trap_hwconfig_hwthreads_num(u32_t unused, void *unusedp, u32_t num, u32_t unused3, H2K_thread_context *me) {

	u32_t i = 0;
	u32_t nthreads = 0;
	u32_t new_mask = 0;

	/* if (0 >= num || num > MAX_HTHREADS) { */
	/* 	return -1; */
	/* } */

	if (0x65 < H2K_gp->arch) {  // hthreads_mask in cfg_table
		H2K_gp->hthreads_mask = H2K_cfg_table(CFG_TABLE_HTHREADS_MASK);

		if (Q6_R_popcount_P(H2K_gp->hthreads_mask) < num) {
			num = Q6_R_popcount_P(H2K_gp->hthreads_mask);
		}
		while (nthreads < num) {
			if (H2K_gp->hthreads_mask & (1 << i)) {
				new_mask |= (1 << i);
				nthreads++;
			}
			i++;
		}
		H2K_gp->hthreads_mask = new_mask;
	} else {  // thread numbers are contiguous for ARCHV <= 65
		H2K_gp->hthreads_mask = (1 << num) - 1;
	}
	H2K_start_threads(H2K_gp->hthreads_mask);
	H2K_isync();
	asm ( " %0 = modectl " :"=r"(H2K_gp->hthreads_mask));
	H2K_gp->hthreads_mask &= 0xffff;
	H2K_gp->hthreads = Q6_R_popcount_P(H2K_gp->hthreads_mask);

#ifdef CLUSTER_SCHED
	H2K_cluster_config();
#endif
	return H2K_gp->hthreads;
}

u32_t H2K_trap_hwconfig_getdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t unused3, H2K_thread_context *me) {

	u32_t ret = -1;

#if ARCHV >= 68
	if (H2K_gp->dma_version) {
		ret = H2K_dmcfgrd(index);
	}
#endif
	return ret;
}

u32_t H2K_trap_hwconfig_setdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t data, H2K_thread_context *me) {

#if ARCHV >= 68
	if (H2K_gp->dma_version) {
		H2K_dmcfgwr(index, data);
		return 0;
	}
#endif
	return -1;
}

u32_t H2K_trap_hwconfig_l2gclean(u32_t unused, void *unusedp, u32_t inv, u32_t unused3, H2K_thread_context *me) {

#if ARCHV >= 60
	H2K_syncht();
	if (inv) {
		H2K_l2gcleaninv();
	} else {
		H2K_l2gclean();
	}
	return 0;
#endif
	return -1;
}

u32_t H2K_trap_hwconfig_getstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	if ((offset > H2K_gp->hthreads * 4) 
			|| ((H2K_cfg_table(CFG_TABLE_CORECFG_PRESENT) & CORECFG_PRESENT_STRIDE_PREFETCHER_MASK) == 0)) {  // out of range: reg0 + per-thread regs
		H2K_gp->kernel_error = KERROR_HWCONFIG_STRIDE_PREFETCHER_RANGE;
		return 0;
	}

	return getxreg(CFG_TABLE_CORECFG_BASE, CORECFG_STRIDE_PREFETCHER_BASE + offset);
}

u32_t H2K_trap_hwconfig_setstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
	if ((offset > H2K_gp->hthreads * 4) 
			|| ((H2K_cfg_table(CFG_TABLE_CORECFG_PRESENT) & CORECFG_PRESENT_STRIDE_PREFETCHER_MASK) == 0)) {  // out of range: reg0 + per-thread regs
		H2K_gp->kernel_error = KERROR_HWCONFIG_STRIDE_PREFETCHER_RANGE;
		return 0;
	}

	return setxreg(CFG_TABLE_CORECFG_BASE, CORECFG_STRIDE_PREFETCHER_BASE + offset, val);
}

u32_t H2K_trap_hwconfig_set_hmx_power_on_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me) {
#if ARCHV >= 68
	H2K_gp->hmx_rsc_seq_power_on_start_addr = addr;
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_set_hmx_power_off_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me) {
#if ARCHV >= 68
	H2K_gp->hmx_rsc_seq_power_off_start_addr = addr;
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_gpio_toggle(u32_t unused, void *unusedp, u32_t on, u32_t unused3, H2K_thread_context *me) {
#if ARCHV >= 68
	if (0 == H2K_gp->gpio_reg) return -1;  // not set
	
	u32_t volatile *reg = (u32_t *)GPIO_VA;
	u32_t val = ((*reg) & ~(0x3c)) | 0x200;

	*reg = val;
	if (on) {
		*(reg + 0x1) = 0x2;
	} else {
		*(reg + 0x1) = 0x0;
	}
	return 0;
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_set_gpio_addr(u32_t unused, void *unusedp, u32_t addr_hi, u32_t addr_lo, H2K_thread_context *me) {
#if ARCHV >= 68
	H2K_mem_tlbfmt_t entry;

	H2K_gp->gpio_reg = ((pa_t)addr_hi) << 32 | addr_lo;

	entry.raw = 0;
	entry.ppd = ((H2K_gp->gpio_reg & GPIO_PG_MASK ) >> (PAGE_BITS - 1)) | (1 << GPIO_PG_SIZE);
	entry.pa35 = (H2K_gp->gpio_reg >> 35) & 0x1;
#if ARCHV >= 73
	entry.pa3637 = (H2K_gp->gpio_reg >> 36) & 0x3;
#endif
	entry.cccc = DEVICE_TYPE;
	entry.xwru = 0;  // only monitor access
	entry.vpn = (GPIO_VA >> PAGE_BITS);
	// entry.asid = don't care
	entry.global = 1;
	entry.valid = 1;
	return H2K_tlb_tlbop(TLBOP_TLBALLOC, 0, entry.raw, me);
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_l2cp(u32_t unused, void *unusedp, u32_t configval, u32_t unused3, H2K_thread_context *me) {
	/* SSR/CCR gets saved/restored at trap time.  If that changes to switch
	 * time, modify SSR/CCR directly. */
	me->ccr = Q6_R_insert_RII(me->ccr, configval, CCR_L2CP_NBITS, CCR_L2CP_BITS);
	return 0;
}

u32_t H2K_trap_hwconfig_geteccreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
	if (offset > ECCREGS_MAX) {  // out of range
		H2K_gp->kernel_error = KERROR_HWCONFIG_ECCREG_RANGE;
		return 0;
	}

	return getxreg(CFG_TABLE_ECC_BASE, offset);
}

u32_t H2K_trap_hwconfig_getvwctrl(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me) {

#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	if (0x79 <= H2K_gp->arch) {
		return H2K_get_vwctrl();
	} else {
		return -1;
	}
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_setvwctrl(u32_t unused, void *unusedp, u32_t val, u32_t unused3, H2K_thread_context *me) {

#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	if (0x79 <= H2K_gp->arch) {
		me->vwctrl = val;
		H2K_set_vwctrl(val);
		return 0;
	} else {
		return -1;
	}
#else
	return -1;
#endif
}

u32_t H2K_trap_hwconfig_get_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) {
  if (0x79 <= H2K_gp->arch) {  // hthreads_mask in cfg_table
    
    if ((H2K_cfg_table(CFG_TABLE_CORECFG_PRESENT) & CORECFG_PRESENT_DPM_VOLTLMTMGMT_MASK) == 0) {  // out of range: reg0 + per-thread regs
      H2K_gp->kernel_error = KERROR_HWCONFIG_DPM_VOLTLMTMGMT_RANGE;
      return 0;
    }
    
    return getxreg(CFG_TABLE_CORECFG_BASE, CORECFG_DPM_VOLTLMTMGMT_BASE + offset);
  }
  else {
    return 0;
  }
}

u32_t H2K_trap_hwconfig_set_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) {
  if (0x79 <= H2K_gp->arch) {  // hthreads_mask in cfg_table

    if ((H2K_cfg_table(CFG_TABLE_CORECFG_PRESENT) & CORECFG_PRESENT_DPM_VOLTLMTMGMT_MASK) == 0) {  // out of range: reg0 + per-thread regs
      H2K_gp->kernel_error = KERROR_HWCONFIG_DPM_VOLTLMTMGMT_RANGE;
      return 0;
    }
    
    return setxreg(CFG_TABLE_CORECFG_BASE, CORECFG_DPM_VOLTLMTMGMT_BASE + offset, val);
  }
  else {
    return 0;
  }
}

