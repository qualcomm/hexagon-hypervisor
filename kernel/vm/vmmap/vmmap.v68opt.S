/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <h2_common_asid.h>
#include <max.h>

#define ASID_MASK ((1 << ASID_BITS) - 1)

/* void H2K_vmtrap_clrmap(H2K_thread_context *me) */
#define ME_ARG r0		/* arg0: me (until copied into ME) */
#define COUNT_VA r19:18		/* me->r0100: r01 = count, r00 = va */
#define COUNT r19		/* me->r01 */
#define VA r18			/* me->r00 */
#define SSR_ASID_BYTE r2	/* byte 1 of me->ssr, low ASID_BITS = me->ssr_asid */
FUNC_START H2K_vmtrap_clrmap .text.vm.funcs .p2align 2
	{
		COUNT_VA = memd(ME_ARG+#CONTEXT_r0100)
		memd(r29+#-0x18) = r19:18		// save callee-saved r19:18
	}
	/* me->r00 = 0; count == 0: return */
	{
		SSR_ASID_BYTE = memub(ME_ARG+#CONTEXT_ssr+1)
		memw(ME_ARG+#CONTEXT_r00) = #0x0
		p0 = cmp.eq(COUNT,#0x0)
		if (p0.new) jump:nt .Lret
	}
#define ME r16			/* me, kept across the call */
#define ASID r17		/* me->ssr_asid, kept across the call */
	/* H2K_mem_stlb_invalidate_va(va, count, me->ssr_asid, me) */
	{
		memd(r29+#-0x10) = r17:16		// save callee-saved r17:16
		ME = ME_ARG
		r1:0 = combine(COUNT,VA)
		r2 = and(SSR_ASID_BYTE,#ASID_MASK)
	}
#undef ME_ARG
#undef SSR_ASID_BYTE
	{
		allocframe(#0x10)
		call H2K_mem_stlb_invalidate_va
		r3 = ME
		ASID = r2
	}
	/* tail: H2K_mem_tlb_invalidate_va(va, count, me->ssr_asid, me) */
	{
		r1:0 = COUNT_VA
		r3 = ME
		r2 = ASID
		r17:16 = memd(r29+#0x8)			// restore r17:16
	}
	{
		r19:18 = memd(r29+#0x0)			// restore r19:18
		deallocframe
		jump H2K_mem_tlb_invalidate_va
	}
.Lret:
	{
		r19:18 = memd(r29+#-0x18)		// restore r19:18
		jumpr r31
	}
FUNC_END H2K_vmtrap_clrmap
#undef COUNT_VA
#undef COUNT
#undef VA
#undef ME
#undef ASID

/* void H2K_vmtrap_newmap(H2K_thread_context *me) */
#define ME_ARG r0		/* arg0: me (until r0 becomes newptb) */
#define FLAG_EXTRA r3:2		/* me->r0302 */
#define FLAG r2			/* me->r02: tlb_invalidate_flag, arg2 of H2K_asid_table_inc */
#define EXTRA r3		/* me->r03, arg3 of H2K_asid_table_inc */
#define TYPE r1			/* me->r01: translation_type, arg1 of H2K_asid_table_inc */
#define VMBLOCK r4		/* me->vmblock, arg4 of H2K_asid_table_inc */
#define NEWPTB r0		/* me->r00, arg0 of H2K_asid_table_inc */
FUNC_START H2K_vmtrap_newmap .text.vm.funcs .p2align 2
	{
		FLAG_EXTRA = memd(ME_ARG+#CONTEXT_r0302)
		TYPE = memub(ME_ARG+#CONTEXT_r01)
	}
	/* bad flag: me->r00 = -1; return */
	{
		p0 = cmp.gtu(FLAG,#H2K_ASID_TLB_INVALIDATE_XXX_LAST-1)
		if (p0.new) jump:nt .Ldealloc_ret_fail
		allocframe(#0x8)
		if (!p0.new) VMBLOCK = memw(ME_ARG+#CONTEXT_vmblock)
		FLAG = and(FLAG,#255)
	}
#define ME r16			/* me, kept across the calls */
	/* newasid = H2K_asid_table_inc(newptb, type, flag, extra, me->vmblock) */
	{
		ME = ME_ARG
		NEWPTB = memw(ME_ARG+#CONTEXT_r00)
		memd(r29+#0x0) = r17:16			// save callee-saved r17:16
		call H2K_asid_table_inc
	}
#undef ME_ARG
#undef FLAG_EXTRA
#undef FLAG
#undef EXTRA
#undef TYPE
#undef VMBLOCK
#undef NEWPTB
#define NEWASID r0		/* return value of H2K_asid_table_inc */
#define SSR_ASID_BYTE r2	/* byte 1 of me->ssr, low ASID_BITS = me->ssr_asid */
	/* newasid == -1: me->r00 = -1; return */
	{
		p0 = cmp.eq(NEWASID,#-0x1)
		if (p0.new) memw(ME+#CONTEXT_r00) = #-0x1
		if (p0.new) jump:nt .Ldealloc_ret
		if (!p0.new) SSR_ASID_BYTE = memub(ME+#CONTEXT_ssr+1)
	}
#define NEWASID_SAVED r17	/* newasid, kept across the call */
	/* H2K_asid_table_dec(me->ssr_asid) */
	{
		call H2K_asid_table_dec
		r0 = and(SSR_ASID_BYTE,#ASID_MASK)
		NEWASID_SAVED = NEWASID
	}
#undef NEWASID
	/* me->r00 = 0; me->ssr_asid = (u8_t)newasid */
	{
		memw(ME+#CONTEXT_r00) = #0x0
		SSR_ASID_BYTE = memub(ME+#CONTEXT_ssr+1)
	}
	{
		SSR_ASID_BYTE = insert(NEWASID_SAVED,#0x7,#0x0)
		memb(ME+#CONTEXT_ssr+1) = SSR_ASID_BYTE.new
	}
#undef SSR_ASID_BYTE
#undef NEWASID_SAVED
#undef ME
.Ldealloc_ret:
	{
		r17:16 = memd(r29+#0x0)			// restore r17:16
		dealloc_return
	}
#define ME_ARG r0		/* arg0: me */
.Ldealloc_ret_fail:
	{
		memw(ME_ARG+#CONTEXT_r00) = #-0x1
	}
	{
		dealloc_return
	}
#undef ME_ARG
FUNC_END H2K_vmtrap_newmap
