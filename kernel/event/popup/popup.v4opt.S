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
#define PRIOMASK r6
#define WAITMASK r7
#define WAIT_PRIOMASKS r7:6
#define HANDLERBASE r8
#define RUNLIST r9
#define RUNLIST_PRIOS r10
#define READY_VALID r11
#define READY_VALID_WORD r12
#define NEGONE r13
#define NEWPRIO r14
#define PRIO r15
	k0lock
	{
		WAIT_PRIOMASKS = memd(H2K_GP+#KG_lowprio_masks)		// share w/ priomask?
		HANDLERBASE = add(H2K_GP,#KG_inthandlers)
		NEWPARAM_HANDLER = #0
		RUNLIST = add(H2K_GP,#KG_runlist)
	}
	{
		memd(HANDLERBASE+INTNO<<#3) = NEWPARAM_HANDLER
		if (!p0.new) PRIO = memub(PARAM+#CONTEXT_prio)
		p0 = cmp.eq(PARAM,#0) 
		if (p0.new) jump:nt .Lnohandler
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
	}
#undef NEWPARAM_HANDLER
#define WIDX r5
#define BITIDX r4
#define WIDX_BITIDX r5:4
#undef HANDLERBASE
#define READY r8
	{
		WAITMASK = clrbit(WAITMASK,HWTNUM)
		PRIOMASK = clrbit(PRIOMASK,HWTNUM)
		NEWPRIO = memub(PARAM+#CONTEXT_prio)
		memw(RUNLIST+HWTNUM<<#2) = PARAM
	}
	{
		memd(H2K_GP+#KG_lowprio_masks) = WAIT_PRIOMASKS
		memh(RUNLIST_PRIOS+HWTNUM<<#1) = NEWPRIO
		p1 = cmp.eq(ME,#0)
		if (p1.new) jump:t 1f
		NEGONE = #-1
	}
#undef WAIT_PRIOMASKS
#undef PRIOMASK
#undef WAITMASK
#undef RUNLIST_PRIOS
#undef RUNLIST
#define PREV_HEAD r7:6
#define PREV r7
#define HEAD r6
	{
		memb(ME+#CONTEXT_status) = #STATUS_READY
		READY_VALID = add(H2K_GP,#KG_ready_valids)
		WIDX_BITIDX = bitsplit(PRIO,#5)
		READY = add(H2K_GP,#KG_ready)
	}
	{
		READY_VALID_WORD = memw(READY_VALID+WIDX<<#2)
		HEAD = memw(READY+PRIO<<#2)
	}
	{
		p0 = cmp.eq(HEAD,#0)
		if (p0.new) PREV_HEAD = combine(ME,ME)	// if empty, point to self
		if (!p0.new) PREV = memw(HEAD+#4)	// get *ring->prev
		if (p0.new) memw(READY+PRIO<<#2) = ME	// if ring empty, point to new node
	}
	{
		if (!p0) memw(HEAD+#4) = ME		// *ring->prev = node
		if (!p0) memw(PREV+#0) = ME
		READY_VALID_WORD = setbit(READY_VALID_WORD,BITIDX)
	}
1:
	{
		IMASK = NEGONE
	}
	{
		r1:0 = combine(PARAM,ME)
		jump H2K_switch
		if (!p1) memw(READY_VALID+WIDX<<#2) = READY_VALID_WORD
		if (!p1) memd(ME) = PREV_HEAD
	}
.Lnohandler:
	k0unlock
	jumpr r31
#undef INTNO
#undef ME
#undef HWTNUM
#undef PARAM
#undef RUNLIST
#undef RUNLIST_PRIOS
#undef READY_VALID_ADDR
#undef READY_VALID_WORD
#undef NEGONE
#undef NEWPRIO
#undef PRIO
#undef READY_RING
#undef WIDX
#undef BITIDX
#undef WIDX_BITIDX
#undef PREV_HEAD
#undef PREV
#undef HEAD
FUNC_END H2K_popup_int

FUNC_START H2K_popup_wait .text.int.popup .falign
#define INTNO r0
#define ME r1
#define HWTNUM r2
#define INTHANDLERS_ADDR r3
#define PARAM_HANDLER r5:4
#define RUNLIST r6
#define RUNLIST_PRIOS r7
#define OLDHANDLER r8
#define OLDPARAM r9
#define OLDPARAM_HANDLER r9:8
#define ZERO r11
#define NEGONE r10
#define ZERO_NEGONE r11:10
#define ACKBASE r12
#define MASK r13
#define RETURN_PAIR r15:14
	{
		p0 = cmp.gtu(INTNO,#MAX_INTERRUPTS-1)
		if (p0.new) jump:nt 9f
		p1 = cmp.eq(INTNO,#31)
		if (p1.new) jump:nt 9f
		HWTNUM = memub(ME+#CONTEXT_hthread)
	}
	k0lock
	{
		PARAM_HANDLER = combine(ME,##H2K_popup_int)
		INTHANDLERS_ADDR = add(H2K_GP,#KG_inthandlers)
		RUNLIST = add(H2K_GP,#KG_runlist)
	}
	{
		OLDPARAM_HANDLER = memd(INTHANDLERS_ADDR+INTNO<<#3)
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
		ZERO_NEGONE = combine(#0,#-1)
	}
	{
		p0 = cmp.eq(OLDPARAM,#0)
		if (!p0.new) jump:nt 8f
		if (p0.new) memd(INTHANDLERS_ADDR+INTNO<<#3) = PARAM_HANDLER
	}
#undef OLDPARAM_HANDLER
#undef OLDPARAM
#undef OLDHANDLER
#define WIDX r9
#define BITNO r8
#define WIDX_BITNO r9:8
	{
		memh(RUNLIST_PRIOS+HWTNUM<<#1) = NEGONE
		memw(RUNLIST+HWTNUM<<#2) = ZERO
		RETURN_PAIR = combine(#0,INTNO)
		INTNO = add(INTNO,#-32)
	}
	{
		memb(ME+#CONTEXT_status) = #STATUS_INTBLOCKED
		memd(ME+#CONTEXT_r0100) = RETURN_PAIR
		WIDX_BITNO = bitsplit(INTNO,#5)
	}
	{
		ACKBASE = memw(H2K_GP+#KG_l2_ack_base)
		p0 = cmp.gt(INTNO,#-1)
		if (!p0.new) jump:nt 1f
		MASK = lsl(#1,BITNO)
	}
	{
		memw(ACKBASE+WIDX<<#2) = MASK
		r1:0 = combine(HWTNUM,ME)
		jump H2K_dosched
	}
1:
	ciad(MASK)
	{
		r1:0 = combine(HWTNUM,ME)
		jump H2K_dosched
	}
8:
	k0unlock
9:
	{
		r0 = #-1
		jumpr r31
	}
#undef INTNO
#undef ME
#undef HWTNUM
#undef INTHANDLERS_ADDR
#undef PARAM_HANDLER
#undef RUNLIST
#undef RUNLIST_PRIOS
#undef ZERO_NEGONE
#undef ZERO
#undef NEGONE
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
		memd(INTHANDLERS+INTNO<<#2) = LLZERO
		memw(DEST+#CONTEXT_r00) = #-1
		jumpr r31
	}
#undef DEST
#undef LLZERO
#undef INTNO
#undef INTHANDLERS
FUNC_END H2K_popup_cancel

