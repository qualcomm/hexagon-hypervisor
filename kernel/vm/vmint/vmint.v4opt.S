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

/* 
 * EJP: I know that having a table may be faster, but I'm pretty sure this is smaller code size.
 * If performance problem, we can resort by frequency or eat the size and make a table
 */

FUNC_START H2K_vm_int_deliver_locked .text.vm.int .p2align 2 // deliver(vmblock,thread,intno)
#define VMBLOCK r0
#define THREAD r1
#define INTNO r2
#define STATUS r3
#define VMSTATUS r4
#define PIE p0
#define PTMP p1
	{
		STATUS = memub(THREAD+#CONTEXT_status)
		VMSTATUS = memw(THREAD+#CONTEXT_atomic_status_word)
	}
	{
		PTMP = cmp.eq(STATUS,#STATUS_VMWAIT)
		if (!PTMP.new) jump:t .Ltry_RUNNING
		PIE = tstbit(VMSTATUS,#H2K_VMSTATUS_IE_BIT)
		memw(THREAD+#CONTEXT_r00) = INTNO
	}
#undef VMSTATUS
#define PRIO r4
.Lenqueue:
	// THREAD must be valid
#define WORDIDX_BITIDX r15:14
#define WORDIDX r15
#define BITIDX r14
#define READY r13
#define READYVALIDS r12
#define MASK r11
#define PREV_NEXT r7:6
#define PREV r7
#define NEXT r6
#define READYHEAD r5
	{
		PRIO = memub(THREAD+#CONTEXT_prio)
		memb(THREAD+#CONTEXT_status) = #STATUS_READY
		READY = add(H2K_GP,#KG_ready)
		READYVALIDS = add(H2K_GP,#KG_ready_valids)
	}
	{
		WORDIDX_BITIDX = bitsplit(PRIO,#5)
		PREV_NEXT = combine(THREAD,THREAD)
		READYHEAD = memw(READY+PRIO<<#2)
	}
	{
		MASK = memw(READYVALIDS+WORDIDX<<#2)
		PTMP = cmp.eq(READYHEAD,#0)
		if (!PTMP.new) NEXT = READYHEAD
		if (!PTMP.new) PREV = memw(READYHEAD+#CONTEXT_prev)
	}
	{
		MASK = setbit(MASK,BITIDX)
		memd(THREAD+#CONTEXT_next) = PREV_NEXT
		if (PTMP) memw(READY+PRIO<<#2) = THREAD
	}
	{
		if (!PTMP) memw(PREV+#CONTEXT_next) = THREAD
		if (!PTMP) memw(NEXT+#CONTEXT_prev) = THREAD
	}
	{
		r1:0 = #0
		memw(READYVALIDS+WORDIDX<<#2) = MASK
		jump H2K_check_sanity
	}
#undef WORDIDX_BITIDX
#undef WORDIDX
#undef BITIDX
#undef READY
#undef READYVALIDS
#undef MASK
#undef PREV_NEXT
#undef PREV
#undef NEXT
#undef READYHEAD
.Ltry_RUNNING:
	{
		PTMP = cmp.eq(STATUS,#STATUS_RUNNING)
		if (!PTMP.new) jump:t .Ltry_INTBLOCKED
	}
	{
		if (!PIE) jump .Lret_zero
		if (PIE) r0 = THREAD
		jump H2K_vm_ipi_send_withlock
	}
.Ltry_INTBLOCKED:
	{
		PTMP = cmp.eq(STATUS,#STATUS_INTBLOCKED)
		if (!PTMP.new) jump:t .Ltry_BLOCKED
		if (!PIE) jump .Lret_zero
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r1:0
		r0 = THREAD
		call H2K_popup_cancel
	}
	{
		deallocframe
		r1:0 = memd(r29+#0)
		jump .Lenqueue
	}
.Ltry_BLOCKED:
	{
		PTMP = cmp.eq(STATUS,#STATUS_BLOCKED)
		if (!PTMP.new) jump:t .Ltry_READY
		if (!PIE) jump .Lret_zero
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r1:0
		r0 = THREAD
		call H2K_futex_cancel
	}
	{
		deallocframe
		r1:0 = memd(r29+#0)
		jump .Lenqueue
	}
.Ltry_READY:
	{
		PTMP = cmp.eq(STATUS,#STATUS_READY)
		if (PTMP.new) r0 = #0
		if (!PTMP.new) r0 = #-1
		jumpr r31
	}
.Lret_zero:
	{
		r0 = #0
		jumpr r31
	}
#undef PTMP
#undef PIE
#undef STATUS
#undef INTNO
#undef THREAD
#undef VMBLOCK
FUNC_END H2K_vm_int_deliver_locked 

FUNC_START H2K_vm_int_deliver .text.vm.int .p2align 2
	allocframe(#0)
	k0lock
	call H2K_vm_int_deliver_locked
	k0unlock
	dealloc_return
FUNC_END H2K_vm_int_deliver

FUNC_START H2K_vmtrap_intop .text.vm.int .p2align 2
#define ME r0
#define VMBLOCK r1
#define OP r2
#define ARG r3
#define ARG_OP r3:2
#define INTINFO r4
#define OPTAB r5
#define DST r6
#define MAX_CPUS r7 
	{
		ARG_OP = memd(ME+#CONTEXT_r0100)
		VMBLOCK = memw(ME+#CONTEXT_vmblock)
	}
	{
		p0 = !cmp.eq(OP,#INTOP_INDEX_get)
		p0 = !cmp.eq(OP,#INTOP_INDEX_peek)
		if (!p0.new) ARG = #0
		INTINFO = memw(VMBLOCK+#VMBLOCK_intinfo)
	}
	{
		p1 = cmp.eq(OP,#INTOP_INDEX_setaffinity)
		if (p1.new) jump:nt .Lfixup_aff
		p0 = cmp.eq(OP,#INTOP_INDEX_post)
		if (p0.new) jump:nt .Lfixup_post
		OPTAB = memw(INTINFO+#INTINFO_handlers)
		DST = ME
	}
.Lfixup_resume:
	{
		p0 = cmp.gtu(OP,#INTOP_INDEX_INVALID-1)
		if (p0.new) jump:nt .Lintop_fail
		if (!p0.new) OPTAB = memw(OPTAB+OP<<#2)
	}
	{
		allocframe(#8)
		memd(r29+#-16) = r17:16
#define ME_SAV r16
		ME_SAV = ME
	}
	{
		r1:0 = combine(DST,VMBLOCK)
		r3:2 = combine(INTINFO,ARG)
		callr OPTAB
	}
	{
		memw(ME_SAV+#CONTEXT_r00) = r0
	}
#undef ME_SAV
	{
		r17:16 = memd(r29+#0)
		dealloc_return
	}
.Lintop_fail:
	{
		memw(ME+#CONTEXT_r00) = #-1
		jumpr r31
	}
#define DST_CPU r8
#define DST_VMBLOCK r9
#define DST_ID r10
#define DST_VM r11
#define ME_ID r12
#define ME_VM r13
#define ME_VMBLOCK_PARENT r14
#define DST_VMBLOCK_PARENT r15
.Lfixup_aff:
	{
		DST_CPU = memw(ME+#CONTEXT_r0302)
		DST_VMBLOCK = VMBLOCK
		jump .Lfixup_cpuidx_to_context
	}
.Lfixup_post:
	// if r02 == 0 jump .Lfixup_resume (interrupting myself)
	{
		DST_ID = memw(ME+#CONTEXT_r0302)
		if (cmp.eq(DST_ID.new,#0)) jump:t .Lfixup_resume
	}
	// If me==NULL, or if dest parent vm is my vm, or if my parent vm is dest vm, or if dest m is my vm, we can post
	// otherwise, we should fail.
	{
		DST_VM = extractu(DST_ID,#H2K_ID_VMIDX_BITS,#H2K_ID_VMIDX_OFFSET)
		ME_ID = memw(ME+#CONTEXT_id)
		DST_VMBLOCK = add(H2K_GP,#KG_vmblocks)
	}
	{
		ME_VM = extractu(ME_ID,#H2K_ID_VMIDX_BITS,#H2K_ID_VMIDX_OFFSET)
		DST_VMBLOCK = memw(DST_VMBLOCK+DST_VM<<#2)
		p0 = cmp.eq(ME,#0)
		if (p0.new) jump:nt .Lfixup_post_ok
	}
	{
		ME_VMBLOCK_PARENT = memw(VMBLOCK+#VMBLOCK_parent)
		DST_VMBLOCK_PARENT = memw(DST_VMBLOCK+#VMBLOCK_parent)
		p0 = cmp.eq(ME_VM,DST_VM)
		if (p0.new) jump:nt .Lfixup_post_ok
	}
	{
		ME_VMBLOCK_PARENT = extractu(ME_VMBLOCK_PARENT,#H2K_ID_VMIDX_BITS,#H2K_ID_VMIDX_OFFSET)
		DST_VMBLOCK_PARENT = extractu(DST_VMBLOCK_PARENT,#H2K_ID_VMIDX_BITS,#H2K_ID_VMIDX_OFFSET)
	}
	{
		p0 = cmp.eq(ME_VMBLOCK_PARENT,DST_VM)
		if (p0.new) jump:t .Lfixup_post_ok
		p1 = cmp.eq(DST_VMBLOCK_PARENT,ME_VM)
		if (p1.new) jump:t .Lfixup_post_ok
	}
#undef ME_VMBLOCK_PARENT
#undef DST_VMBLOCK_PARENT
#define SIZE r14
.Lfixup_post_fail:
	// r0 = #-1?
	jump .Lintop_fail
.Lfixup_post_ok:
	// Hunt down DST thread
	// If id_to_context fails, we fail
	// Otherwise, we just fixup DST and we're done
	DST_CPU = extractu(DST_ID,#H2K_ID_CPUIDX_BITS,#H2K_ID_CPUIDX_OFFSET)
.Lfixup_cpuidx_to_context:
	{
		MAX_CPUS = memw(DST_VMBLOCK+#VMBLOCK_max_cpus)
		VMBLOCK = DST_VMBLOCK
		INTINFO = memw(DST_VMBLOCK+#VMBLOCK_intinfo)
	}
	{
		p0 = cmp.gt(MAX_CPUS,DST_CPU)
		if (!p0.new) jump:nt .Lfixup_post_fail
		DST = memw(VMBLOCK+#VMBLOCK_contexts)
		SIZE = #CONTEXT_SIZE
	}
	{
		DST += mpyi(DST_CPU,SIZE)
		OPTAB = memw(INTINFO+#INTINFO_handlers)
		jump .Lfixup_resume
	}
#undef ME
#undef VMBLOCK
#undef OP
#undef ARG
#undef ARG_OP
#undef INTINFO
#undef OPTAB
#undef DST
#undef MAX_CPUS
#undef DST_CPU
#undef DST_VMBLOCK
#undef DST_ID
#undef DST_VM
#undef ME_ID
#undef ME_VM
#undef SIZE
FUNC_END H2K_vmtrap_intop

FUNC_START H2K_disable_guest_interrupts .text.vm.int .p2align 2 // disable(me)
#define ME r0
#define WADDR r1
#define TMP r3
	WADDR = add(ME,#CONTEXT_atomic_status_word)
1:
	{
		TMP = memw_locked(WADDR)
	}
	{
		p0 = tstbit(TMP,#H2K_VMSTATUS_IE_BIT)
		TMP = clrbit(TMP,#H2K_VMSTATUS_IE_BIT)
	}
	{
		memw_locked(WADDR,p1) = TMP
	}
	{
		if (!p1) jump 1b
	}
	{
		r0 = mux(p0,#1,#0)
		jumpr r31
	}
#undef TMP
#undef WADDR
#undef ME
FUNC_END H2K_disable_guest_interrupts

FUNC_START H2K_enable_guest_interrupts .text.vm.int .p2align 2 // enable(me)
#define ME r0
#define WADDR r1
#define MASK r2
#define TMP r3
	WADDR = add(ME,#CONTEXT_atomic_status_word)
1:
	{
		TMP = memw_locked(WADDR)
		MASK = #1<<H2K_VMSTATUS_IE_BIT
	}
	{
		p0 = tstbit(TMP,#H2K_VMSTATUS_IE_BIT)
		TMP = or(TMP,MASK)
		if (p0.new) jump:nt .Lalready_enabled
	}
	{
		memw_locked(WADDR,p0) = TMP
	}
	{
		if (!p0) jump 1b
	}
	{
		allocframe(#0)
		call H2K_vm_check_interrupts
	}
	{
		r0 = #0
		dealloc_return
	}
.Lalready_enabled:
	{
		r0 = #1
		jumpr r31
	}
#undef TMP
#undef MASK
#undef WADDR
#undef ME
FUNC_END H2K_enable_guest_interrupts

FUNC_START H2K_vm_check_interrupts .text.vm.int .p2align 2 // check(me)
#define ME r0
#define VMSTATUS r1
#define VMBLOCK r2
#define INTINFO r3
#define HANDLERS r4
#define HANDLER r5
#define ME_SAV r16
#define OP r17
	{
		VMSTATUS = memub(ME+#CONTEXT_vmstatus)
		VMBLOCK = memw(ME+#CONTEXT_vmblock)
	}
	{
		allocframe(#8)
		INTINFO = memw(VMBLOCK+#VMBLOCK_intinfo)
		p0 = tstbit(VMSTATUS,#H2K_VMSTATUS_IE_BIT)
	}
	{
		memd(r29+#0) = r17:16
		HANDLERS = memw(INTINFO+#INTINFO_handlers)
		OP = mux(p0,#INTOP_INDEX_get,#INTOP_INDEX_peek)
	}
	{
		HANDLER = memw(HANDLERS+OP<<#2)
		ME_SAV = ME
	}
	{
		r1:0 = combine(ME,VMBLOCK)
		r3:2 = combine(INTINFO,#0)
		callr HANDLER
	}
#undef HANDLERS
#undef INTINFO
#undef VMBLOCK
#undef VMSTATUS
#undef ME
#undef HANDLER
#define INT_OFFSET_ZERO r3:2
#define INT_OFFSET r3
#define ZERO r2
	{
		p0 = !cmp.eq(r0,#-1)
		p0 = cmp.eq(OP,#INTOP_INDEX_get)
#undef OP
#define INTNO_SAV r17
		INTNO_SAV = r0
		INT_OFFSET_ZERO = combine(#INTERRUPT_GEVB_OFFSET,#0)
	}
	{
		if (p0) r1:0 = combine(INTNO_SAV,ZERO)
		if (p0) r3:2 = combine(ME_SAV,INT_OFFSET)
		if (p0) call H2K_vm_event
	}
	{
		r0 = INTNO_SAV
		r17:16 = memd(r29+#0)
		dealloc_return
	}
#undef ZERO
#undef INT_OFFSET
#undef INT_OFFSET_ZERO
#undef ME_SAV
#undef INTNO_SAV
FUNC_END H2K_vm_check_interrupts

FUNC_START H2K_vm_int_intinfo_init .text.vm.int .p2align 2 // init(vmblock,num_shints)
#define VMBLOCK r0
#define NUM_SHINTS r1
#define HANDLERS_NUM_INTS r3:2
#define INFO r4
	{
		INFO = memw(VMBLOCK+#VMBLOCK_intinfo)
		HANDLERS_NUM_INTS = combine(##H2K_vm_cpuint_ops,#32)
	}
	{
		memd(INFO++#8) = HANDLERS_NUM_INTS
		HANDLERS_NUM_INTS = combine(##H2K_vm_shint_ops,NUM_SHINTS)
	}
	{
		p0 = cmp.gtu(NUM_SHINTS,#0)
		if (p0.new) memd(INFO++#8) = HANDLERS_NUM_INTS
		HANDLERS_NUM_INTS = combine(##H2K_vm_badint_ops,#-1)
	}
	{
		memd(INFO) = HANDLERS_NUM_INTS
		jumpr r31
	}
#undef INFO
#undef HANDLERS_NUM_INTS
#undef NUM_SHINTS
#undef VMBLOCK
FUNC_END H2K_vm_int_intinfo_init

