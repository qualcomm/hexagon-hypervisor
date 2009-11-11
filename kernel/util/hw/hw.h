/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _HEADER_HW_H
#define _HEADER_HW_H 1

static inline void ciad(unsigned int mask)
{
	asm (" ciad(%0) // clear IAD " : : "r"(mask));
}

#if (ARCHV <= 2)
static inline void int_hthread(int hthread)
{
	asm(" swi(%0) // wake up a hw thread " : : "r"(HW_TH_0_INTMASK<<hthread));
}

static inline void int_hthread_mask(int mask)
{
	asm(" swi(%0) // wake up a hw thread " : : "r"(mask<<HW_TH_0_LEFTSHIFT));
}
#elif (ARCHV >= 3)
static inline void change_imask(int thread, unsigned int imask)
{
	asm(" p0 = %0\n setimask(p0,%1)" : : "r"(thread),"r"(imask):"p0");
}

#endif

static inline void resched_int()
{
	asm(" swi(%0) // wake up & resched" : : "r"(RESCHED_INT_INTMASK));
}

static inline void highprio_imask(int hthread)
{
	asm(" imask = %0 // set to high priority " : : "r"((-1)-(HW_TH_0_INTMASK << (hthread))));
}

static inline void lowprio_imask(int hthread)
{
	asm(" imask = %0 // set to low priority " : : "r"(HW_TH_ALL_INTMASK ^ (HW_TH_0_INTMASK << (hthread))));
}

static inline unsigned int get_ssr()
{
	unsigned int ret;
	asm(" %0 = ssr // get SSR" : "=r"(ret));
	return ret;
}

static inline unsigned int get_hwtnum()
{
	return Q6_R_extractu_RII(get_ssr(),3,19);
}

#if (ARCHV >= 3)
static inline void BLASTK_mutex_lock_k0()
{
	asm(" k0lock");
}

static inline void BLASTK_mutex_unlock_k0()
{
	asm(" k0unlock");
}
#endif

#endif

