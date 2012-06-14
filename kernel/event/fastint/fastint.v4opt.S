/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

#define CONTEXT_SIZE_VAL r10 
#define SSR_VAL r11
#define OLD_SP_SAVE r19
#define OTHER_SGP r18
#define TMP r14
#define INTINFO r5:4
#define INT_HANDLER r4
#define INT_ARG r5
#define TMP10 r11:10
#define TMP1 r11
#define TMP0 r10
#define TMP32 r7:6
#define TMP3 r7
#define TMP2 r6
#define TMP54 r9:8
#define TMP5 r9
#define TMP4 r8
#define INT_SAVE r30

FUNC_START H2K_fastint .text.core.fastint .p2align 5
	// r0 = intno
	// r1 = me, can be NULL!
	// r2 = hthread
	// r3 = argument (future)
	// H2K_GP = H2K_kg

	/* EJP: Not all of the context has been saved/restored 
	 * In particular, many control registers need to be saved
	 * here and restored below.
	 * We will assume, however, that the called function respects the ABI, so 
	 * registers that are fixed (UGP) and callee-save registers are assumed
	 * to be OK not to save
	 */
	{
		CONTEXT_SIZE_VAL = #FASTINT_CONTEXT_SIZE
		p3 = cmp.eq(r1,#0)
		SSR_VAL = ##0x01c80000
	}
	{
		TMP = r29
		r29 = add(##H2K_fastint_contexts+FASTINT_CONTEXT_SIZE,mpyi(r2,CONTEXT_SIZE_VAL))
		ssr = SSR_VAL  // AXOK (if changing bits other than IE, EX, UM)
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r19:18
		OTHER_SGP = add(r29,#-(FASTINT_CONTEXT_SIZE))
		TMP10 = c7:6				// m1:m0
	}
	{
		crswap(OTHER_SGP,sgp0)
		TMP = add(H2K_GP,#KG_inthandlers)
		OLD_SP_SAVE = TMP
	}
	{
		INTINFO = memd(TMP+r0<<#3)
		TMP = memw(H2K_GP+#KG_fastint_gp)
		if (p3) jump 1f
		TMP32 = c13:12				// cs1:cs0
	}
	{
		memd(OTHER_SGP+#CONTEXT_cs1cs0) = TMP32
		memd(OTHER_SGP+#CONTEXT_m1m0) = TMP10
		TMP54 = c11:10				// GP:UGP
	}
	{
		memd(OTHER_SGP+#CONTEXT_gpugp) = TMP54
		TMP32 = c3:2				// lc1/sa1
	}
	{
		memd(OTHER_SGP+#CONTEXT_lc1sa1) = TMP32
		TMP0 = elr
	}
	.falign
1:
	{
		if (!p3) memw(OTHER_SGP+#CONTEXT_elr) = TMP0
		gp = TMP
		INT_SAVE = r0			// NOTE: THIS IS R30 currently, don't dealloc!
		callr INT_ARG
	}
	.falign
H2K_fastint_return:
	{
		p0 = cmp.eq(r0,#0)		// if no ack, skip all
		if (p0.new) jump:nt .Lno_ack
		p1 = cmp.gtu(INT_SAVE,#31)		// if non-l2 interrupt, ciad
		if (!p1.new) jump:nt .L1ack
		INT_SAVE = add(INT_SAVE,#-32)
	}
	{
		TMP = memw(##H2K_kg+KG_l2_ack_base)
		TMP1 = extractu(INT_SAVE,#5,#5)
		TMP0 = and(INT_SAVE,#0x1f)
	}
	{
		TMP2 = lsl(#1,TMP0)
		memw(TMP+TMP1<<#2) = TMP2.new
		jump .Lno_ack
	}
.L1ack:
	TMP0 = and(INT_SAVE,#0x1f)
	TMP2 = lsl(#1,TMP0)
	ciad(TMP2)
	.falign
.Lno_ack:
	{
		p0 = cmp.eq(OTHER_SGP,#0)
		if (p0.new) jump:t .Lidle
		crswap(OTHER_SGP,sgp0)
		if (!p0.new) TMP32 = memd(OTHER_SGP+#CONTEXT_gpugp)
		TMP = OTHER_SGP
	}
	{
		TMP54 = memd(TMP+#CONTEXT_cs1cs0)
		TMP10 = memd(TMP+#CONTEXT_lc1sa1)
		c3:2 = TMP10
	}
	{
		TMP10 = memd(TMP+#CONTEXT_m1m0)
		TMP = memw(TMP+#CONTEXT_elr)
		c11:10 = TMP32
	}
	{
		elr = TMP
	}
	.falign
.Lidle:
	{
		r19:18 = memd(r29+#0)
		r31:30 = memd(r29+#8)		// like dallocframe
		r29 = OLD_SP_SAVE
		c13:12 = TMP54
	}
	{
		H2K_GP = ##H2K_kg
		c7:6 = TMP10
		jumpr r31
	}
FUNC_END H2K_fastint

