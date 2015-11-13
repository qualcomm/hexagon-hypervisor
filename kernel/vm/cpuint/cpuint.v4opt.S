/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>
#include <vmint.h>
#include <idtype.h>
#include <vmdefs.h>

#define OP_INDEX r4

FUNC_START H2K_cpuint_chain .text.vm.int .p2align 2 // s32_t int_chain(vmblock,me,intno,info,op,num_ints)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define HANDLER r8
	{
		INTNO = add(INTNO,#-PERCPU_INTERRUPTS)
		INFO = add(INFO,#INTINFO_SIZE)
		HANDLER=memw(INFO+#INTINFO_handlers+INTINFO_SIZE)
	}
	HANDLER=memw(HANDLER+OP_INDEX<<#2)
	jumpr HANDLER
#undef HANDLER
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_cpuint_chain

FUNC_START H2K_vm_cpuint_post .text.vm.int .p2align 2 // s32 post(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define FN r4
#define TMP r5
#define ENABLED r7
#define PENDING r6
#define WADDR r8
#define MASK r10
	{
		OP_INDEX = #INTOP_INDEX_post
		p0 = cmp.gt(INTNO,#PERCPU_INTERRUPTS-1)
		if (p0.new) jump:nt H2K_cpuint_chain
	}
	{
		r4 = ##H2K_vm_int_deliver
		jump H2K_vm_cpuint_post_common
	}
FUNC_END H2K_vm_cpuint_post

FUNC_START H2K_vm_cpuint_post_locked .text.vm.int .p2align 2 // s32 post(vmblock,me,intno,info)
	{
		r4 = ##H2K_vm_int_deliver_locked
		jump H2K_vm_cpuint_post_common
	}
FUNC_END H2K_vm_cpuint_post

FUNC_START H2K_vm_cpuint_post_common .text.vm.int .p2align 2 // s32 post_common(vmblock,me,intno,info,fn)

	{
		WADDR = add(ME,#CONTEXT_cpuint_enabled_pending)
		ENABLED = #0
	}
1:
	{
		PENDING = memw_locked(WADDR)
		MASK = lsl(#1,INTNO)
	}
	{
		p0 = tstbit(PENDING,INTNO)	// already set?
		if (p0.new) jump:nt .Lret_zero
		PENDING = or(PENDING,MASK)
		ENABLED = combine(ENABLED.H,PENDING.H)
	}
	{
		memw_locked(WADDR,p0) = PENDING
		PENDING = and(PENDING,ENABLED)
	}
	{
		if (!p0) jump 1b
		WADDR = add(ME,#CONTEXT_atomic_status_word)
	}
1:
	{
		TMP = memw_locked(WADDR)
	}
	{
		TMP = or(TMP,#1<<H2K_VMSTATUS_VMWORK_BIT)
	}
	{
		memw_locked(WADDR,p0) = TMP
		p1 = cmp.eq(PENDING,#0)
	}
	{
		if (!p0) jump 1b
		if (p1) jump .Lret_zero
	}
	{
		allocframe(#0)
		callr r4
	}
	{
		r0 = #0
		dealloc_return
	}
#undef ENABLED
#undef PENDING
#undef MASK
#undef TMP
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_post

FUNC_START H2K_vm_cpuint_clear .text.vm.int .p2align 2 // s32 clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define BITIDX r5
#define TMP r6
#define WADDR r7
	{
		OP_INDEX = #INTOP_INDEX_clear
		WADDR = add(ME,#CONTEXT_cpuint_pending)
		jump H2K_vm_cpuint_discommon
		BITIDX = INTNO
	}
#undef BITIDX
#undef TMP
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_clear

FUNC_START H2K_vm_cpuint_enable .text.vm.int .p2align 2 // s32 clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define ENABLED_PENDING r6
#define BITIDX r7
#define WADDR r8
#define TMP r9
#define MASK r10
#define L2ACK r11
#define L2WIDX_L2BIDX r15:14
#define L2WIDX r15
#define L2BIDX r14
	{
		OP_INDEX = #INTOP_INDEX_enable
		p0 = cmp.gt(INTNO,#PERCPU_INTERRUPTS-1)
		if (p0.new) jump:nt H2K_cpuint_chain
		WADDR = add(ME,#CONTEXT_cpuint_enabled_pending)
		BITIDX = add(INTNO,#16)
	}
1:
	{
		ENABLED_PENDING = memw_locked(WADDR)
		MASK = lsl(#1,BITIDX)
	}
	{
		ENABLED_PENDING = or(ENABLED_PENDING,MASK)
		p0 = tstbit(ENABLED_PENDING,BITIDX)
		if (p0.new) jump:nt .Lret_zero
		L2ACK = memw(H2K_GP+#KG_l2_ack_base)
	}
	{
		memw_locked(WADDR,p0) = ENABLED_PENDING
	}
	{
		if (!p0) jump 1b
		WADDR = add(ME,#CONTEXT_atomic_status_word)
		ENABLED_PENDING &= lsr(ENABLED_PENDING,#16)
	}
	{
		TMP = memw(VMBLOCK+#VMBLOCK_int_v2p)
		if (cmp.eq(TMP.new,#0)) jump:nt 1f
	}
	{
		TMP = memuh(TMP+INTNO<<#1)
		if (cmp.eq(TMP.new,#0)) jump:nt 1f
	}
	{
		L2WIDX_L2BIDX = bitsplit(TMP,#5)
	}
	{
		L2WIDX = add(L2WIDX,#-1)
		MASK = lsl(#1,L2BIDX)
	}
	{
		memw(L2ACK+L2WIDX<<#2) = MASK
	}
1:
	{
		TMP = memw_locked(WADDR)
	}
	{
		p0 = cmp.eq(ENABLED_PENDING,#0)
		// p0 = tstbit(TMP,#H2K_VMSTATUS_VMWORK_BIT)
		if (p0.new) jump:t 9f // if nothing enabled+pending, skip vmwork
		TMP = or(TMP,#1<<H2K_VMSTATUS_VMWORK_BIT)
	}
	memw_locked(WADDR,p0) = TMP
	if (!p0) jump 1b
	.falign
9:
	{
		r0 = #0
		jumpr r31
	}
#undef L2ACK
#undef L2WIDX_L2BIDX
#undef L2WIDX
#undef L2BIDX
#undef ENABLED_PENDING
#undef BITIDX
#undef MASK
#undef WADDR
#undef TMP
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_enable

FUNC_START H2K_vm_cpuint_discommon .text.vm.int .p2align 2 // s32 discommon !!SPECIAL!!
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define BITIDX r5
#define WADDR r7 /* INPUT */
#define TMP r6
	{
		p0 = cmp.gt(INTNO,#PERCPU_INTERRUPTS-1)
		if (p0.new) jump:nt H2K_cpuint_chain
	}
1:
	TMP = memw_locked(WADDR)
	TMP = clrbit(TMP,BITIDX)
	memw_locked(WADDR,p1) = TMP
	if (!p1) jump 1b
	{
		r0 = #0
		jumpr r31
	}
#undef BITIDX
#undef WADDR
#undef TMP
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_discommon

FUNC_START H2K_vm_cpuint_disable .text.vm.int .p2align 2 // s32 clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define BITIDX r5
#define WADDR r7
	{
		OP_INDEX = #INTOP_INDEX_disable
		WADDR = add(ME,#CONTEXT_cpuint_enabled_pending)
		jump H2K_vm_cpuint_discommon
		BITIDX = add(INTNO,#16)
	}
#undef BITIDX
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_disable

FUNC_START H2K_vm_cpuint_localdis .text.vm.int .p2align 2
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
	OP_INDEX = #INTOP_INDEX_localdis
.Lcpuint_invalid_common:
	{
		p0 = cmp.gt(INTNO,#PERCPU_INTERRUPTS-1)
		if (p0.new) jump:nt H2K_cpuint_chain
	}
	{
		r0 = #-1
		jumpr r31
	}
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_localdis
	
FUNC_START H2K_vm_cpuint_localen .text.vm.int .p2align 2
	{
		OP_INDEX = #INTOP_INDEX_localen
		jump .Lcpuint_invalid_common
	}
FUNC_END H2K_vm_cpuint_localen

FUNC_START H2K_vm_cpuint_setaffinity .text.vm.int .p2align 2
	{
		OP_INDEX = #INTOP_INDEX_setaffinity
		jump .Lcpuint_invalid_common
	}
FUNC_END H2K_vm_cpuint_setaffinity

FUNC_START H2K_vm_cpuint_get .text.vm.int .p2align 2 // s32_t get(vmblock,me,offset,info)
#define VMBLOCK r0
#define ME r1
#define OFFSET r2
#define INFO r3
#define BITIDX r5
#define ENABLED_PENDING r6
#define MASK r7
#define BITIDX_TMP r8
#define WADDR r10
	{
		WADDR = add(ME,#CONTEXT_cpuint_enabled_pending)
	}
1:
	{
		BITIDX = memw_locked(WADDR)
	}
	{
		ENABLED_PENDING = BITIDX
		BITIDX &= lsr(BITIDX,#16)
		if (cmp.eq(BITIDX.new,#0)) jump:nt .Lget_next
	}
	{
		BITIDX = ct0(BITIDX)
	}
	{
		ENABLED_PENDING = clrbit(ENABLED_PENDING,BITIDX)
		BITIDX_TMP = add(BITIDX,#16)
	}
	{
		ENABLED_PENDING = clrbit(ENABLED_PENDING,BITIDX_TMP)
	}
	{
		memw_locked(WADDR,p0) = ENABLED_PENDING
	}
	{
		if (!p0) jump 1b
	}
	{
		r0 = add(BITIDX,OFFSET)
		jumpr r31
	}
.Lget_next:
	{
		OP_INDEX = #INTOP_INDEX_get
		OFFSET = add(OFFSET,#2*PERCPU_INTERRUPTS)
		jump H2K_cpuint_chain
	}
#undef WADDR
#undef PENDING
#undef ENABLED
#undef ENABLED_PENDING
#undef BITIDX
#undef BITIDX_TMP
#undef INFO
#undef OFFSET
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_get

FUNC_START H2K_vm_cpuint_peek .text.vm.int .p2align 2 // s32_t peek(vmblock,me,offset,info)
#define VMBLOCK r0
#define ME r1
#define OFFSET r2
#define INFO r3
#define ENABLED_PENDING r6
	{
		ENABLED_PENDING = memw(ME+#CONTEXT_cpuint_enabled_pending)
	}
	{
		ENABLED_PENDING &= lsr(ENABLED_PENDING,#16)
		if (cmp.eq(ENABLED_PENDING.new,#0)) jump:nt .Lpeek_next
	}
	{
		ENABLED_PENDING = ct0(ENABLED_PENDING)
	}
	{
		r0 = add(ENABLED_PENDING,OFFSET)
		jumpr r31
	}
.Lpeek_next:
	{
		OFFSET = add(OFFSET,#2*PERCPU_INTERRUPTS)
		OP_INDEX = #INTOP_INDEX_peek
		jump H2K_cpuint_chain
	}
#undef ENABLED
#undef PENDING
#undef ENABLED_PENDING
#undef CPU
#undef INFO
#undef OFFSET
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_peek

FUNC_START H2K_vm_cpuint_status .text.vm.int .p2align 2 // s32 status(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define ENABLED_PENDING r6
#define BITIDX r7
	{
		OP_INDEX = #INTOP_INDEX_status
		p0 = cmp.gt(INTNO,#PERCPU_INTERRUPTS-1)
		if (p0.new) jump:nt H2K_cpuint_chain
		ENABLED_PENDING = memw(ME+#CONTEXT_cpuint_enabled_pending)
		BITIDX = add(INTNO,#16)
	}
	{
		p1 = tstbit(ENABLED_PENDING,BITIDX)
		p0 = tstbit(ENABLED_PENDING,INTNO)
		if (p0.new) r0 = #1
		if (!p0.new) r0 = #0
	}
	{
		if (p1) r0 = add(r0,#4)
		jumpr r31
	}
#undef ENABLED
#undef PENDING
#undef ENABLED_PENDING
#undef PERCPU
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_cpuint_status

FUNC_START H2K_vm_cpuint_nop .text.vm.int .p2align 2
.Lret_zero:
	{
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_vm_cpuint_nop

	.section .rodata
	.p2align 2
	.global H2K_vm_cpuint_ops
H2K_vm_cpuint_ops:
	.word H2K_vm_cpuint_nop
	.word H2K_vm_cpuint_enable
	.word H2K_vm_cpuint_disable
	.word H2K_vm_cpuint_localen
	.word H2K_vm_cpuint_localdis
	.word H2K_vm_cpuint_setaffinity
	.word H2K_vm_cpuint_get
	.word H2K_vm_cpuint_peek
	.word H2K_vm_cpuint_status
	.word H2K_vm_cpuint_post
	.word H2K_vm_cpuint_clear

	.size H2K_vm_cpuint_ops,.-H2K_vm_cpuint_ops

