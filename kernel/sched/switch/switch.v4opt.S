/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

/*
 * We are switching from thread in r0 to thread in r1
 * We don't care about any registers
 * Handle any tracing and profiling
 * If switching to NULL, go to sleep
 * Otherwise, load continuation into LR and jump to check_sanity_unlock
 */

#define NOW r19:18
#define HTNUM r7
#define KG_CPUSTART r6
#define KG_CPUWAIT r9
#define KG_WAITCYCLES r8
#define START r11:10
#define P_FROMWAIT p1
#define P_VMWORK p3
#define SRC r16
#define DST r17
#define DSTSRC r17:16
#define PMUVAL r12
#define PMU_ON r13
#define VMSTATUS r14
#define TIDVAL r15

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
	}
	.falign
.Lfrom_sleep:
	{
		PMUVAL = PMUCFG
		p0 = cmp.eq(DST,#0)
		if (p0.new) jump:nt .Lto_sleep
		if (!p0.new) PMU_ON = memub(DST+#CONTEXT_pmu_on)
		if (!p0.new) VMSTATUS = memub(DST+#CONTEXT_vmstatus)
	}
	/* Load rest-of-context for new thread */
	SGP0 = DST
	{
		P_VMWORK = tstbit(VMSTATUS,#H2K_VMSTATUS_VMWORK_BIT)
		KG_WAITCYCLES = add(H2K_GP,#KG_waitcycles)
		r21:20 = memd(DST+#CONTEXT_lc1sa1)
	}
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
		r1:0 = combine(#1,HTNUM)
		c7:6 = r25:24				// m1m0
	}
	{
		r21:20 = memd(DST+#CONTEXT_gpugp)
		r0 = memw(DST+#CONTEXT_elr)
		PMUVAL = insert(PMU_ON,r1:0)
		START = sub(NOW,START)			// pcycles - oncpu wait
	}
		STID = TIDVAL
	{
		r3:2 = add(r3:2,START)			// waitcycles + pcycles - oncpu wait
		r25:24 = memd(DST+#CONTEXT_gssr_gelr)
		r27:26 = memd(DST+#CONTEXT_gbadva_gosp)
		c11:10 = r21:20				// gp:ugp
	}
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
	elr = r0					// XXX: FIXME: cram somewhere...
	PMUCFG = PMUVAL
	{
		if (P_FROMWAIT) memd(KG_WAITCYCLES+HTNUM<<#3) = r3:2  // waitcycles
		memb(DST+#CONTEXT_hthread) = HTNUM
		r0 = DST
		if (P_VMWORK) call H2K_vm_do_work
	}
	{
		r31 = memw(DST+#CONTEXT_continuation)
		r1:0 = memd(DST+#CONTEXT_r0100)
		jump H2K_check_sanity_unlock
	}
	.falign
.Lto_sleep:
	{
		memd(KG_CPUWAIT+HTNUM<<#3) = NOW       // oncpu wait
		r0 = lsl(#1,HTNUM)
		PMUVAL = PMUCFG
	}
	{
		memw(H2K_GP+#KG_wait_mask) |= r0
		r3:2 = combine(#0,#-1)
	}
	SGP0 = DST
	STID = r2
	{
		memw(H2K_GP+#KG_priomask) |= r0
		PMUVAL = clrbit(PMUVAL,HTNUM)
	}
	{
		r0 = ##0x00040000 // insert(r5,#3,#16)
		PMUCFG = PMUVAL
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

