/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_error .text.vm.error .p2align 2
	/* TBD: do double exception check as spec'd */
	/* 
	 * XXX: this is rather V3 oriented, we should probably macro 
	 * the guest regs stuff to use guest regs in V4 
	 */
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* r7 = me, r8 = ssr, r9 = htid */
	r3 = r7
	r1 = and(r8, #0xff)

#ifdef DO_EXT_SWITCH
	p0 = cmp.eq(r1, #0x16)  // XE exception?
	if (!p0) jump 1f
	r0 = memw(r7 + #CONTEXT_vmblock)
	r10 = memw(r0 + #VMBLOCK_flags)
	p0 = tstbit(r10, #0)
	if (!p0) jump 1f  // VM using ext?

	jump H2K_ext_context_restore // continues to H2K_context_restore_return
#endif

1:
	r0 = badva
	// r1 = cause
	r2 = #ERROR_GEVB_OFFSET
	// r3 = me
	call H2K_vm_event
	call H2K_preempt
	jump H2K_context_restore_return

	/* EJP: Dead Code! */
	/* Set up KSP and jump to fatal_thread */

//#if ARCHV <= 3
//	r29.h = #hi(H2K_stacks)
//	r29.l = #lo(H2K_stacks)
//	r28 = ssr
//	r28 = extractu(r28,#3,#19)
//	r2 = #KERNEL_STACK_SIZE
//	r29 += mpyi(r28,r2)
//#else
//	r28 = HTID
//	r29 = #KERNEL_STACK_SIZE
//	r29 = add(##H2K_stacks,mpyi(r28,r29))
//#endif
//	r4 = r28
//	r28.h = #hi(H2K_kg)
//	r28.l = #lo(H2K_kg)
//	r0 = #-3
//	r1 = r24
//	r3:2 = #0
//	crswap(r24,sgp)
//	jump H2K_fatal_thread

FUNC_END H2K_handle_error

/*
 * User-specified soft crash. r0 is how to crash, r1 is me.
 */
#define WORK r16
#define ME r17
#define ME_WORK r17:16
FUNC_START H2K_fatal_crash .text.vm.error .p2align 2
	{
		ME_WORK = r1:0
		allocframe(#0)
	}
	{
		p0 = tstbit(WORK,#0)
		if (!p0.new) jump:nt 9f
	}
	{
		memw(H2K_GP+#KG_is_nmi_soft) = #1
	}
	k0lock
	r2 = htid
	r3 = #-1
	r3 = togglebit(r3,r2)
	nmi(r3)
	/* EJP: probably should put stuff from ME into my hthread's crash context here?? */
.Lfatal_crash_hard_join:
.Lmodectl_wait:
	r3 = #0
	r2 = modectl
#if ARCHV >= 5
	r1 = popcount(r3:2)
#else
	r1 = #0
	p0 = tstbit(r2,#0)
	if (p0) r1 = add(r1,#1)
	p0 = tstbit(r2,#1)
	if (p0) r1 = add(r1,#1)
	p0 = tstbit(r2,#2)
	if (p0) r1 = add(r1,#1)
	p0 = tstbit(r2,#3)
	if (p0) r1 = add(r1,#1)
	p0 = tstbit(r2,#4)
	if (p0) r1 = add(r1,#1)
	p0 = tstbit(r2,#5)
	if (p0) r1 = add(r1,#1)
#endif
	{
		p0 = cmp.eq(r1,#1)
		if (!p0.new) jump:nt .Lmodectl_wait
	}
	r1 = syscfg
	r1 = clrbit(r1,#4)		// clear GIE
	syscfg = r1
	// We should now be in "Single Thread Mode": only one thread active, and that's the one executing here
	// Did we possibly nmi a thread holding a lock? We can fix it!
	k0unlock
	tlbunlock
	// Bit 1: Save off TLB and flush caches
	// EJP: for now, let's do this all the time.
9:
	{
		p0 = tstbit(WORK,#1)
		//if (!p0.new) jump:nt 9f
	}

#ifdef CRASH_DEBUG
	// Save off TLB. FIXME: could be done in C...
	r3:2 = combine(#0,##H2K_kg+KG_crash_tlb)
	loop0(1f,#MAX_TLB_ENTRIES)
1:
	r5:4 = tlbr(r3)
	{
		memd(r2++#8) = r5:4
		r3 = add(r3,#1)
	}:endloop0
	nop

	call H2K_intconfig_l2vic_crash
	call H2K_tcm_crash_copy
#endif

	call H2K_cache_d_cleaninv
	call H2K_cache_l2_cleaninv
	syncht
9:
	// Bit 2: Die
	{
		p0 = tstbit(WORK,#2)
		if (!p0.new) jump:nt 9f
	}
	syncht
	nop
	stop(r0)
	nop
9:
	dealloc_return
FUNC_END H2K_fatal_crash

/*
 * Crash!
 * 
 * This is an interesting place to be.  NMI could be from internal or external.
 * 
 * Serialization: k0lock and tlblock could have been interrupted, so might be bogus.
 *  ... so what good trick could we do?
 * We want to preserve as much state as possible, but we don't know much about the 
 * registers... including, whether sgp0 has a thread context or something invalid
 * because it might have been swapped and NMI'd in the middle of a context switch 
 * 
 * 0) Save registers as carefully as possible to crash area. will be reloaded by t32
 * 1) Single thread mode
 * 2) Save TLB entries
 * 3) Clean caches
 * 4) invoke SW routine.  reuse fastint context?
 */
FUNC_START H2K_handle_nmi .text.vm.error .p2align 2
#ifdef NMI_STOP
	r1=##0xe0f0beef
	stop(r0)
#else

#ifdef CRASH_DEBUG
	// Part 0: save off context. Kill cs1:0 registers, hopefully they're not important
	c13:12 = r1:0 // CS1:0 registers
.Lhandle_nmi_soft_join:
	r0 = ##H2K_kg+KG_crash_contexts
	r1 = htid
	r0 += mpyi(r1,##CONTEXT_SIZE)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	// r0 and sgp0 are swapped
	// H2K_kg in r28
	// SP set up
	// lc0/sa0 saved
	// usr/p30 saved
	// ccrssr saved
	// stuff from thread_switch not saved... 
	// r0/sgp0 swapped, context r0100 doesn't have the right values.
	{
		r3:2 = c13:12
	}
	{
		memd(r7+#CONTEXT_r0100) = r3:2
		memd(r7+#CONTEXT_r2322) = r23:22
		r5:4 = c7:6	// m1:0
	}
	{
		memd(r7+#CONTEXT_m1m0) = r5:4
		memd(r7+#CONTEXT_r2120) = r21:20
		r3:2 = c11:10
	}
	{
		memd(r7+#CONTEXT_gpugp) = r3:2
		memd(r7+#CONTEXT_r2524) = r25:24
		r5:4 = g1:0
	}
	{
		memd(r7+#CONTEXT_r2726) = r27:26
		memd(r7+#CONTEXT_gssr_gelr) = r5:4
		r3:2 = g3:2
	}
	{
		memd(r7+#CONTEXT_gbadva_gosp) = r3:2
		r1 = s1
	}
	{
		r6 = ELR
	}
	{
		memw(r7+#CONTEXT_elr) = r6
		memd(r7+#CONTEXT_cs1cs0) = r1:0	// save sgp1:0 here
	}
	// EJP: do we want to save out global registers?
	// EJP: If we have a soft NMI, 
	r0 = memw(##H2K_kg+KG_is_nmi_soft)
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:t .Lstop
	}
	// Hard NMI, all threads are getting it.  Only T0 will continue
	r0 = HTID
	{
		p0 = cmp.eq(r0,#0)
		if (!p0.new) jump:nt .Lstop
	}
	// Now we can join the common code for H2K_fatal_crash
	// But we need to make sure that all steps are going to be taken.
	ME_WORK = combine(#0,#-1)
	jump .Lfatal_crash_hard_join
	// All threads but one (unknown: which one...) should be getting NMI and shutting down.
	// Wait for them here.

.Lstop:
	stop(r0)
	jump .Lstop

#else
crswap(r0,sgp)
memd(r0+#CONTEXT_r3130) = r31:30
call H2K_context_save
r0 = #-3
r1 = r7
r3:2 = #0
r4 = r9
jump H2K_fatal_thread
#endif

#endif
FUNC_END H2K_handle_nmi

FUNC_START H2K_handle_rsvd .text.vm.error .p2align 2
	jump H2K_handle_nmi
FUNC_END H2K_handle_rsvd

FUNC_START H2K_handle_debug .text.vm.error .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	r0 = #0
	r1 = and(r8,#0xff)
	r2 = #DEBUG_GEVB_OFFSET
	r3 = r7
	jump H2K_vm_event
FUNC_END H2K_handle_debug

