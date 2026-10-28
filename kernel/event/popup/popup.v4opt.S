/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_popup_int .text.int.popup .p2align 5
#define INTNO r0
#define ME r1
#define HWTNUM r2
#define PARAM r3
#define NEWPARAM_HANDLER r5:4
#define HEAD r6
#define PREV r7
#define PREV_HEAD r7:6
#define HANDLERBASE r8
#define READY r9
#define TMP r10
#define READY_VALID r11
#define BITIDX r12
#define WIDX r13
#define WIDX_BITIDX r13:12
#define BESTWAIT r14
#define PRIO r15
#define READY_VALID_WORD r16
#define TMP2 r17
	ASM_BKL_LOCK(r6, r7, r8, p0)
	{
		HANDLERBASE = add(H2K_GP,#KG_inthandlers)
		NEWPARAM_HANDLER = #0
		p1 = cmp.eq(ME,#0)
		READY = add(H2K_GP,#KG_ready)
	}
	{
		memd(HANDLERBASE+INTNO<<#3) = NEWPARAM_HANDLER
		if (!p0.new) READY_VALID = add(H2K_GP,#KG_ready_valids)
		if (!p1) PRIO = memub(ME+#CONTEXT_prio)
		p0 = cmp.eq(PARAM,#0)
		if (p0.new) jump:nt .Lnohandler
	}
	{
		if (!p1) memb(ME+#CONTEXT_status) = #STATUS_READY
		if (p1) jump:t 1f
		WIDX_BITIDX = bitsplit(PRIO,#5)
		memb(PARAM+#CONTEXT_status) = #STATUS_RUNNING
	}

	{
		HEAD = memw(READY+PRIO<<#2)
		BESTWAIT = bestwait
		if (cmp.eq(HEAD.new,#0)) jump:nt .Lempty_entry // this saves us 1 cycle for non cluster sched. 
	}
	{
		READY_VALID_WORD = memw(READY_VALID+WIDX<<#2)
		PREV = memw(HEAD+#4)	// get *ring->prev
		BESTWAIT = minu(BESTWAIT, PRIO)
	}
	{
		bestwait = BESTWAIT
		memw(HEAD+#4) = ME		// *ring->prev = node
		memw(PREV+#0) = ME
		READY_VALID_WORD = setbit(READY_VALID_WORD,BITIDX)
	}
	{
		memw(READY_VALID+WIDX<<#2) = READY_VALID_WORD
		memd(ME) = PREV_HEAD
		r1:0 = combine(PARAM,ME)
		jump H2K_switch
	}

.Lempty_entry:
	{
		BESTWAIT = minu(BESTWAIT, PRIO)
		PREV_HEAD = combine(ME,ME)	// empty, point to self
		memw(READY+PRIO<<#2) = ME	// ring empty, point to new node
		READY_VALID_WORD = memw(READY_VALID+WIDX<<#2)
	}
	{
		READY_VALID_WORD = setbit(READY_VALID_WORD,BITIDX)
		bestwait = BESTWAIT
	}
	{
		memw(READY_VALID+WIDX<<#2) = READY_VALID_WORD
		memd(ME) = PREV_HEAD
		r1:0 = combine(PARAM,ME)
		jump H2K_switch
	}
1:
	{
#if CLUSTER_SCHED
		TMP = lsl(#1, HWTNUM)
	}
	{
		memw(H2K_GP+#KG_wait_mask) -= TMP
#endif
		r1:0 = combine(PARAM,ME)
		jump H2K_switch
	}
.Lnohandler:
	ASM_BKL_UNLOCK(r6, r7, r8, p0)
	jumpr r31
#undef INTNO
#undef ME
#undef HWTNUM
#undef PARAM
#undef READY_VALID_ADDR
#undef READY_VALID_WORD
#undef BESTWAIT
#undef PRIO
#undef READY_RING
#undef WIDX
#undef BITIDX
#undef WIDX_BITIDX
#undef PREV_HEAD
#undef PREV
#undef HEAD
#undef TMP
FUNC_END H2K_popup_int

FUNC_START H2K_popup_wait .text.int.popup .falign
#define INTNO r0
#define ME r1
#define HWTNUM r2
#define INTHANDLERS_ADDR r3
#define PARAM_HANDLER r5:4
#define TMP r6
#define ENTRYPTR r7
#define OLDHANDLER r8
#define OLDPARAM r9
#define OLDPARAM_HANDLER r9:8
#define VMSTATUS r10
#define ACKBASE r12
#define MASK r13
#define RETURN_PAIR r15:14
	{
		p0 = !cmp.gtu(INTNO, #MAX_INTERRUPTS-1)
		p0 = cmp.gtu(INTNO, #L2_FIRST_AVAILABLE_INTERRUPT-1)
		p1 = cmp.gtu(INTNO, #L1_FIRST_AVAILABLE_INTERRUPT-1)
		p1 = !cmp.gtu(INTNO, #L1_INTERRUPTS-1)
	}
	{
		p2 = or(p0,p1)
		if (!p2.new) r0 = #-1
		if (!p2.new) jumpr:nt r31
	}
	ASM_BKL_LOCK(r6, r7, r8, p0)
	{

		TMP = memub(ME+#CONTEXT_vmstatus)
		MASK = #H2K_VMSTATUS_VMWORK
		INTHANDLERS_ADDR = add(H2K_GP,#KG_inthandlers)
		HWTNUM = memub(ME+#CONTEXT_hthread)
	}
	{
		p0 = bitsset(TMP,MASK)
		if (p0.new) jump:nt .Lpopup_wait_vmwork
		ENTRYPTR = addasl(INTHANDLERS_ADDR,INTNO,#3)
	}

#undef TMP	
	{
		OLDPARAM = memw(ENTRYPTR+#4)
		if(!cmp.eq(OLDPARAM.new, #0)) jump:nt 8f
		RETURN_PAIR = combine(#0,INTNO)
		INTNO = add(INTNO,#-32)
	}
#undef OLDPARAM_HANDLER
#undef OLDPARAM
#undef OLDHANDLER
#define WIDX r9
#define BITNO r8
#define WIDX_BITNO r9:8
	{
		PARAM_HANDLER = combine(ME,##H2K_popup_int)
		memd(ME+#CONTEXT_r0100) = RETURN_PAIR
		WIDX_BITNO = bitsplit(INTNO,#5)
	}
	{
		ACKBASE = memw(H2K_GP+#KG_l2_ack_base)
		memd(ENTRYPTR) = PARAM_HANDLER
		p0 = cmp.gt(INTNO,#-1)
		if (!p0.new) jump:nt 1f
		MASK = lsl(#1,BITNO)
	}
	{
		memb(ME+#CONTEXT_status) = #STATUS_INTBLOCKED
		memw(ACKBASE+WIDX<<#2) = MASK
		r1:0 = combine(HWTNUM,ME)
		jump H2K_dosched
	}
1:
	ciad(MASK)
	{
		memb(ME+#CONTEXT_status) = #STATUS_INTBLOCKED
		r1:0 = combine(HWTNUM,ME)
		jump H2K_dosched
	}
8:
	ASM_BKL_UNLOCK(r6, r7, r8, p0)
	{
		r0 = #-1
		jumpr r31
	}
.Lpopup_wait_vmwork:
	{
		allocframe(#0)
		r0 = ME
		call H2K_vm_do_work_withlock
	}
	ASM_BKL_UNLOCK(r6, r7, r8, p0)
	{
		r0 = #-1
		dealloc_return
	}
#undef INTNO
#undef ME
#undef HWTNUM
#undef INTHANDLERS_ADDR
#undef PARAM_HANDLER
#undef ACKBASE
#undef RETURN_PAIR
#undef WIDX
#undef BITNO
#undef WIDX_BITNO
#undef MASK
FUNC_END H2K_popup_wait

FUNC_START H2K_popup_cancel .text.int.popup 
#define DEST r0
#define INTNO r1
#define INTHANDLERS r2
#define LLZERO r5:4
	{
		INTHANDLERS = add(H2K_GP,#KG_inthandlers)
		INTNO = memw(DEST+#CONTEXT_r00)
		LLZERO = #0
	}
	{
		memd(INTHANDLERS+INTNO<<#3) = LLZERO
		memw(DEST+#CONTEXT_r00) = #-1
		jumpr r31
	}
#undef DEST
#undef LLZERO
#undef INTNO
#undef INTHANDLERS
FUNC_END H2K_popup_cancel

