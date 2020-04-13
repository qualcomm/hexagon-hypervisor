/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pmu.h>
#include <asm_offsets.h>
#include <thread.h>
#include <globals.h>
#include <atomic.h>
#include <hw.h>
#include <id.h>
#include <hexagon_protos.h>

u32_t trap_pmuctrl_threadset(u32_t unused, u32_t vdest, u32_t turnon, u32_t unused3, H2K_thread_context *me);

typedef u32_t (*pmuctrlptr_t)(u32_t, u32_t, u32_t, u32_t, H2K_thread_context *);

static const pmuctrlptr_t H2K_pmuctrltab[PMUCTRL_MAX] IN_SECTION(".data.config.pmuctrl") = {
	trap_pmuctrl_threadset,
	H2K_trap_pmuctrl_setreg,
	H2K_trap_pmuctrl_getreg
};

u32_t H2K_trap_pmuctrl(pmuop_type configtype, u32_t val1, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if ((configtype == PMUCTRL_THREADSET) || (configtype >= PMUCTRL_MAX)) return -1;
	return H2K_pmuctrltab[configtype](0,val1,val2,val3,me);
}

u32_t trap_pmuctrl_threadset(u32_t unused, u32_t vdest, u32_t turnon, u32_t unused3, H2K_thread_context *me)
{
	return -1; // functionality removed
}

u32_t H2K_trap_pmuctrl_setreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t newval, H2K_thread_context *me)
{
	switch ((s32_t)whichreg) {
	case 8: H2K_set_pmuevtcfg(newval); return 0;
	case 10: H2K_set_pmucfg(newval); return 0;

#ifdef COUNT_TLB_EVENTS
	case -2: me->vmblock->tlbmissx_lo = newval;
	case -3: me->vmblock->tlbmissx_hi = newval;
	case -4: me->vmblock->tlbmissrw_lo = newval;
	case -5: me->vmblock->tlbmissrw_hi = newval;

	case -6: me->vmblock->stlbmiss_lo = newval;
	case -7: me->vmblock->stlbmiss_hi = newval;
#endif

	case 0: H2K_set_pmucnt0(newval); return 0;
	case 1: H2K_set_pmucnt1(newval); return 0;
	case 2: H2K_set_pmucnt2(newval); return 0;
	case 3: H2K_set_pmucnt3(newval); return 0;
	default: return -1;
	}
	return -1;
}

u32_t H2K_trap_pmuctrl_getreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t unused3, H2K_thread_context *me)
{
	switch ((s32_t)whichreg) {
	case 8: return H2K_get_pmuevtcfg();
	case 10: return H2K_get_pmucfg();

#ifdef COUNT_TLB_EVENTS
	case -2: return me->vmblock->tlbmissx_lo;
	case -3: return me->vmblock->tlbmissx_hi;
	case -4: return me->vmblock->tlbmissrw_lo;
	case -5: return me->vmblock->tlbmissrw_hi;

	case -6: return me->vmblock->stlbmiss_lo;
	case -7: return me->vmblock->stlbmiss_hi;
#endif

	case 0: return H2K_get_pmucnt0();
	case 1: return H2K_get_pmucnt1();
	case 2: return H2K_get_pmucnt2();
	case 3: return H2K_get_pmucnt3();
	default: return -1;
	}
	return -1;
}

