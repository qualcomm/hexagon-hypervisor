/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

/*
 * 31: user
 * 30: IE
 * 29: SS
 */

#define BAD_VA r0
#define CAUSE r1
#define VEC_OFFSET r2
#define ME r3

#define GEVB r4
#define NEWELR r5

#define GOSP r6
#define GBADVA r7
#define GBADVA_GOSP r7:6

#define GELR r8
#define GSSR r9
#define GSSR_GELR r9:8

#define VMSTATUS r10
#define ATOMIC_STATUS_ADDR r11
#define GSSR_HIGH r12
#define SSR r14
#define TMP r15

#define P_NOGEVB p0
//#define P_SS p1
//#define P_IE p2
#define P_GUEST p3

/* void H2K_vm_event(u32_t gbadva, u32_t cause, u32_t vec_offset, H2K_thread_context *me) */
FUNC_START H2K_vm_event .text.vm.event .p2align 5
	{
		GEVB = memw(ME+#CONTEXT_gevb)
		SSR = memw(ME+#CONTEXT_ssr)
		GELR = ELR							// CR RD
		ATOMIC_STATUS_ADDR = add(ME,#CONTEXT_atomic_status_word)
	}
	{
		P_NOGEVB = cmp.eq(GEVB,#0)
		if (P_NOGEVB.new) jump:nt .Lfatal				// NO CR/LLSC
		VMSTATUS = memub(ME+#CONTEXT_vmstatus)
		NEWELR = add(GEVB,VEC_OFFSET)
		GOSP = g2							// CR RD
	}
#undef P_NOGEVB
#define P_ATOMIC p0
	{
		//P_IE = tstbit(VMSTATUS,#H2K_VMSTATUS_IE_BIT)
		//P_SS = tstbit(SSR,#SSR_SS_BIT)
		P_GUEST = tstbit(SSR,#SSR_GUEST_BIT)
		GSSR_HIGH = extractu(SSR,#1,#SSR_SS_BIT)			//if (P_SS) GSSR_HIGH = add(GSSR_HIGH,#1)
		TMP = memw_locked(ATOMIC_STATUS_ADDR)				// LL AXOK
	}
	{
		TMP = clrbit(TMP,#H2K_VMSTATUS_IE_BIT)
		if (!P_GUEST) GSSR_HIGH = add(GSSR_HIGH,#4)
		ELR = NEWELR							// CR WR AXOK
	}
.Latomic_again:
	{
		memw_locked(ATOMIC_STATUS_ADDR,P_ATOMIC) = TMP			// SC AXOK
		SSR = clrbit(SSR,#SSR_SS_BIT)
		GSSR_HIGH = tableidxh(VMSTATUS,#1,#H2K_VMSTATUS_IE_BIT)		// if (P_IE) GSSR_HIGH = add(GSSR_HIGH,#2)
		GSSR = zxth(CAUSE)
	}
	{
		if (!P_ATOMIC) jump .Latomic_fail
		GSSR |= asl(GSSR_HIGH,#29)
		GBADVA = BAD_VA
	}
	{
		SSR = setbit(SSR,#SSR_GUEST_BIT)
		if (!P_GUEST) GOSP = memw(ME+#CONTEXT_r29)
		if (!P_GUEST) memw(ME+#CONTEXT_r29) = GOSP
		g1:0 = GSSR_GELR						// CR WR
	}
	{
		g3:2 = GBADVA_GOSP						// CR WR
		memw(ME+#CONTEXT_ssr) = SSR
		jumpr r31
	}
.Latomic_fail:
	TMP = memw_locked(ATOMIC_STATUS_ADDR)
	{
		TMP = clrbit(TMP,#H2K_VMSTATUS_IE_BIT)
		jump .Latomic_again
	}
.Lfatal:
	{
		r1:0 = combine(#-3,ME)
		r3:2 = #0
		r4 = HTID
		jump H2K_fatal_thread
	}
FUNC_END H2K_vm_event
