/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>
#include <h2_common_timer_defs.h>

#define TIMERHW_TICK_REALFREQ 19200000
#define TIMERHW_NSEC_PER_TICK 52
#define TIMERHW_NSEC_FREQ (TIMERHW_TICK_REALFREQ * TIMERHW_NSEC_PER_TICK)
#define TICK_GRANULARITY 4

#define HW_COUNT_LO 0x1000
#define HW_COUNT_HI 0x1004
#define HW_MATCH_LO 0x1020
#define HW_MATCH_HI 0x1024
#define HW_ENABLE   0x102c
#define HW_CNTFRQ   0x0000
#define HW_CNTSR    0x0004
#define HW_CNTACR   0x0040

#ifndef NO_DEVICES

// Does this really need to be in assembly? Could move to .all.c
FUNC_START H2K_timer_hw_init  .text.time.timer  .falign
#define DEVPTR r1
#define DEVSLICE r0
#define ZEROPAIR r5:4
#define ALLFS r7:6
#define MASK r3
#define L2ACK r2
#define TIMER_INTNUM r8
#define L2OFF r9

	TIMER_INTNUM = memw(H2K_GP+#KG_timer_intnum)
	{
		ZEROPAIR = #0
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
		L2ACK = memw(H2K_GP+#KG_l2_ack_base)
		TIMER_INTNUM = add(TIMER_INTNUM, #-32)
	}
	{
		memd(H2K_GP+#KG_time_last_ticks) = ZEROPAIR
		memd(H2K_GP+#KG_time_last_pcycles) = ZEROPAIR
		DEVSLICE = add(DEVPTR,#HW_COUNT_LO)
	}
	{
		memw(DEVSLICE+#HW_MATCH_HI-HW_COUNT_LO) = #-1
		memw(DEVSLICE+#HW_MATCH_LO-HW_COUNT_LO) = #-1
		ALLFS = #-1
		MASK = and(TIMER_INTNUM, #31)  // TIMER_INTNUM % 32
	}
	{
		memw(DEVPTR+#HW_CNTSR) = #-1
		memw(DEVPTR+#HW_CNTACR) = #-1
		MASK = lsl(#1, MASK)
		L2OFF = lsr(TIMER_INTNUM, #5)  // TIMER_INTNUM / 32
	}
	{
		memw(DEVPTR+#HW_CNTFRQ) = ##TIMERHW_TICK_REALFREQ
		memw(DEVSLICE+#HW_ENABLE-HW_COUNT_LO) = #1
		L2OFF = asl(L2OFF, #2)  // L2OFF *= 4
	}
	{
		memw(L2ACK + L2OFF << #0) = MASK
		memd(H2K_GP+#KG_time_next_ticks) = ALLFS
		jumpr r31
	}
#undef L2ACK
#undef MASK
#undef ALLFS
#undef ZEROPAIR
#undef DEVPTR
#undef DEVSLICE
#undef TIMER_INTNUM
#undef L2OFF
FUNC_END H2K_timer_hw_init

#endif

FUNC_START H2K_timer_get_ticks  .text.time.timer  .falign
#define TICKHI r1
#define TICKLO r0
#define TICKS r1:0
#define TICKHI2 r2
#define PCYCLES r5:4
#define DEVPTR r3
#define LASTTICKS r7:6
#define CREV r8
#define V61 r9
	{
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
	}
	{
		PCYCLES = s31:30
		LASTTICKS = memd(H2K_GP+#KG_time_last_ticks)
		DEVPTR = add(DEVPTR,#HW_COUNT_LO)
	}
	{
		TICKHI = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
		TICKLO = memw(DEVPTR+#HW_COUNT_LO-HW_COUNT_LO)
	}
	{
		p1 = cmp.gtu(TICKS,LASTTICKS)		// nowticks > last_ticks?
		TICKHI2 = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
	}
	{
		p0 = cmp.eq(TICKHI2,TICKHI)
		if (!p0.new) jump:nt H2K_timer_get_ticks
	}
	{
		if (p1) memd(H2K_GP+#KG_time_last_ticks) = TICKS
		if (p1) memd(H2K_GP+#KG_time_last_pcycles) = PCYCLES
		jumpr r31
	}
#undef TICKHI
#undef TICKLO
#undef TICKS 
#undef TICHI2
#undef PCYCLES
#undef DEVPTR
#undef LASTTICKS
#undef CREV
#undef V61
FUNC_END H2K_timer_get_ticks

FUNC_START H2K_timer_init  .text.time.timer  .falign	// init(devpage_offset)

#ifndef NO_DEVICES
	{
		allocframe(#0)
		r5:4 = #0
		r1 = memw(H2K_GP+#KG_time_devptr)
	}
	{
		p0 = cmp.eq(r1,#0)
		if (p0.new) r1 = add(r0,##Q6_SS_BASE_VA+TIMER_OFFSET)
		memd(H2K_GP+#KG_time_timeouts) = r5:4
	}
	{
		memw(H2K_GP+#KG_time_devptr) = r1
		call H2K_timer_hw_init
	}
	{
		deallocframe
		jump H2K_timer_get_ticks
	}
#else
	jumpr r31
#endif

FUNC_END H2K_timer_init

/* H2K_timer_dotimeout(H2K_treenode_t *treenode, void *me) */
FUNC_START H2K_timer_dotimeout .text.time.timer
	{
		r1 = add(r0,#-CONTEXT_tree)
		r3:2 = #0
	}
	{
		memd(r1+#CONTEXT_timeout) = r3:2
		r0 = memw(r1+#CONTEXT_vmblock)
		r2 = #H2K_TIME_GUESTINT
	}
	{
		r3 = memw(r0+#VMBLOCK_intinfo)
		jump H2K_vm_cpuint_post_locked
	}
FUNC_END H2K_timer_dotimeout

/* H2K_timer_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum) */
FUNC_START H2K_timer_int .text.time.timer
#define ME r16
#define DEVPTR r17
#define DEVPTR_ME r17:16
#define TIMER_INTNUM r18

	{
		allocframe(#16)
		memd(r29+#-16) = r17:16
		ME = r1
		call H2K_timer_get_ticks
	}
#define TICKS r5:4 // must be in r5:4 for calling convention
	{
		TICKS = #TICK_GRANULARITY
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
		memw(r29+#0) = #0
	}
	{
		TICKS = add(TICKS,r1:0)
		r0 = add(r29,#0)
		r1 = add(H2K_GP,#KG_time_timeouts)
		DEVPTR = add(DEVPTR,#HW_COUNT_LO)
	}
	{
		k0lock
	}
	{
		TIMER_INTNUM = memw(H2K_GP+#KG_timer_intnum)
		r2 = memw(r1)
		// r5:4 = TICKS
		call H2K_tree_bisect	// bisect(&timedout,&GP->time.timeouts,gp->time.timeouts,ticks)
	}
#undef TICKS
	.falign
#define NEXTTICKS r7:6
#define NEXTTICKS_HI r7
#define NEXTTICKS_LO r6
#define ROOT r5
#define OLDROOT r4
	{
		TIMER_INTNUM = add(TIMER_INTNUM, #-32)
		NEXTTICKS = #-1
		ROOT = memw(H2K_GP+#KG_time_timeouts)
		if (cmp.eq(ROOT.new,#0)) jump:t .Ltree_min_loop_done
	}
	.falign
.Ltree_min_loop:
	{
		OLDROOT = ROOT
		ROOT = memw(ROOT+#0)
		if (!cmp.eq(ROOT.new,#0)) jump:t .Ltree_min_loop
	}
	ROOT = OLDROOT
.Ltree_min_loop_done:
	{
		p0 = cmp.eq(ROOT,#0)
		if (!p0.new) NEXTTICKS = memd(ROOT+#8)
		memw(DEVPTR+#HW_MATCH_HI-HW_COUNT_LO) = #-1
	}
	{
		memw(DEVPTR+#HW_MATCH_LO-HW_COUNT_LO) = NEXTTICKS_LO
		r2 = ##H2K_timer_dotimeout
		memd(H2K_GP+#KG_time_next_ticks) = NEXTTICKS
	}
	{
		memw(DEVPTR+#HW_MATCH_HI-HW_COUNT_LO) = NEXTTICKS_HI
		r0 = memw(r29)
		r1 = ME
		call H2K_tree_destructive_iterate // ..iterate(timedout,me,H2K_timer_dotimeout)
	}
#undef NEXTTICKS
#undef NEXTTICKS_HI
#undef NEXTTICKS_LO
#undef ROOT
#undef LEFTPTR
	{
		k0unlock
	}

#define VICBASE r7
#define L2VICWORD r6
#define MASK r5
	{
		VICBASE = memw(H2K_GP+#KG_l2_ack_base)
		L2VICWORD = lsr(TIMER_INTNUM, #5)
		r17:16 = memd(r29+#8)
		MASK = and(TIMER_INTNUM, #0x1f)
	}
	{
		MASK = lsl(#1, MASK)
		memw(VICBASE+L2VICWORD<<#2) = MASK.new
		deallocframe
	}
	{
		jumpr r31
	}
#undef VICBASE
#undef L2VICWORD
#undef MASK
#undef ME
#undef DEVPTR
#undef DEVPTR_ME
#undef TIMER_INTNUM
FUNC_END H2K_timer_int

FUNC_START H2K_timer_get_freq .text.time.timer 
	{
		r1:0 = combine(#0,##TIMERHW_NSEC_FREQ)
		jumpr r31
	}
FUNC_END H2K_timer_get_freq

FUNC_START H2K_timer_get_resolution .text.time.timer
	{
		r1:0 = combine(#0,#TIMERHW_NSEC_PER_TICK)
		jumpr r31
	}
FUNC_END H2K_timer_get_resolution

FUNC_START H2K_timer_get_time .text.time.timer
#define NSECS r1:0
#define NSECS_HI r1
#define NSECS_LO r0
#define TICKS r3:2
#define TICKS_LO r2
#define TICKS_HI r3
#define TICKS_HI2 r4
#define DEVPTR r5
#define PCYCLES r7:6
#define NSEC_PER_TICK r8
#define CREV r9
#define LAST_TICKS r11:10
#define V61 r12
	{
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
		NSEC_PER_TICK = #TIMERHW_NSEC_PER_TICK
	}
	{
		PCYCLES = s31:30
		LAST_TICKS = memd(H2K_GP+#KG_time_last_ticks)
		DEVPTR = add(DEVPTR,#HW_COUNT_LO)
	}
	{
		TICKS_HI = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
		TICKS_LO = memw(DEVPTR+#HW_COUNT_LO-HW_COUNT_LO)
	}
	{
		p1 = cmp.gtu(TICKS,LAST_TICKS)		// nowticks > last_ticks?
	}
	{
		NSECS = mpyu(TICKS_LO,NSEC_PER_TICK)
		TICKS_HI2 = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
	}
	{
		p2 = cmp.eq(TICKS_HI2,TICKS_HI)
		if (!p2.new) jump:nt H2K_timer_get_time
	}
	{
		NSECS_HI += mpyi(TICKS_HI,NSEC_PER_TICK)
		if (p1) memd(H2K_GP+#KG_time_last_ticks) = TICKS
		if (p1) memd(H2K_GP+#KG_time_last_pcycles) = PCYCLES
		jumpr r31
	}
#undef NSEC_PER_TICK
#undef PCYCLES
#undef DEVPTR
#undef TICKS_HI2
#undef TICKS_HI
#undef TICKS_LO
#undef TICKS
#undef NSECS_LO
#undef NSECS_HI
#undef NSECS
#undef LAST_TICKS
#undef CREV
#undef V61
FUNC_END H2K_timer_get_time 

FUNC_START H2K_timer_get_timeout .text.time.timer
#define ME r4
#define TICKS r3:2
#define TICKS_LO r2
#define TICKS_HI r3
#define NSEC_PER_TICK r5
	{
		TICKS = memd(ME+#CONTEXT_timeout)
		NSEC_PER_TICK = #TIMERHW_NSEC_PER_TICK
	}
#undef ME
#define NSECS r1:0
#define NSECS_HI r1
	{
		NSECS = mpyu(TICKS_LO,NSEC_PER_TICK)
	}
	{
		NSECS_HI += mpyi(TICKS_HI,NSEC_PER_TICK)
		jumpr r31
	}
#undef NSECS
#undef NSECS_HI
#undef TICKS_LO
#undef TICKS_HI
#undef TICKS
#undef NSEC_PER_TICK
FUNC_END H2K_timer_get_timeout

FUNC_START H2K_timer_set_timeout_tick .text.time.timer
#define ME_DEVPTR r19:18
#define ME r19
#define DEVPTR r18
#define TIMEOUT_TICK r17:16
#define TIMEOUT_TICK_HI r17
#define TIMEOUT_TICK_LO r16
#define NEXT_TICKS r11:10
#define LAST_TICKS r5:4
#define ZERO r7:6
	{
		memd(r29+#-16) = TIMEOUT_TICK
		allocframe(#16)
		TIMEOUT_TICK = r3:2
	}
#define CUR_TIMEOUT r3:2	// must be r3:2 for argument to tree_remove_key
	{
		memd(r29+#0) = ME_DEVPTR
		LAST_TICKS = memd(H2K_GP+#KG_time_last_ticks)
		ME = r4
		ZERO = #0
	}
	k0lock
	{
		p0 = cmp.gtu(TIMEOUT_TICK,LAST_TICKS)
		if (!p0.new) TIMEOUT_TICK = ZERO
		CUR_TIMEOUT = memd(ME+#CONTEXT_timeout)
	}
	{
		p0 = cmp.eq(CUR_TIMEOUT,ZERO)
		if (p0.new) jump:t .Lskip_remove
	}
	{
		r0 = add(H2K_GP,#KG_time_timeouts)
		r1 = add(ME,#CONTEXT_tree)
		call H2K_tree_remove_key
	}
#undef CUR_TIMEOUT
.Lskip_remove:
	{
		ZERO = #0
		memd(ME+#CONTEXT_timeout) = TIMEOUT_TICK
	}
	{
		memd(ME+#CONTEXT_rightleft) = ZERO
		p0 = cmp.eq(TIMEOUT_TICK,ZERO)
		if (p0.new) jump:nt .Lskip_add
	}
	{
		r0 = add(H2K_GP,#KG_time_timeouts)
		r1 = add(ME,#CONTEXT_tree)
		r3:2 = TIMEOUT_TICK
		call H2K_tree_add_key
	}
.Lskip_add:
#define NSEC_PER_TICK r15
	{
		NEXT_TICKS = memd(H2K_GP+#KG_time_next_ticks)
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
	}
	{
		p0 = cmp.gtu(NEXT_TICKS,TIMEOUT_TICK)
		if (!p0.new) jump:nt .Lskip_hw_set
		DEVPTR = add(DEVPTR,#HW_COUNT_LO)
		NSEC_PER_TICK = #TIMERHW_NSEC_PER_TICK
	}
	{
		memw(DEVPTR+#HW_MATCH_HI-HW_COUNT_LO) = #-1
		memd(H2K_GP+#KG_time_next_ticks) = TIMEOUT_TICK
	}
	{
		memw(DEVPTR+#HW_MATCH_LO-HW_COUNT_LO) = TIMEOUT_TICK_LO
		memw(DEVPTR+#HW_MATCH_HI-HW_COUNT_LO) = TIMEOUT_TICK_HI
	}
	.falign
.Lskip_hw_set:
	k0unlock
	{
		r1:0 = mpyu(TIMEOUT_TICK_LO,NSEC_PER_TICK)
		ME_DEVPTR = memd(r29+#0)
	}
	{
		r1 += mpyi(TIMEOUT_TICK_HI,NSEC_PER_TICK)
		TIMEOUT_TICK = memd(r29+#8)
		dealloc_return
	}
#undef NEXT_TICKS
#undef ME_DEVPTR
#undef ME
#undef DEVPTR
#undef TIMEOUT_TICK
#undef TIMEOUT_TICK_HI
#undef TIMEOUT_TICK_LO
#undef LAST_TICKS
#undef CUR_TIMEOUT
#undef ZERO
#undef NSEC_PER_TICK
FUNC_END H2K_timer_set_timeout_tick

FUNC_START H2K_timer_set_timeout .text.time.timer
#define MAGIC_HI r15
#define MAGIC_LO r14
#define HL r13:12
#define HL_H r13
#define HL_L r12
#define LH r11:10
#define LH_H r11
#define LH_L r10
#define HH r9:8
#define FOREVER r7:6
#define DEVPTR r5
#define ME r4
#define TIMEOUT r3:2
#define TIMEOUT_HI r3
#define TIMEOUT_LO r2
#define ONE r1
	{
		MAGIC_LO = ##0xc4ec4ec5
		MAGIC_HI = ##0x4ec4ec4e
	}
	{
		HL = mpyu(TIMEOUT_HI,MAGIC_LO)
		LH_H = #0
		LH_L = mpyu(TIMEOUT_LO,MAGIC_LO)
		ONE = #1
	}
	{
		LH += mpyu(TIMEOUT_LO,MAGIC_HI)
		HH = mpyu(TIMEOUT_HI,MAGIC_HI)
		FOREVER = #-1
	}
	{
		HH += lsr(HL,#32)
		LH += mpyu(HL_L,ONE)
	}
#undef HL
#undef HL_H
#undef HL_L
#define ZERO r13:12
	{
		HH += lsr(LH,#32)
		p0 = cmp.eq(TIMEOUT,FOREVER)
	}
#undef TIMEOUT
#undef TIMEOUT_HI
#undef TIMEOUT_LO
#define TIMEOUT_TICK r3:2
	{
		TIMEOUT_TICK = lsr(HH,#4)
		ZERO = #0
	}
	{
		if (p0) TIMEOUT_TICK = ZERO
		jump H2K_timer_set_timeout_tick
	}
#undef MAGIC_HI
#undef MAGIC_LO
#undef LH
#undef LH_H
#undef LH_L
#undef HH
#undef FOREVER
#undef DEVPTR
#undef ME
#undef TIMEOUT_TICK
#undef ONE
FUNC_END H2K_timer_set_timeout

FUNC_START H2K_timer_delta_timeout .text.time.timer
#define MAGIC_LO r15
#define ONE r14
#define MAGIC_LO_ONE r15:14
#define HL r13:12
#define HL_H r13
#define HL_L r12
#define LH r11:10
#define LH_H r11
#define LH_L r10
#define HH r9:8
#define CREV r7
#define V61 r6
#define DEVPTR r5
#define ME r4
#define TIMEOUT r3:2
#define TIMEOUT_HI r3
#define TIMEOUT_LO r2
#define MAGIC_HI r0

	{
		MAGIC_LO_ONE = combine(##0xc4ec4ec5,#1)
		MAGIC_HI = ##0x4ec4ec4e
	}
	{
		HL = mpyu(TIMEOUT_HI,MAGIC_LO)
		LH_H = #0
		LH_L = mpyu(TIMEOUT_LO,MAGIC_LO)
		DEVPTR = memw(H2K_GP+#KG_time_devptr)
	}
#undef MAGIC_LO
#undef MAGIC_LO_ONE
#define TICKS_HI r15
#define TICKS_LO r14
#define TICKS r15:14
	{
		LH += mpyu(TIMEOUT_LO,MAGIC_HI)
		HH = mpyu(TIMEOUT_HI,MAGIC_HI)
		DEVPTR = add(DEVPTR,#HW_COUNT_LO)
	}
#undef MAGIC_HI
	{
		HH += lsr(HL,#32)
		LH += mpyu(HL_L,ONE)
#undef HL
#undef HL_H
#undef HL_L
#undef ONE
#define MIN r13:12
#define TICKS_HI2 r0
		TICKS_HI = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
		TICKS_LO = memw(DEVPTR+#HW_COUNT_LO-HW_COUNT_LO)
	}
	{
		HH += lsr(LH,#32)
#undef LH
#define LASTTICKS r11:10
		MIN = #TICK_GRANULARITY
		TICKS_HI2 = memw(DEVPTR+#HW_COUNT_HI-HW_COUNT_LO)
	}
	{
		p1 = cmp.eq(TICKS_HI2,TICKS_HI)
		if (!p1.new) jump:nt H2K_timer_delta_timeout
	}
#undef TICKS_HI2
#define FOREVER r1:0
#undef CREV
#undef V61
#define TIMEOUT_TICK r7:6
	{
		LASTTICKS = memd(H2K_GP+#KG_time_last_ticks)
		TIMEOUT_TICK = lsr(HH,#4)
		FOREVER = #-1
	}
#undef HH
#define PCYCLES r9:8
	{
		PCYCLES = s31:30
		TIMEOUT_TICK = max(TIMEOUT_TICK,MIN)
#undef MIN // r13:12
#define ZERO r13:12
		ZERO = #0
	}
	{
		p1 = cmp.gtu(TICKS,LASTTICKS)
		TIMEOUT_TICK = add(TIMEOUT_TICK,TICKS)
		if (p1.new) memd(H2K_GP+#KG_time_last_ticks) = TICKS
		if (p1.new) memd(H2K_GP+#KG_time_last_pcycles) = PCYCLES
	}
	{
		p0 = cmp.eq(TIMEOUT,FOREVER)
		if (p0.new) TIMEOUT = ZERO
		if (!p0.new) TIMEOUT = TIMEOUT_TICK
		jump H2K_timer_set_timeout_tick
	}
#undef FOREVER
#undef TICKS	// r15:14
#undef TICKS_HI
#undef TICKS_LO
#undef ZERO // r13:12
#undef LASTTICKS // r11:10
#undef PCYCLES // r9:8
#undef TIMEOUT_TICK // r7:6
#undef DEVPTR // r5
#undef ME // r4
#undef TIMEOUT // r3:2
#undef TIMEOUT_HI
#undef TIMEOUT_LO
FUNC_END H2K_timer_delta_timeout

#define H2K_TIMER_TRAP_INVALID 6

	.section .rodata
	.p2align 3
	.type	H2K_timer_trap_handlers, @object
	.size	H2K_timer_trap_handlers, H2K_TIMER_TRAP_INVALID*4
H2K_timer_trap_handlers:
	.word	H2K_timer_get_freq
	.word	H2K_timer_get_resolution
	.word	H2K_timer_get_time
	.word	H2K_timer_get_timeout
	.word	H2K_timer_set_timeout
	.word	H2K_timer_delta_timeout

FUNC_START H2K_timer_trap .text.time.timer
	{
		p0 = cmp.gtu(r0,#H2K_TIMER_TRAP_INVALID-1)
		if (p0.new) jumpr:nt r31
		if (p0.new) r1 = #-1
		if (p0.new) r0 = #-1
	}
	{
		r7 = memw(r0<<#2+##H2K_timer_trap_handlers)
	}
	{
		jumpr r7
	}
FUNC_END H2K_timer_trap

FUNC_START H2K_vmtrap_timer .text.time.timer
#define ME r16
	{
		allocframe(#0)
		ME = r0
	}
	{
		r3:2 = memd(ME+#CONTEXT_r0302)
		r0 = memw(ME+#CONTEXT_r00)
		r4 = ME
		call H2K_timer_trap
	}
	.falign
	{
		memd(ME+#CONTEXT_r0100) = r1:0
		deallocframe
	}
	jumpr r31
#undef ME
FUNC_END H2K_vmtrap_timer

