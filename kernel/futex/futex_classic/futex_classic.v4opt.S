/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <futex.h>
#include <max.h>
#include <safemem.h>
#include <vmdefs.h>

/* H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me) */
FUNC_START H2K_futex_wait  .text.core.futex  .p2align  5
	{
		allocframe(#16)
		r17:16 = combine(r1,r0)
		r18 = r2
		r19 = memub(r2+#CONTEXT_hthread)
	}
#define ME r18
#define HTHREAD r19
#define LOCKPTR r16
#define LOCKVAL r17
#define TMP r6
#define MASK r7
	k0lock
	{
		TMP = memub(ME+#CONTEXT_vmstatus)
		r1 = #SAFEMEM_R
		r3:2 = combine(r2,r29)
		MASK = #H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_IE
	}
	{
		p0 = bitsset(TMP,MASK)
		if (p0.new) jump:nt .Lwait_fail_vmwork
	}
	{
		call H2K_safemem_check_and_lock
	}
#undef TMP
#define LOCKPTR r16
#define LOCKVAL r17
#define ME r18
#define HTHREAD r19
#define HASH r2
#define TMP r3
#define ZEROPAIR r9:8
#define ZERO_NEGONE r5:4
#define ZERO r5
#define NEGONE r4
#define RUNLIST r7
#define RUNLIST_PRIOS r6
#define PA r11:10
#define PA_HI r11
#define PA_LO r10
	.falign
	{
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:nt .Lwait_fail
		PA = memd(r29)
		if (!p0.new) TMP = memw(LOCKPTR)
		ZEROPAIR = #0
	}
	tlbunlock
#define RING r0
	{
		p0 = cmp.eq(TMP,LOCKVAL)
		if (!p0.new) jump:nt .Lwait_fail
		ZERO_NEGONE = combine(#0,#-1)
		RING = add(H2K_GP,#KG_futexhash)
		PA = lsr(PA,#2)
	}
	{
		memw(ME+#CONTEXT_futex_ptr_hi) |= PA_HI
		HASH =+mpyi(PA_LO,##FUTEX_PRIME)
	}
	{
		RUNLIST = add(H2K_GP,#KG_runlist)
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
		memd(ME+#CONTEXT_r0100) = ZEROPAIR
		memw(ME+#CONTEXT_futex_ptr_lo) = PA_LO
	}
	{
		r1 = ME
		HASH ^= asl(PA_HI,#32-FUTEX_HASHBITS)
		memb(ME+#CONTEXT_status) = #STATUS_BLOCKED
	}
	{
		memw(RUNLIST+HTHREAD<<#2) = ZERO
		memh(RUNLIST_PRIOS+HTHREAD<<#1) = NEGONE
		RING = tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		call H2K_futex_hash_add_ring
	}
	.falign
	{
		deallocframe
		r1:0 = combine(HTHREAD,ME)
		jump H2K_dosched
	}
.Lwait_fail:
	k0unlock
	{
		r0 = #-1
		dealloc_return
	}
.Lwait_fail_vmwork:
	k0unlock
	{
		r0 = ME
		call H2K_vm_do_work
	}
	{
		r0 = #-1
		dealloc_return
	}
#undef LOCKPTR
#undef LOCKVAL
#undef ME
#undef HTHREAD
#undef PA
#undef PA_LO
#undef PA_HI
#undef TMP
#undef RING
#undef ZEROPAIR
#undef ZERO_NEGONE
#undef ZERO
#undef NEGONE
#undef HASH
#undef RUNLIST
#undef RUNLIST_PRIOS
FUNC_END H2K_futex_wait

/* H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me) */
FUNC_START H2K_futex_resume .text.core.futex .falign
#define ME r19
#define N_TO_WAKE r18
#define PA_LO_SAV r17
#define RING r16
#define PA r11:10
#define PA_LO r10
#define PA_HI r11
#define HASH r5
#define TMP r4
	{
		allocframe(#16)
		r19:18 = combine(r2,r1)
		p0 = cmp.eq(r1,#0)
		if (p0.new) jump:nt .Lresume_null
	}
	{
		r1 = #SAFEMEM_R
		r3:2 = combine(r2,r29)
		memw(r29+#12) = N_TO_WAKE
		call H2K_safemem_check_and_lock
	}
	tlbunlock
	{
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:nt .Lresume_bad
		PA = memd(r29)
	}
	{
		PA = lsr(PA,#2)
	}
	{
		HASH =+mpyi(PA_LO,##FUTEX_PRIME)
		RING = add(H2K_GP,#KG_futexhash)
	}
	{
		HASH ^= asl(PA_HI,#32-FUTEX_HASHBITS)
	}
	{
		RING = tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		PA_LO_SAV = PA_LO
	}
	k0lock
	{
		TMP = memw(RING)
	}
	{
		p0 = cmp.eq(TMP,#0)
		if (p0.new) jump:nt .Lresume_unlock_null
		memw(r29) = TMP
	}
	.falign
.Lresume_next:
	{
		r1:0 = combine(RING,PA_LO_SAV)
		r2 = r29
		call H2K_futex_hash_remove_one
	}
	{
		N_TO_WAKE = add(N_TO_WAKE,#-1)
		p0 = !cmp.eq(N_TO_WAKE,#1)
		p0 = !cmp.eq(r0,#0)
		if (p0.new) jump:t .Lresume_next
	}
	{
		r0 = memw(r29+#12)			// orig n_to_wake
		p0 = cmp.eq(r0,#0)
		if (!p0.new) N_TO_WAKE = add(N_TO_WAKE,#1)	// adjust woken count
	}
	{
		r0 = sub(r0,N_TO_WAKE)			// number woken = orig n_to_wake - cur
		deallocframe
		jump H2K_check_sanity_unlock
	}
.Lresume_unlock_null:
	k0unlock
.Lresume_null:
	{
		r0 = #0
		dealloc_return
	}
.Lresume_bad:
	{
		r0 = #-1
		dealloc_return
	}
#undef ME
#undef N_TO_WAKE
#undef PA
#undef PA_LO
#undef PA_HI
#undef RING
#undef HASH
#undef TMP
FUNC_END H2K_futex_resume

