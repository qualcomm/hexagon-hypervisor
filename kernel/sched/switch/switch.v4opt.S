/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asm_offsets.h>
#include <asm_std.h>
#include <vmdefs.h>
#include <idtype.h>
#include <trace_asm_macro.h>
#include <trace_constants.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

#define KG_CPUSTART r6
#define HTNUM r7
#define KG_WAITCYCLES r8
#define KG_CPUWAIT r9
#define START r11:10
#define P_FROMWAIT p1
#define P_VMWORK p3
#define PMUVAL r12
#define PMU_ON r13
#define VMSTATUS r14
#define TIDVAL r15
#define DSTSRC r17:16
#define SRC r16
#define DST r17
#define NOW r19:18
#define NOW_LO r18

FUNC_START H2K_switch .text.core.switch .p2align 5
/* EJP: seems like trace is commented out... maybe need to add trace... */
	{
		HTNUM = HTID			// TNUM
		KG_CPUSTART = add(H2K_GP,#KG_oncpu_start)
		P_FROMWAIT = cmp.eq(r0,#0)
		DSTSRC = r1:0
	}
	{
		NOW = s31:30			// PCYCLE HI/LO
		START = memd(KG_CPUSTART+HTNUM<<#3)		// oncpu start
		KG_CPUWAIT = add(H2K_GP,#KG_oncpu_wait)
		if (P_FROMWAIT) jump .Lfrom_sleep
	}
	/* Save off rest of context */
	{
		r3:2 = sub(NOW,START)
		START = memd(SRC+#CONTEXT_totalcycles)
		memd(SRC+#CONTEXT_r2120) = r21:20
		r1:0 = c3:2			// lc1/sa1
	}
	{
		memd(SRC+#CONTEXT_r2322) = r23:22
		memd(SRC+#CONTEXT_lc1sa1) = r1:0
		r3:2 = add(r3:2,START)
		r1:0 = c13:12			// cs1:0
	}
	{
		memd(SRC+#CONTEXT_totalcycles) = r3:2
		memd(SRC+#CONTEXT_cs1cs0) = r1:0
		r1:0 = c7:6			// m1:0
	}
	{
		memd(SRC+#CONTEXT_r2524) = r25:24
		memd(SRC+#CONTEXT_m1m0) = r1:0
		r1:0 = c11:10			// gp:ugp
	}
	{
		memd(SRC+#CONTEXT_r2726) = r27:26
		memd(SRC+#CONTEXT_gpugp) = r1:0
		r0 = elr
	}
	{
		memw(SRC+#CONTEXT_elr) = r0
		r1:0 = g3:2
	}
	{
		memd(SRC+#CONTEXT_gbadva_gosp) = r1:0
		r1:0 = g1:0
	}
	{
		memd(SRC+#CONTEXT_gssr_gelr) = r1:0
#if ARCHV >= 60
		r1:0 = c17:16
#endif
	}
	{
		memd(SRC+#CONTEXT_framekey_framelimit) = r1:0
#if ARCHV >= 60
		r1:0 = c19:18
#endif
	}
	{
		memd(SRC+#CONTEXT_pktcount) = r1:0
#ifdef DO_EXT_SWITCH

// check if ext needs saving, and mark as saved
	 	r13 = add(SRC, ##CONTEXT_atomic_status_word)
	}
1:
	r14 = memw_locked(r13)
	{ p1 = tstbit(r14, #H2K_VMSTATUS_SAVEXT_BIT)  // ext regs live?
		r14 = clrbit(r14, #H2K_VMSTATUS_SAVEXT_BIT)
	}
	memw_locked(r13, p0) = r14
	if (!p0) jump 1b

	// cause exception on next ext access to restore
	{
		r15 = memw(SRC + #CONTEXT_ssr)
		r14 = memw(SRC + #CONTEXT_id)
		r20 = ##H2K_ID_CPUIDX_MASK
	}
	{
		r15 = clrbit(r15, #SSR_XE_BIT)
		r14 = and(r14, r20)
		r22 = ##EXT_CONTEXT_SIZE
	}
	{
	  memw(SRC + #CONTEXT_ssr) = r15
		r14 = lsr(r14, #H2K_ID_VINT_BITS)  // index
		r20 = memw(SRC + #CONTEXT_vmblock)
		r23 = ssr
	}
	{
		r21 = memw(r20 + #VMBLOCK_ext_contexts)  // start of ext contexts
		r23 = setbit(r23, #SSR_XE_BIT)
		if (!p1) jump .Lfrom_sleep  // no need to save
	}

// save; cpuid is ext_context index
	{
		r21 += mpyu(r22.l, r14.l)  // my ext context
		ssr = r23
	}

	{
		vmem(r21++#1) = v0
// predicates
	 	r20 = #0x00000000
	}
	{
		v0 = vsplatw(r20)
		vmem(r21++#1) = v1
	}
	{
		r20 = ##0x01010101
		vmem(r21++#1) = v2
	}
	{
		v1 = vsplatw(r20)
		vmem(r21++#1) = v3
	}
	{
		v3 = vmux(q0, v1, v0)
		vmem(r21++#1) = v4
	}
	{
		r20 = #1
		vmem(r21++#1) = v5
	}
	{
		v1 = vaslw(v1, r20) // 0x02020202
		vmem(r21++#1) = v6
	}
	{
		v2 = vmux(q1, v1, v0)
		vmem(r21++#1) = v7
	}
	{
		v3 = vor(v3, v2)
		vmem(r21++#1) = v8
	}
	{
		v1 = vaslw(v1, r20) // 0x04040404
		vmem(r21++#1) = v9
	}
	{
		v2 = vmux(q2, v1, v0)
		vmem(r21++#1) = v10
	}
	{
		v3 = vor(v3, v2)
		vmem(r21++#1) = v11
	}
	{
		v1 = vaslw(v1, r20) // 0x08080808
		vmem(r21++#1) = v12
	}
	{
		v2 = vmux(q3, v1, v0)
		vmem(r21++#1) = v13
	}
	{
		v3 = vor(v3, v2)
		vmem(r21++#1) = v14
	}
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

// save predicates
	vmem(r21+#0) = v3
#else
	} // end of packet at #ifdef DO_EXT_SWITCH
#endif

// fall through

#define OLDIDX r0
#define NEWIDX r1
#define IDXADDR r31
#define TRACE r2
#define DATA r3
#define ENTRIES r5
#define BUFADDR r4
#define ENTRIES_BUFADDR r5:4

	{
		TRACE = #H2K_TRACE_SWITCH
		jump 2f
	}

	.falign
.Lfrom_sleep:
	TRACE = #H2K_TRACE_SWITCH_FROM_SLEEP

2:
	{
		p0 = cmp.eq(DST,#0)
		if (p0.new) jump:nt .Lto_sleep
		if (!p0.new) VMSTATUS = memub(DST+#CONTEXT_vmstatus)
	}
	/* Load rest-of-context for new thread and trace */
	/* r0-r5 are available */
	/* r20-27 are available now, will be loaded later */

	{
		sgp0 = DST
		DATA = memw(DST+#CONTEXT_id)		// lets already fill this with switch id
	}
	{
		DATA = lsr(DATA, #2)  // assuming #define H2K_ID_VINT_BITS 10
		r21:20 = memd(DST+#CONTEXT_lc1sa1)
	}
	{
		DATA = insert(TRACE, #8, #0)
	}
	{
		P_VMWORK = tstbit(VMSTATUS,#H2K_VMSTATUS_VMWORK_BIT)
		KG_WAITCYCLES = add(H2K_GP,#KG_waitcycles)
		trace(DATA)
	}
#undef OLDIDX
#undef NEWIDX
#undef IDXADDR
#undef TRACE
#undef DATA
#undef ENTRIES
#undef BUFADDR
#undef ENTRIES_BUFADDR
	{
		r23:22 = memd(DST+#CONTEXT_cs1cs0)
		r25:24 = memd(DST+#CONTEXT_m1m0)
		c3:2 = r21:20				// lc1:sa1
	}
	{
		START = memd(KG_CPUWAIT+HTNUM<<#3)      // oncpu wait
		r3:2 = memd(KG_WAITCYCLES+HTNUM<<#3)    // waitcycles
		c13:12 = r23:22				// cs1cs0
	}
	{
		memd(KG_CPUSTART+HTNUM<<#3) = NOW
		TIDVAL = memw(DST+#CONTEXT_status_prio_hthread_tid)
		c7:6 = r25:24				// m1m0
	}
	{
		r21:20 = memd(DST+#CONTEXT_gpugp)
		r0 = memw(DST+#CONTEXT_elr)
		START = sub(NOW,START)			// pcycles - oncpu wait
	}
		STID = TIDVAL
	{
		r3:2 = add(r3:2,START)			// waitcycles + pcycles - oncpu wait
		r25:24 = memd(DST+#CONTEXT_gssr_gelr)
		r27:26 = memd(DST+#CONTEXT_gbadva_gosp)
		c11:10 = r21:20				// gp:ugp
	}

	elr = r0
	{
		r21:20 = memd(DST+#CONTEXT_ccrssr)	// fixme: will restore later?
		r23:22 = memd(DST+#CONTEXT_pktcount)
	}
	{
		s7:6 = r21:20
		r21:20 = memd(DST+#CONTEXT_framekey_framelimit)
	}
#if ARCHV >= 60
	c19:18 = r23:22
	c17:16 = r21:20
#endif

	{
		r21:20 = memd(DST+#CONTEXT_r2120)
		r23:22 = memd(DST+#CONTEXT_r2322)
		g1:0 = r25:24
	}
	{
		g3:2 = r27:26
		r25:24 = memd(DST+#CONTEXT_r2524)
		r27:26 = memd(DST+#CONTEXT_r2726)
	}
	{
		if (P_FROMWAIT) memd(KG_WAITCYCLES+HTNUM<<#3) = r3:2  // waitcycles
		memb(DST+#CONTEXT_hthread) = HTNUM
		r0 = DST
		if (P_VMWORK) call H2K_vm_do_work
	}
	{
		r31 = memw(DST+#CONTEXT_continuation)
		r1:0 = memd(DST+#CONTEXT_r0100)		// for return val from blocking traps
	}
	{
		r31 = and(r31,#-4)
		jump H2K_check_sanity_unlock
	}

#define TRACE r4
#define DATA r5
	.falign
.Lto_sleep:
	/* Don't care as much about register values here, certainly not r0-r5 and r20-27 */
	{
		// r1:0 = combine(HTNUM,NOW_LO)
		p0 = cmp.eq(SRC, #0)
		TRACE = #H2K_TRACE_SWITCH_TO_SLEEP
		DATA = #0
		if (p0.new) jump:nt 2f
	}
	{
		DATA = memw(SRC + #CONTEXT_id)  // ID we're switching *from*
	}

2:
	{
		DATA = lsr(DATA, #2)  // assuming #define H2K_ID_VINT_BITS 10
		memd(KG_CPUWAIT+HTNUM<<#3) = NOW       // oncpu wait
		r0 = lsl(#1,HTNUM)
	}
	{
		DATA = insert(TRACE, #8, #0)
		memw(H2K_GP+#KG_wait_mask) |= r0
		r3:2 = combine(#0,#-1)
	}
	trace(DATA)
#undef TRACE
#undef DATA
	STID = r2
	SGP0 = DST
	{
		memw(H2K_GP+#KG_priomask) |= r0
		r0 = ##0x00040000 // insert(r5,#3,#16)
	}
		k0unlock
		IMASK = r3
	{
		ssr = r0
	}
	{
		jump H2K_wait_forever
	}
FUNC_END H2K_switch

FUNC_START H2K_wait_forever .text.core.switch .p2align 2
	wait(r1)
	jump H2K_wait_forever
	nop				// used by Harness to overwrite with redirect
FUNC_END H2K_wait_forever

