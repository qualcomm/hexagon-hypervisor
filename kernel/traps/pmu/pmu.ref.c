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

typedef u32_t (*pmuconfigptr_t)(u32_t, u32_t, u32_t, u32_t, H2K_thread_context *);

#define MAX_PMUCONFIGS 3

static const pmuconfigptr_t H2K_pmuconfigtab[MAX_PMUCONFIGS] IN_SECTION(".data.config.pmuconfig") = {
	H2K_trap_pmuconfig_threadset,
	H2K_trap_pmuconfig_setreg,
	H2K_trap_pmuconfig_getreg
};

u32_t H2K_trap_pmuconfig(u32_t configtype, u32_t val1, u32_t val2, u32_t val3, H2K_thread_context *me)
{
	if (configtype >= MAX_PMUCONFIGS) return -1;
	return H2K_pmuconfigtab[configtype](0,val1,val2,val3,me);
}

u32_t H2K_trap_pmuconfig_threadset(u32_t unused, u32_t vdest, u32_t turnon, u32_t unused2, H2K_thread_context *me)
{
	u32_t val;
	H2K_id_t id;
	H2K_thread_context *dest;
	id.raw = vdest;
	if ((dest = H2K_id_to_context(id)) == NULL) return -1;
	turnon = (turnon != 0);
	if (dest->status == H2K_STATUS_DEAD) return -1;
	BKL_LOCK();
	H2K_atomic_insert(&dest->atomic_status_word,turnon,8,24);
	if (dest->status == H2K_STATUS_RUNNING) {
		val = H2K_get_pmucfg();
		if (turnon) {
			val = Q6_R_setbit_RR(val,dest->hthread);
		} else {
			val = Q6_R_clrbit_RR(val,dest->hthread);
		}
		H2K_set_pmucfg(val);
	}
	BKL_UNLOCK();
	return 0;
}

u32_t H2K_trap_pmuconfig_setreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t newval, H2K_thread_context *me)
{
	switch (whichreg) {
	case 0xFFFFFFFF: H2K_set_pmuevtcfg(newval); return 0;

#ifdef COUNT_TLB_MISSES
	case 0xfffffffe: me->vmblock->tlbmissx_lo = newval;
	case 0xfffffffd: me->vmblock->tlbmissx_hi = newval;
	case 0xfffffffc: me->vmblock->tlbmissrw_lo = newval;
	case 0xfffffffb: me->vmblock->tlbmissrw_hi = newval;
#endif

	case 0x0: H2K_set_pmucnt0(newval); return 0;
	case 0x1: H2K_set_pmucnt1(newval); return 0;
	case 0x2: H2K_set_pmucnt2(newval); return 0;
	case 0x3: H2K_set_pmucnt3(newval); return 0;
	default: return -1;
	}
	return -1;
}

u32_t H2K_trap_pmuconfig_getreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t unused3, H2K_thread_context *me)
{
	switch (whichreg) {
	case 0xFFFFFFFF: return H2K_get_pmuevtcfg();

#ifdef COUNT_TLB_MISSES
	case 0xfffffffe: return me->vmblock->tlbmissx_lo;
	case 0xfffffffd: return me->vmblock->tlbmissx_hi;
	case 0xfffffffc: return me->vmblock->tlbmissrw_lo;
	case 0xfffffffb: return me->vmblock->tlbmissrw_hi;
#endif

	case 0x0: return H2K_get_pmucnt0();
	case 0x1: return H2K_get_pmucnt1();
	case 0x2: return H2K_get_pmucnt2();
	case 0x3: return H2K_get_pmucnt3();
	default: return -1;
	}
	return -1;
}

