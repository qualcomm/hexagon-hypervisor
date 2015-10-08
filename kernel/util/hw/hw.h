/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _HEADER_HW_H
#define _HEADER_HW_H 1

#include <max.h>
#include <hexagon_protos.h>
#include <context.h>

static inline void ciad(u32_t mask)
{
	asm (" ciad(%0) // clear IAD " : : "r"(mask));
}

#if (ARCHV >= 4)
static inline void siad(u32_t mask)
{
	asm (" siad(%0) // Set IAD " : : "r"(mask));
}
#endif

static inline void swi(u32_t mask)
{
	asm (" swi(%0) // clear IAD " : : "r"(mask));
}

static inline void H2K_hw_trace(u32_t val)
{
	asm (" trace(%0) // trace " : : "r"(val));
}

#if (ARCHV <= 2)
static inline void int_hthread(u32_t hthread)
{
	asm(" swi(%0) // wake up a hw thread " : : "r"(HW_TH_0_INTMASK<<hthread));
}

static inline void int_hthread_mask(u32_t mask)
{
	asm(" swi(%0) // wake up a hw thread " : : "r"(mask<<HW_TH_0_LEFTSHIFT));
}
#elif (ARCHV >= 3)
static inline void change_imask(u32_t thread, u32_t imask)
{
	asm(" p0 = %0\n setimask(p0,%1)" : : "r"(thread),"r"(imask):"p0");
}

static inline u32_t get_imask(u32_t thread)
{
	u32_t imask;
	asm(" %0 = getimask(%1)" : "=r"(imask) : "r"(thread));
	return imask;
}

static inline void iassignw(u32_t intno, u32_t threadmask)
{
	asm(" iassignw(%0)" : : "r"(Q6_R_combine_RlRl(intno,threadmask)));
}

static inline u32_t iassignr(u32_t intno)
{
	u32_t ret;
	asm(" %0=iassignr(%1)" : "=r"(ret): "r"(intno<<16));
	return ret;
}

static inline u32_t hthreads_mask()
{
	iassignw(0,-1);
	return iassignr(0);
}

static inline u32_t get_hthreads()
{
#if ARCHV >= 5
	return Q6_R_popcount_P(hthreads_mask());
#elif ARCHV == 4
	return 3;
#endif
}

#endif

static inline void resched_int()
{
	asm(" swi(%0) // wake up & resched" : : "r"(RESCHED_INT_INTMASK));
}

#if (ARCHV <= 2)
static inline void highprio_imask(u32_t hthread)
{
	asm(" imask = %0 // set to high priority " : : "r"((-1)-(HW_TH_0_INTMASK << (hthread))));
}

static inline void lowprio_imask(u32_t hthread)
{
	asm(" imask = %0 // set to low priority " : : "r"(HW_TH_ALL_INTMASK ^ (HW_TH_0_INTMASK << (hthread))));
}
#elif (ARCHV >= 3)
static inline void highprio_imask(u32_t hthread)
{
	asm(" imask = %0 // set to high priority " : : "r"(-1));
}

static inline void lowprio_imask(u32_t hthread)
{
	asm(" imask = %0 // set to low priority " : : "r"(0));
}

#endif

static inline u32_t get_ssr()
{
	u32_t ret;
	asm volatile (" %0 = ssr // get SSR" : "=r"(ret));
	return ret;
}

static inline u32_t H2K_get_syscfg()
{
	u32_t ret;
	asm volatile (" %0 = syscfg // get syscfg" : "=r"(ret));
	return ret;
}

static inline void H2K_set_syscfg(u32_t val)
{
	asm volatile (" syscfg = %0 // set syscfg" : : "r"(val));
}

static inline u32_t H2K_get_livelock()
{
	u32_t ret;
	asm volatile (" %0 = s35 // get livelock" : "=r"(ret));
	return ret;
}

static inline void H2K_set_livelock(u32_t val)
{
	asm volatile (" s35 = %0 // set livelock" : : "r"(val));
}

static inline u32_t H2K_get_duck()
{
	u32_t ret;
	asm volatile (" %0 = s62 // get duck" : "=r"(ret));
	return ret;
}

static inline void H2K_set_duck(u32_t val)
{
	asm volatile (" s62 = %0 // set duck" : : "r"(val));
}

static inline u32_t H2K_get_chicken()
{
	u32_t ret;
	asm volatile (" %0 = s63 // get chicken" : "=r"(ret));
	return ret;
}

static inline void H2K_set_chicken(u32_t val)
{
	asm volatile (" s63 = %0 // set chicken" : : "r"(val));
}

static inline u32_t H2K_get_rgdr()
{
	u32_t ret;
	asm volatile (" %0 = s60 // get rgdr" : "=r"(ret));
	return ret;
}

static inline void H2K_set_rgdr(u32_t val)
{
	asm volatile (" s60 = %0 // set rgdr" : : "r"(val));
}

static inline void H2K_clear_gie()
{
	u32_t scratch;
	asm volatile ("%0 = SYSCFG; %0 = clrbit(%0,#4); SYSCFG=%0; isync;":"=r"(scratch));
}

static inline void H2K_set_gie()
{
	u32_t scratch;
	asm volatile ("%0 = SYSCFG; %0 = setbit(%0,#4); SYSCFG=%0; isync;":"=r"(scratch));
}

static inline void H2K_isync()
{
	asm volatile (" isync ");
}

static inline void H2K_ickill()
{
	asm volatile (" ickill ");
}

static inline void H2K_l2kill()
{
	asm volatile (" l2kill ");
}

#if ARCHV >= 60
static inline void H2K_l2gcleaninv()
{
	asm volatile (" l2gcleaninv ");
}
#endif

