/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _HEADER_HW_H
#define _HEADER_HW_H 1

#include <max.h>
#include <q6protos.h>

static inline void ciad(u32_t mask)
{
	asm (" ciad(%0) // clear IAD " : : "r"(mask));
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
	asm(" p0 = %1;\n %0 = getimask(p0)" : "=r"(imask) : "r"(thread):"p0");
	return imask;
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
	asm(" %0 = ssr // get SSR" : "=r"(ret));
	return ret;
}

static inline u32_t H2K_get_syscfg()
{
	u32_t ret;
	asm(" %0 = syscfg // get syscfg" : "=r"(ret));
	return ret;
}

static inline void H2K_clear_gie()
{
	asm("r0 = SYSCFG; r0 = clrbit(r0,#4); SYSCFG=r0; isync;":::"r0");
}

static inline void H2K_set_gie()
{
	asm("r0 = SYSCFG; r0 = setbit(r0,#4); SYSCFG=r0; isync;":::"r0");
}

static inline u32_t H2K_get_ipend()
{
	u32_t ret;
	asm("%0 = ipend;":"=r" (ret));
	return(ret);
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
	asm(" %0 = tnum // get tnum" :"=r"(ret));
	return ret;
}
#endif

#if (ARCHV >= 3)
static inline void H2K_mutex_lock_k0()
{
	asm(" k0lock");
}

static inline void H2K_mutex_unlock_k0()
{
	asm(" k0unlock");
}
#define BKL_LOCK(...) H2K_mutex_lock_k0()
#define BKL_UNLOCK(...) H2K_mutex_unlock_k0()
#endif

#endif

