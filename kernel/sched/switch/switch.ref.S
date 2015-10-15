/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asm_offsets.h>
#include <asm_std.h>
#include <trace_asm_macro.h>
#include <trace_constants.h>
#include <vmdefs.h>
#include <idtype.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

FUNC_START H2K_switch .text.core.switch .p2align 5
	r17:16 = combine(r1,r0)	// save args for call
	// Get current pcycle count
	r19 = PCYCLEHI
	r18 = PCYCLELO
	r5 = PCYCLEHI
	p0 = cmp.eq(r19,r5)
	if (!p0) jump H2K_switch

#if (ARCHV) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r7:6 = combine(r5,r18)
	r15 = PMUCFG
	r0 = #3
	r3:2 = r19:18
	r10 = r5
	//call H2K_trace
	r1:0 = r17:16
	r8 = #KG_oncpu_start
	r8 = addasl(r8,r5,#3)
	r8 = add(H2K_GP,r8)

	r9 = #KG_oncpu_wait
	r9 = addasl(r9,r5,#3)
	r9 = add(H2K_GP,r9)

	r11 = #KG_waitcycles
	r11 = addasl(r11,r5,#3)
	r11 = add(H2K_GP,r11)

	p1 = cmp.eq(r0,#0)
	if (p1) jump .Lfrom_sleep

	r5:4 = memd(r8)
	r7:6 = sub(r19:18,r5:4)
	r5:4 = memd(r0+#CONTEXT_totalcycles)
	r5:4 = add(r7:6,r5:4)
	memd(r0+#CONTEXT_totalcycles) = r5:4

	memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r2322) = r23:22
	memd(r0+#CONTEXT_r2524) = r25:24
	memd(r0+#CONTEXT_r2726) = r27:26

	r14 = elr
	r25 = m1
	r24 = m0
	r23 = gp
	r22 = ugp
	r21 = lc1
	r20 = sa1

	memw(r0+#CONTEXT_elr) = r14
	memd(r0+#CONTEXT_gpugp) = r23:22
	memd(r0+#CONTEXT_lc1sa1) = r21:20
	memd(r0+#CONTEXT_m1m0) = r25:24

#if ARCHV >= 4
	r27:26 = g3:2
	r25:24 = g1:0
	r21 = cs1
	r20 = cs0
	r14 = ccr
	memd(r0+#CONTEXT_gbadva_gosp) = r27:26
	memd(r0+#CONTEXT_gssr_gelr) = r25:24
	memd(r0+#CONTEXT_cs1cs0) = r21:20
	memw(r0+#CONTEXT_ccr) = r14
#endif

#if ARCHV >= 60
	r21:20 = c19:18 // pktcount
	r25:24 = c17:16 // framekey / framelimit
	memd(r0+#CONTEXT_framekey_framelimit) = r25:24
	memd(r0+#CONTEXT_pktcount) = r21:20
#endif

#ifdef DO_EXT_SWITCH

	// check if ext needs saving, and mark as saved
	r13 = add(r0, ##CONTEXT_atomic_status_word)
1:
	r14 = memw_locked(r13)
	{ p1 = tstbit(r14, #H2K_VMSTATUS_SAVEXT_BIT)  // ext regs live?
		r14 = clrbit(r14, #H2K_VMSTATUS_SAVEXT_BIT)
	}
	memw_locked(r13, p0) = r14
	if (!p0) jump 1b

	// cause exception on next ext access to restore
	r14 = memw(r0 + #CONTEXT_ssr)
	r14 = clrbit(r14, #SSR_XE_BIT)
	memw(r0 + #CONTEXT_ssr) = r14

	if (!p1) jump .Lfrom_sleep  // no need to save

	// save; cpuid is ext_context index
	r14 = memw(r0 + #CONTEXT_id)
	r20 = ##H2K_ID_CPUIDX_MASK
	r14 = and(r14, r20)
	r14 = lsr(r14, #H2K_ID_VINT_BITS)  // index
	r22 = ##EXT_CONTEXT_SIZE
	r20 = memw(r0 + #CONTEXT_vmblock)
	r21 = memw(r20 + #VMBLOCK_ext_contexts)  // start of ext contexts
	r21 += mpyu(r22.l, r14.l)  // my ext context

	r23 = ssr
	r23 = setbit(r23, #SSR_XE_BIT)
	ssr = r23

	vmem(r21++#1) = v0
	vmem(r21++#1) = v1
	vmem(r21++#1) = v2
	vmem(r21++#1) = v3
	vmem(r21++#1) = v4
	vmem(r21++#1) = v5
	vmem(r21++#1) = v6
	vmem(r21++#1) = v7
	vmem(r21++#1) = v8
	vmem(r21++#1) = v9
	vmem(r21++#1) = v10
	vmem(r21++#1) = v11
	vmem(r21++#1) = v12
	vmem(r21++#1) = v13
	vmem(r21++#1) = v14
	vmem(r21++#1) = v15
	vmem(r21++#1) = v16
	vmem(r21++#1) = v17
	vmem(r21++#1) = v18
	vmem(r21++#1) = v19
	vmem(r21++#1) = v20
	vmem(r21++#1) = v21
	vmem(r21++#1) = v22
	vmem(r21++#1) = v23
	vmem(r21++#1) = v24
	vmem(r21++#1) = v25
	vmem(r21++#1) = v26
	vmem(r21++#1) = v27
	vmem(r21++#1) = v28
	vmem(r21++#1) = v29
	vmem(r21++#1) = v30
	vmem(r21++#1) = v31

	// predicates
	r20 = #0x00000000
	v0 = vsplatw(r20)

	r20 = ##0x01010101
	v1 = vsplatw(r20)
	v3 = vmux(q0, v1, v0)

	r20 = #1
	v1 = vaslw(v1, r20) // 0x02020202
	v2 = vmux(q1, v1, v0)
	v3 = vor(v3, v2)

	v1 = vaslw(v1, r20) // 0x04040404
	v2 = vmux(q2, v1, v0)
	v3 = vor(v3, v2)

	v1 = vaslw(v1, r20) // 0x08080808
	v2 = vmux(q3, v1, v0)
	v3 = vor(v3, v2)

	vmem(r21+#0) = v3
#endif

.Lfrom_sleep:
	if (r1 == #0) jump:nt .Lto_sleep
	r27:26 = combine(r10,r18)
	r20 = #H2K_TRACE_SWITCH
	r14 = memw(r1+#CONTEXT_id)
	r14 = insert(r20,#8,#0)
	ASM_INLINE_TRACE(r14,r27:26,r27,r26,r25:24,r25,r24,p0,r21,r20,r31,,)
	memd(r8) = r19:18

#if ARCHV >= 4
	r15:14 = memd(r1+#CONTEXT_ccrssr)
	r21:20 = memd(r1+#CONTEXT_cs1cs0)
	r25:24 = memd(r1+#CONTEXT_gssr_gelr)
	r27:26 = memd(r1+#CONTEXT_gbadva_gosp)
	ccr = r15
	ssr = r14
	cs0 = r20
	cs1 = r21
	g1:0 = r25:24
	g3:2 = r27:26
#endif

#if ARCHV >= 60
	r21:20 = memd(r1+#CONTEXT_pktcount)
	r25:24 = memd(r1+#CONTEXT_framekey_framelimit)
	c19:18 = r21:20
	c17:16 = r25:24
#endif

	r14 = memw(r1+#CONTEXT_elr)
	r23:22 = memd(r1+#CONTEXT_gpugp)
	r21:20 = memd(r1+#CONTEXT_lc1sa1)
	r25:24 = memd(r1+#CONTEXT_m1m0)

	elr = r14
	m1 = r25
	m0 = r24
	gp = r23
	ugp = r22
	lc1 = r21
	sa1 = r20

	r21:20 = memd(r1+#CONTEXT_r2120)
	r23:22 = memd(r1+#CONTEXT_r2322)
	r25:24 = memd(r1+#CONTEXT_r2524)
	r27:26 = memd(r1+#CONTEXT_r2726)

	r5:4 = memd(r9)
	r7:6 = sub(r19:18,r5:4)
	r5:4 = memd(r11)
	r5:4 = add(r7:6,r5:4)
	if (p1) memd(r11) = r5:4

	memb(r1+#CONTEXT_hthread) = r10
	r14 = memb(r1+#CONTEXT_pmu_on)
	r11 = #1
	r10 = r5
	r15 = insert(r14,r11:10)
	PMUCFG = r15
	sgp0 = r1
	r4 = memw(r1+#CONTEXT_status_prio_hthread_tid)
#if (ARCHV <= 3)
	tid = r4
#else
	stid = r4
#endif

	r17 = r1
	r0 = r17
	call H2K_vm_do_work
	r1:0 = memd(r17+#CONTEXT_r0100)
	r31 = memw(r17+#CONTEXT_continuation)
	jump H2K_check_sanity_unlock

.Lto_sleep:
	memd(r9) = r19:18
#if (ARCHV) <= 3
	r5 = ssr
	r5 = extractu(r5,#3,#19)
#else
	r5 = HTID
#endif
	r27:26 = combine(r5,r18)
	r14 = #H2K_TRACE_SWITCH
	ASM_INLINE_TRACE(r14,r27:26,r27,r26,r25:24,r25,r24,p0,r21,r20,r31,,)

	r15 = clrbit(r15,r5)
	PMUCFG = r15
	r9 = memw(H2K_GP+#KG_wait_mask)
	r8 = memw(H2K_GP+#KG_priomask)
	r9 = setbit(r9,r5)
	r8 = setbit(r8,r5)
	memw(H2K_GP+#KG_wait_mask) = r9
	memw(H2K_GP+#KG_priomask) = r8
	k0unlock
	// set SGP to NULL
	sgp0 = r1

	// set IMASK to receive most interrupts
	r0 = imask
	r1.h = #HI(0)
	r1.l = #LO(0)
	r0 = and(r0,r1)
	imask = r0

	// set TID to special value (-1)
	r1 = #-1
#if (ARCHV <= 3)
	tid = r1
#else
	stid = r1
#endif

	// set SSR to enable interrupts
	// NOTE: should be the last thing before going to WAIT mode, we may get an interrupt
	// before the actual wait instruction.
	r2 = ssr
	r3 = #4
	r2 = insert(r3,#3,#16)
	ssr = r2
	jump H2K_wait_forever
FUNC_END H2K_switch

FUNC_START H2K_wait_forever .text.core.switch .p2align 2
	wait(r1)
	jump H2K_wait_forever
	nop				// used by Harness to overwrite with redirect
FUNC_END H2K_wait_forever