static inline u32_t H2K_get_modectl()
{
	u32_t ret;
	asm volatile (" %0 = modectl\n" :"=r"(ret));
	return ret;
}

static inline u32_t H2K_get_ipend()
{
	u32_t ret;
	asm volatile ("%0 = ipend;":"=r" (ret));
	return(ret);
}

static inline u32_t H2K_get_tid_reg()
{
	u32_t ret;
#if (ARCHV <= 3)
	asm("%0 = tid;":"=r" (ret));
#else
	asm("%0 = stid;":"=r" (ret));
#endif
	return(ret);
}

static inline void H2K_set_tid_reg(u8_t val)
{
#if (ARCHV <= 3)
	asm volatile ("tid = %0 ;"::"r" (val));
#else
	asm volatile ("stid = %0 ;"::"r" (val));
#endif
}

static inline u64_t H2K_get_pcycle_reg()
{
u64_t ret;
#if (ARCHV <= 3)
	asm volatile (
		"1: %H0 = pcyclehi \n"
		" %L0 = pcyclelo \n"
		" r15 = pcyclehi \n"
		" p0 = cmp.eq(r15,%H0) \n"
		" if (!p0) jump 1b \n" : "=r"(ret) : : "r15","p0");
#else
		asm volatile ( " %0 = s31:30 // READ PCYCLES \n" : "=r"(ret));
		//	}
#endif
	return ret;
}

static inline u64_t H2K_get_timer_reg() {
	u64_t ret;

	asm volatile ( " %0 = s57:56 // READ TIMER \n" : "=r"(ret));
	return ret;
}

static inline void H2K_clear_ipend(u32_t hthread_mask)
{
	asm("cswi(%0);" : : "r" (hthread_mask));
}

#if (ARCHV <= 3)
static inline u32_t get_hwtnum()
{
	return Q6_R_extractu_RII(get_ssr(),3,19);
}
#else
static inline u32_t get_hwtnum()
{
	u32_t ret;
	asm(" %0 = HTID // get hardware thread number " :"=r"(ret));
	return ret;
}
#endif

static inline void H2K_syncht()
{
	asm volatile (" syncht " : : : "memory");
}

#if (ARCHV >= 3)
static inline void H2K_mutex_lock_k0()
{
	asm volatile (" k0lock" : : : "memory");
}

static inline void H2K_mutex_unlock_k0()
{
	asm volatile (" k0unlock" : : : "memory");
}

static inline void H2K_mutex_lock_tlb()
{
	asm volatile (" tlblock" : : : "memory");
}

static inline void H2K_mutex_unlock_tlb()
{
	asm volatile (" tlbunlock" : : : "memory");
}

#if (ARCHV <= 3)
#define H2K_TLB_ATOMIC_START H2K_mutex_lock_tlb()
#define H2K_TLB_ATOMIC_END H2K_mutex_unlock_tlb()
#else
#define H2K_TLB_ATOMIC_START
#define H2K_TLB_ATOMIC_END
#endif

#if (ARCHV <= 3)
static inline void H2K_gregs_save(H2K_thread_context *me) { }
static inline void H2K_gregs_restore(H2K_thread_context *me) { }
#else
static inline void H2K_gregs_save(H2K_thread_context *me) { 
	u64_t g32,g10;
	asm volatile ( " %0 = g3:2 ; %1 = g1:0 ; " : "=r"(g32),"=r"(g10));
	me->gbadva_gosp = g32;
	me->gssr_gelr = g10;
}
static inline void H2K_gregs_restore(H2K_thread_context *me) { 
	u64_t g32,g10;
	g32 = me->gbadva_gosp;
	g10 = me->gssr_gelr;
	asm volatile ( " g3:2 = %0 ; g1:0 = %1; " : : "r"(g32),"r"(g10));
}
#endif

#define BKL_LOCK(...) H2K_mutex_lock_k0()
#define BKL_UNLOCK(...) H2K_mutex_unlock_k0()
#endif

#if (ARCHV >= 3)

#define MAKE_SETGET(REG) \
static inline u32_t H2K_get_##REG() \
{ u32_t ret; asm volatile (" %0 = " #REG : "=r"(ret)); return ret; } \
static inline void H2K_set_##REG(u32_t val) \
{ asm volatile ( #REG " = %0" : : "r"(val)); }

#else

#define MAKE_SETGET(REG) \
static inline u32_t H2K_get_##REG() \
{ return 0; } \
static inline void H2K_set_##REG(u32_t val) \
{ }

#endif

MAKE_SETGET(pmucfg)
MAKE_SETGET(pmuevtcfg)
MAKE_SETGET(pmucnt0)
MAKE_SETGET(pmucnt1)
MAKE_SETGET(pmucnt2)
MAKE_SETGET(pmucnt3)
MAKE_SETGET(elr)
MAKE_SETGET(gp)

#if (ARCHV <= 3)
static inline u32_t H2K_get_ccr() { return 0; }
static inline void H2K_set_ccr(u32_t val) { }
#else
MAKE_SETGET(ccr)
#endif

#undef MAKE_SETGET

static inline void H2K_dccleana(void *ptr)
{
	asm volatile (" dccleana(%0) " : :"r"(ptr) : "memory");
}

static inline void H2K_dccleaninva(void *ptr)
{
	asm volatile (" dccleaninva(%0) " : :"r"(ptr) : "memory");
}

static inline unsigned char H2K_l2locka(void *ptr)
{
	unsigned char out;
	asm volatile (" p0 = l2locka(%1) ; %0 = p0 " : "=r"(out) :"p"(ptr) : "memory","p0");
	return out;
}

static inline void H2K_l2unlock()
{
	asm volatile (" l2gunlock " : : : "memory");
}

#endif

