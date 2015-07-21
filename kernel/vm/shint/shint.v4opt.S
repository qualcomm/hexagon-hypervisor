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

#define WORDIDX_BITIDX r7:6
#define WORDIDX r7
#define BITIDX r6
#define NUM_INTS r5
#define OP_INDEX r4

FUNC_START H2K_vm_shint_deliver_cpu .text.vm.int .p2align 2 // static s32_t shint_deliver_cpu(vmblock,dest,intno)
#define VMBLOCK r0
#define DEST r1
#define INTNO r2
#define CPU r8
#define PERCPU r9
#define STATUS_WORD r10
#define TMP r11
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		CPU = memw(DEST+#CONTEXT_id)
		PERCPU=memw(VMBLOCK+#VMBLOCK_percpu_mask)
	}
	{
		allocframe(#0)
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
	}
	{
		PERCPU=memw(PERCPU+CPU<<#2)
	}
	{
		PERCPU=memw(PERCPU+WORDIDX<<#2)
		STATUS_WORD=add(DEST,#CONTEXT_atomic_status_word)
	}
1:
	TMP=memw_locked(STATUS_WORD)
	{
		p0 = tstbit(PERCPU,BITIDX)
		if (!p0.new) jump:nt .Ldealloc_ret_zero
		TMP=or(TMP,##1<<H2K_VMSTATUS_VMWORK_BIT)
	}
	memw_locked(STATUS_WORD,p0) = TMP
	if (!p0) jump 1b
	call H2K_vm_int_deliver
	{
		// if (ret == -1) return 0; else return 1
		r0 = add(r0,#1)
		dealloc_return
	}
.Ldealloc_ret_zero:
	{
		r0 = #0
		dealloc_return
	}
#undef TMP
#undef STATUS_WORD
#undef PERCPU
#undef CPU
#undef VMBLOCK
#undef DEST
#undef INTNO
FUNC_END H2K_vm_shint_deliver_cpu

FUNC_START H2K_vm_shint_deliver .text.vm.int .p2align 2 // s32 shint_deliver(vmblock,me,intno) returns zero
#define VMBLOCK r0
#define ME r1
#define INTNO r2
	{
		allocframe(#24)
		memd(r29+#-16) = r17:16
	}
#define MASK r17:16
#define ZERO r11:10
	{
		MASK = memd(VMBLOCK+#VMBLOCK_waiting_cpus)
		ZERO = #0
	}
	{
		p0 = cmp.eq(MASK,ZERO)
		if (p0.new) jump:nt .Ldeliver_mask_loop_done
	}
// Make sure MASK is limited to waiting thread <= max_cpus
// EJP: it is, must vmwait to set bit
// foreach bit in mask vmblock->waiting_cpus
//   try and deliver interrupt to CPU
#define ONE r9
#define I r8
#define ONE_I r9:8
#define TMP r15
1:
	{
		I = ct0(MASK)
		ONE = #1
		r1 = memw(VMBLOCK+#VMBLOCK_contexts)
		TMP = #CONTEXT_SIZE
		
	}
	{
		MASK = insert(r1:0,ONE_I)	// insert zero bit, since vmblock is aligned
		r1 += mpyi(I,TMP)
	}
	{
		call H2K_vm_shint_deliver_cpu
		memd(r29+#0) = r1:0
		memd(r29+#8) = r3:2
	}
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:t .Lshint_deliver_done
		ZERO = #0
		r1:0 = memd(r29+#0)
		r3:2 = memd(r29+#8)
	}
	{
		p0 = cmp.eq(MASK,ZERO)
		if (!p0.new) jump:t 1b
	}
#undef MASK
#undef ZERO
#undef TMP
#undef ONE
#undef I
#undef ONE_I
.Ldeliver_mask_loop_done:
// If not found in waiting cpus,
// Starting with my cpu
// for each cpu
//   try to deliver interrupt to CPU

#define I r17
#define CPUID r16
#define MAX_CPUS r8
#define TMP r9
	{
		CPUID = memw(ME+#CONTEXT_id)
	}
	{
		CPUID = extractu(CPUID,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
	}
	{
		r1 = memw(VMBLOCK+#VMBLOCK_contexts)
		TMP = #CONTEXT_SIZE
		I = CPUID
	}
1:
	{
		call H2K_vm_shint_deliver_cpu
		memd(r29+#0) = r1:0
		memd(r29+#8) = r3:2
		r1 += mpyi(I,TMP)
	}
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:t .Lshint_deliver_done
		r1:0 = memd(r29+#0)
		r3:2 = memd(r29+#8)
	}
	{
		I = add(I,#1)
		MAX_CPUS = memw(VMBLOCK+#VMBLOCK_max_cpus)
		TMP = #CONTEXT_SIZE
	}
	{
		p0 = cmp.gt(MAX_CPUS,I)
		if (!p0.new) I = #0
	}
	{
		p0 = cmp.eq(I,CPUID)
		if (!p0.new) jump:nt 1b
	}
.Lshint_deliver_done:
	{
		r0 = #0
		r17:16 = memd(r29+#0)
		dealloc_return
	}
#undef MAX_CPUS
#undef TMP
#undef I
#undef CPUID
#undef VMBLOCK
#undef ME
#undef INTNO
FUNC_END H2K_vm_shint_deliver

FUNC_START H2K_shint_chain .text.vm.int .p2align 2 // s32_t int_chain(vmblock,me,intno,info,op,num_ints)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define HANDLER r8
	{
		INTNO = sub(INTNO,NUM_INTS)
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
FUNC_END H2K_shint_chain

FUNC_START H2K_vm_shint_post .text.vm.int .p2align 2 // s32 shint_post(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define WADDR r8
#define TMP r9
#define MASK r10
#define ENABLE r11
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		OP_INDEX = #INTOP_INDEX_post
		TMP = memw(VMBLOCK+#VMBLOCK_pending)
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
		WADDR = addasl(TMP,WORDIDX,#2)
		ENABLE = memw(VMBLOCK+#VMBLOCK_enable)
	}
1:
	{
		TMP = memw_locked(WADDR)
		MASK = lsl(#1,BITIDX)
	}
	{	// maybe use OR for setbit
		p0 = tstbit(TMP,BITIDX)
		if (p0.new) jump:nt .Lret_zero
		TMP = or(TMP,MASK)
	}
	memw_locked(WADDR,p1) = TMP
	{
		if (!p1) jump 1b
		ENABLE = memw(ENABLE+WORDIDX<<#2)
	}
	{
		p0 = tstbit(ENABLE,BITIDX)
		if (!p0.new) jump:t .Lret_zero
	}
	jump H2K_vm_shint_deliver
#undef ENABLE
#undef MASK
#undef TMP
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_post

FUNC_START H2K_vm_shint_clear .text.vm.int .p2align 2 // s32 shint_clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define TMP r8
#define WADDR r9
	{
		OP_INDEX = #INTOP_INDEX_clear
		WADDR = memw(VMBLOCK+#VMBLOCK_pending)
		jump H2K_vm_shint_discommon
	}
#undef TMP
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_clear

FUNC_START H2K_vm_shint_enable .text.vm.int .p2align 2 // s32 shint_clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define WADDR r8
#define TMP r9
#define MASK r10
#define L2ACK r11
#define L2WIDX_L2BIDX r15:14
#define L2WIDX r15
#define L2BIDX r14
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		OP_INDEX = #INTOP_INDEX_enable
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
	}
	TMP = memw(VMBLOCK+#VMBLOCK_enable)
	{
		WADDR = addasl(TMP,WORDIDX,#2)
		MASK = lsl(#1,BITIDX)
	}
1:
	TMP = memw_locked(WADDR)
	{	// maybe use OR for setbit
		p0 = tstbit(TMP,BITIDX)
		if (p0.new) jump:nt .Lret_zero
		TMP = or(TMP,MASK)
	}
	memw_locked(WADDR,p1) = TMP
	if (!p1) jump 1b
	{
		TMP = memw(VMBLOCK+#VMBLOCK_int_v2p)
		if (cmp.eq(TMP.new,#0)) jump:nt 1f
		INTNO = add(INTNO, #PERCPU_INTERRUPTS)
	}
	{
		TMP = memuh(TMP+INTNO<<#1)
		if (cmp.eq(TMP.new,#0)) jump:nt 1f
	}
	{
		L2WIDX_L2BIDX = bitsplit(TMP,#5)
	}
	{
		L2ACK = memw(H2K_GP+#KG_l2_ack_base)
		L2WIDX = add(L2WIDX,#-1)
		MASK = lsl(#1,L2BIDX)
	}
	{
		memw(L2ACK + L2WIDX<<#2) = MASK
	}
		
1:
	TMP = memw(VMBLOCK+#VMBLOCK_pending)
	TMP = memw(TMP+WORDIDX<<#2)
	{
		p0 = tstbit(TMP,BITIDX)
		if (!p0.new) jump:nt .Lret_zero
	}
	jump H2K_vm_shint_deliver
#undef L2ACK
#undef L2WIDX_L2BIDX
#undef L2WIDX
#undef L2BIDX
#undef MASK
#undef WADDR
#undef TMP
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_enable

FUNC_START H2K_vm_shint_discommon .text.vm.int .p2align 2 // s32 shint_discommon !!SPECIAL!!
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define TMP r8
#define WADDR r9 /* INPUT */
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
		WADDR = addasl(WADDR,WORDIDX,#2)
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
#undef WADDR
#undef TMP
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_discommon

FUNC_START H2K_vm_shint_disable .text.vm.int .p2align 2 // s32 shint_clear(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define WADDR r9
	{
		OP_INDEX = #INTOP_INDEX_disable
		WADDR = memw(VMBLOCK+#VMBLOCK_enable)
		jump H2K_vm_shint_discommon
	}
#undef WADDR
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_disable

FUNC_START H2K_vm_shint_localdis .text.vm.int .p2align 2 // s32 shint_localdis(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define CPU r8
#define WADDR r9
	{
		OP_INDEX = #INTOP_INDEX_localdis
		CPU = memw(ME+#CONTEXT_id)
	}
	{
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
		WADDR = memw(VMBLOCK+#VMBLOCK_percpu_mask)
	}
	{
		WADDR = memw(WADDR+CPU<<#2)
		jump H2K_vm_shint_discommon
	}
#undef WADDR
#undef CPU
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_localdis

FUNC_START H2K_vm_shint_localen .text.vm.int .p2align 2 // s32 shint_localen(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define CPU r8
#define TMP r9
#define WADDR r10
#define ENABLE r11
#define PENDING r12
#define MASK r13
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		OP_INDEX = #INTOP_INDEX_localen
		CPU = memw(ME+#CONTEXT_id)
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
		TMP = memw(VMBLOCK+#VMBLOCK_percpu_mask)
		ENABLE = memw(VMBLOCK+#VMBLOCK_enable)
	}
	{
		MASK = lsl(#1,BITIDX)
		PENDING = memw(VMBLOCK+#VMBLOCK_pending)
		ENABLE = memw(ENABLE+WORDIDX<<#2)
	}
	{
		PENDING = memw(PENDING+WORDIDX<<#2)
		TMP = memw(TMP+CPU<<#2)
	}
	{
		PENDING &= and(MASK,ENABLE)
		WADDR = addasl(TMP,WORDIDX,#2)
	}
1:
	{
		TMP = memw_locked(WADDR)
	}
	{
		TMP = or(TMP,MASK)
		p0 = tstbit(TMP,BITIDX)
		if (p0.new) jump:nt .Lret_zero
	}
	{
		memw_locked(WADDR,p1) = TMP
		p0 = tstbit(PENDING,BITIDX)
	}
	{
		if (!p1) jump 1b
		if (!p0) jump .Lret_zero
	}
	{
		allocframe(#0)
		call H2K_vm_shint_deliver_cpu
	}
	{
		r0 = #0
		dealloc_return
	}
#undef MASK
#undef PENDING
#undef ENABLE
#undef WADDR
#undef TMP
#undef CPU
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_localen

FUNC_START H2K_vm_shint_setaffinity .text.vm.int .p2align 2 // s32 setaff(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define MAX_CPUS r8
#define SIZE r9
#define I r16
#define ME_SAV r17
#define ME_SAV_I r17:16
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		OP_INDEX = #INTOP_INDEX_setaffinity
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
	}
	{
		allocframe(#24)
		memd(r29+#-16) = ME_SAV_I
		SIZE = #CONTEXT_SIZE
	}
	{
		memd(r29+#8) = r3:2
		ME_SAV_I = combine(ME,#0)
		r1 = memw(VMBLOCK+#VMBLOCK_contexts)
	}
.Loop_setaffinity:
	// foreach cpu:
	//     call H2K_vm_shint_localdis
	{
		memd(r29+#0) = r1:0
		r3:2 = memd(r29+#8)
		call H2K_vm_shint_localdis
		r1 += mpyi(I,SIZE)
	}
	{
		r1:0 = memd(r29+#0)
		I = add(I,#1)
	}
	{
		MAX_CPUS = memw(VMBLOCK+#VMBLOCK_max_cpus)
		if (cmp.gt(MAX_CPUS.new,I)) jump:t .Loop_setaffinity
		SIZE = #CONTEXT_SIZE
	}
	// call H2K_vm_shint_localen
	{
		r3:2 = memd(r29+#8)
		r1 = ME_SAV
		call H2K_vm_shint_localen
	}
	{
		ME_SAV_I = memd(r29+#16)
		dealloc_return
		r0 = #0
	}
#undef ME_SAV
#undef I
#undef ME_SAV_I
#undef SIZE
#undef MAX_CPUS
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_setaffinity

FUNC_START H2K_vm_shint_get .text.vm.int .p2align 2 // s32_t get(vmblock,me,offset,info)
#define VMBLOCK r0
#define ME r1
#define OFFSET r2
#define INFO r3
#define CPU r8
#define PENDING r9
#define PERCPU r10
#define ENABLE r11
#define I r12
#define A r13
#define B r14
#define C r15
	{
		CPU = memw(ME+#CONTEXT_id)
		PERCPU = memw(VMBLOCK+#VMBLOCK_percpu_mask)
	}
	{
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
		PENDING = memw(VMBLOCK+#VMBLOCK_pending)
	}
	{
		ENABLE = memw(VMBLOCK+#VMBLOCK_enable)
		PERCPU = memw(PERCPU+CPU<<#2)
		I = OFFSET
		A = lsr(NUM_INTS,#5)
	}
	{
		loop0(.Lretry_get,A)
		A = memw(PENDING++#4)
	}
.Lretry_get:
	{
		B = memw(PERCPU++#4)
		C = memw(ENABLE++#4)
	}
	A &= and(B,C)
	{
		p0 = cmp.eq(A,#0)
		if (!p0.new) jump:nt .Lget_found
		BITIDX=ct0(A)
	}
	// For shint_get, add atomic clrbits here
	{
		A = memw(PENDING++#4)
		I = add(I,#32)
	}:endloop0
	{
		NUM_INTS = sub(#0,NUM_INTS)
		OP_INDEX = #INTOP_INDEX_get
		jump H2K_shint_chain
	}
#undef A
#undef B
#undef C
#define TMP r15
#define MASK r14
.Lget_found:
	{
		ENABLE = add(ENABLE,#-4)
		PENDING = add(PENDING,#-4)
		PERCPU = add(PERCPU,#-4)
		MASK = #-1
	}
1:
	{
		TMP=memw_locked(ENABLE)
		MASK = clrbit(MASK,BITIDX)
	}
	{	// turn into AND?
		TMP = and(TMP,MASK)
		p0 = tstbit(TMP,BITIDX)
		if (!p0.new) jump:nt .Lretry_get	// already cleared
	}
	{
		memw_locked(ENABLE,p0) = TMP
	}
	{
		if (!p0) jump 1b
	}
2:
	{
		TMP=memw_locked(PENDING)
	}
	{
		TMP = and(TMP,MASK)
		p0 = tstbit(TMP,BITIDX)
		if (!p0.new) jump:nt .Lretry_get	// already cleared
	}
	{
		memw_locked(PENDING,p0) = TMP
		r0 = OFFSET
	}
	{
		if (!p0) jump 2b
	}
	{
		r0 = add(I,BITIDX)
		jumpr r31
	}
#undef TMP
#undef MASK
#undef I
#undef ENABLE
#undef PERCPU
#undef PENDING
#undef CPU
#undef INFO
#undef OFFSET
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_get

FUNC_START H2K_vm_shint_peek .text.vm.int .p2align 2 // s32_t peek(vmblock,me,offset,info)
#define VMBLOCK r0
#define ME r1
#define OFFSET r2
#define INFO r3
#define CPU r8
#define PENDING r9
#define ENABLE r10
#define PERCPU r11
#define I r12
#define A r13
#define B r14
#define C r15
	{
		CPU = memw(ME+#CONTEXT_id)
		PERCPU = memw(VMBLOCK+#VMBLOCK_percpu_mask)
	}
	{
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
		PENDING = memw(VMBLOCK+#VMBLOCK_pending)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		I = OFFSET
	}
	{
		PERCPU = memw(PERCPU+CPU<<#2)
		ENABLE = memw(VMBLOCK+#VMBLOCK_enable)
		A = lsr(NUM_INTS,#5)
	}
	{
		loop0(1f,A)
		A = memw(PENDING++#4)
	}
1:
	{
		B = memw(PERCPU++#4)
		C = memw(ENABLE++#4)
	}
	A &= and(B,C)
	{
		p0 = cmp.eq(A,#0)
		if (!p0.new) jump:nt .Lpeek_found
		A=ct0(A)
	}
	// For shint_get, add atomic clrbits here
	{
		A = memw(PENDING++#4)
		I = add(I,#32)
	}:endloop0
	{
		NUM_INTS = sub(#0,NUM_INTS)
		OP_INDEX = #INTOP_INDEX_peek
		jump H2K_shint_chain
	}
.Lpeek_found:
	{
		r0 = add(I,A)
		jumpr r31
	}
#undef A
#undef B
#undef C
#undef I
#undef PERCPU
#undef ENABLE
#undef PENDING
#undef CPU
#undef INFO
#undef OFFSET
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_peek

FUNC_START H2K_vm_shint_status .text.vm.int .p2align 2 // s32 status(vmblock,me,intno,info)
#define VMBLOCK r0
#define ME r1
#define INTNO r2
#define INFO r3
#define CPU r8
#define PENDING r9
#define PERCPU r10
#define ENABLE r11
	{
		WORDIDX_BITIDX = bitsplit(INTNO,#5)
		NUM_INTS = memw(INFO+#INTINFO_num_ints)
		OP_INDEX = #INTOP_INDEX_status
	}
	{
		p0 = cmp.gt(NUM_INTS,INTNO)
		if (!p0.new) jump:nt H2K_shint_chain
		CPU = memw(ME+#CONTEXT_id)
	}
	{
		CPU = extractu(CPU,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
		PENDING = memw(VMBLOCK+#VMBLOCK_pending)
		ENABLE = memw(VMBLOCK+#VMBLOCK_enable)
	}
	{
		PERCPU = memw(VMBLOCK+#VMBLOCK_percpu_mask)
		ENABLE = memw(ENABLE+WORDIDX<<#2)
	}
	{
		PERCPU = memw(PERCPU+CPU<<#2)
		PENDING = memw(PENDING+WORDIDX<<#2)
	}
	{
		PERCPU = memw(PERCPU+WORDIDX<<#2)
		p0 = tstbit(PENDING,BITIDX)
	}
	{
		p1 = tstbit(PERCPU,BITIDX)
		r0 = mux(p0,#1,#0)
	}
	{
		p2 = tstbit(ENABLE,BITIDX)
		if (p1) r0 = add(r0,#2)
	}
	{
		if (p2) r0 = add(r0,#4)
		jumpr r31
	}
#undef CPU
#undef ENABLE
#undef PENDING
#undef PERCPU
#undef INFO
#undef INTNO
#undef ME
#undef VMBLOCK
FUNC_END H2K_vm_shint_status

FUNC_START H2K_vm_shint_nop .text.vm.int .p2align 2
.Lret_zero:
	{
		r0 = #0
		jumpr r31
	}
FUNC_END H2K_vm_shint_nop

	.section .rodata
	.p2align 2
	.global H2K_vm_shint_ops
H2K_vm_shint_ops:
	.word H2K_vm_shint_nop
	.word H2K_vm_shint_enable
	.word H2K_vm_shint_disable
	.word H2K_vm_shint_localen
	.word H2K_vm_shint_localdis
	.word H2K_vm_shint_setaffinity
	.word H2K_vm_shint_get
	.word H2K_vm_shint_peek
	.word H2K_vm_shint_status
	.word H2K_vm_shint_post
	.word H2K_vm_shint_clear

	.size H2K_vm_shint_ops,.-H2K_vm_shint_ops

