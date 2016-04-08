/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <futex.h>
#include <max.h>
#include <idtype.h>
#include <safemem.h>
#include <vmdefs.h>

/* s32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me) */
FUNC_START H2K_futex_lock_pi .text.core.futex .p2align 5
#define ME_LOCK r19:18
#define ME r19
#define LOCK r18
#define PA_LO_SAV r17
#define DEST r16

// important for cmp+branch
#define PA r7:6
#define PA_HI r7
#define PA_LO r6

#define TMP r5
#define DESTPRIO r4
#define DESTSTATUS r3
#define DEST_HTHREAD r2

#define TMPPAIR r15:14
#define ZERO_NEGONE r13:12
#define ZERO r13
#define NEGONE r12
#define VM r11
#define VCPU r10
#define RUNLIST r9
#define MYID r8
#define RUNLIST_PRIOS r1

	{
		allocframe(#16)
		ME_LOCK = combine(r1,r0)
		DEST = #H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_IE
	}
	k0lock
	{
		TMP = memub(ME+#CONTEXT_vmstatus)
		r1 = #SAFEMEM_RW
		r3:2 = combine(ME,r29)
	}
	{
		p0 = bitsset(TMP,DEST)
		if (p0.new) jump:nt .Llpi_unlock_retfail_vmwork
	}
	{
		PA_LO_SAV = memw(ME+#CONTEXT_continuation)		// read out continuation ptr
		call H2K_safemem_check_and_lock
		DEST = #0
	}
	.falign
	{
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:nt .Llpi_unlock_retfail
		PA = combine(PA_LO_SAV,#0)				// get continuation ptr ready for OR
		TMPPAIR = memd(r29)					// read PA from safemem
		MYID = memw(ME+#CONTEXT_id)
	}
	{
		PA |= lsr(TMPPAIR,#2)					// make PA + continuation
	}
#undef TMPPAIR
#define ZEROPAIR r15:14
	.falign
1:
	{
		TMP = memw_locked(LOCK)
		ZEROPAIR = #0
		ZERO_NEGONE = combine(#0,#-1)
		MYID = or(MYID,#1)
	}
	{
		TMP = or(TMP,#1)
		p0 = cmp.eq(TMP,#0)
		VM = extractu(TMP,#H2K_ID_VMIDX_BITS,#(H2K_ID_VINT_BITS+H2K_ID_CPUIDX_BITS))
		VCPU = extractu(TMP,#H2K_ID_CPUIDX_BITS,#H2K_ID_VINT_BITS)
	}
	{
		memw_locked(LOCK,p2) = TMP
		VM = add(##H2K_kg+KG_vmblocks,asl(VM,#2))
		RUNLIST = add(H2K_GP,#KG_runlist)
	}
	{
		if (!p2) jump 1b
		p1 = and(p2,p0)
		if (p1.new) memw(LOCK) = MYID
		memd(ME+#CONTEXT_futex_ptr_64) = PA		// EJP: This is safe since we also have continuation
	}
	tlbunlock
#undef LOCK
#define PRIO r18
#undef MYID
#define HTHREAD r8
	{
		if (p1) jump .Ldone
		if (!p1) memd(ME+#CONTEXT_r0100) = ZEROPAIR
		VM = memw(VM)
		TMP = #0
	}
	{
		memb(ME+#CONTEXT_status) = #STATUS_BLOCKED
		HTHREAD = memub(ME+#CONTEXT_hthread)
		p0 = cmp.eq(VM,#0)
	}
	{
		if (!p0) TMP = memw(VM+#VMBLOCK_max_cpus)
		if (!p0) DEST = memw(VM+#VMBLOCK_contexts)
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
	}
	{
		DEST += mpyi(VCPU,##CONTEXT_SIZE)
		memw(RUNLIST+HTHREAD<<#2) = ZERO			// not part of raise
		PRIO = memub(ME+#CONTEXT_prio)
	}

	/* BEGIN FUTEX PI RAISE */
	{
		p0 = cmp.gtu(TMP,VCPU)
		if (!p0.new) jump:nt .Lraise_done			// invalid ID
		memh(RUNLIST_PRIOS+HTHREAD<<#1) = NEGONE		// not part of raise
		if (p0.new) DESTPRIO = memub(DEST+#CONTEXT_prio)	// Get dest prio
	}
	{
		p0 = cmp.gt(DESTPRIO,PRIO)
		if (!p0.new) jump:nt .Lraise_done
		DESTSTATUS = memub(DEST+#CONTEXT_status)
		if (p0.new) memb(DEST+#CONTEXT_prio) = PRIO
	}
#undef ZEROPAIR
#undef ZERO_NEGONE
#undef ZERO
#undef VM
#undef VCPU

	{
		p0 = cmp.eq(DESTSTATUS,#STATUS_BLOCKED)
		if (p0.new) jump:nt .Lraise_blocked
		p1 = cmp.eq(DESTSTATUS,#STATUS_READY)
		if (p1.new) jump:nt .Lraise_ready
		p2 = cmp.eq(DESTSTATUS,#STATUS_RUNNING)
		DEST_HTHREAD = memub(DEST+#CONTEXT_hthread)
	}

	{
		if (!p2) jump .Lraise_done
		TMP = memw(H2K_GP+#KG_priomask)
		p3 = DEST_HTHREAD
	}
	{
		TMP = clrbit(TMP,DEST_HTHREAD)
		setimask(p3,NEGONE)
	}
	{
		jump .Lraise_done
		memw(H2K_GP+#KG_priomask) = TMP
		memh(RUNLIST_PRIOS+DEST_HTHREAD<<#1) = DESTPRIO
	}
#undef NEGONE
#undef DESTSTATUS
#undef DEST_HTHREAD
#undef RUNLIST_PRIOS
#define DESTPA r15:14
#define DESTPA_HI r15
#define DESTPA_LO r14
#define HASH r3
#define RING r0
#undef PA
#undef PA_HI
#undef PA_LO
#define PREV r7
#define NEXT r6
#define PREV_NEXT r7:6
	.falign
.Lraise_blocked:
	{
		PREV_NEXT = memd(DEST+#CONTEXT_next)
		DESTPA = memd(DEST+#CONTEXT_futex_ptr_64)
		RING = add(H2K_GP,#KG_futexhash)
	}
	{
		p0 = cmp.eq(NEXT,DEST)
		if (p0.new) jump:nt .Lraise_done
		HASH=mpyi(DESTPA_LO,##FUTEX_PRIME)
		DESTPA_HI = and(DESTPA_HI,#3)
	}
	{
		HASH ^= asl(DESTPA_HI,#32-FUTEX_HASHBITS)
	}
	{
		RING=tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		memw(PREV+#CONTEXT_next) = NEXT
		memw(NEXT+#CONTEXT_prev) = PREV
	}
	{
		TMP = memw(RING)
		memw(r29) = TMP.new
		r1:0 = combine(DEST,RING)
	}
	{
		p0 = cmp.eq(TMP,DEST)
		if (p0.new) memw(RING) = NEXT
		if (p0.new) memw(r29) = NEXT
		call H2K_futex_hash_add_ring
	}
	{
		jump .Lraise_done
	}
#undef HASH
#undef DESTPA
#undef DESTPA_LO
#undef DESTPA_HI
#undef RING

	.falign
.Lraise_ready:
#define READY r15
#define READY_VALIDS r14
#define WIDX_BITNO r13:12
#define WIDX r13
#define BITNO r12
#define FROMRING r3
#define TORING r2
#define FROMHEAD r1
	{
		READY = add(H2K_GP,#KG_ready)
		READY_VALIDS = add(H2K_GP,#KG_ready_valids)
		WIDX_BITNO = bitsplit(DESTPRIO,#5)
		PREV_NEXT = memd(DEST+#CONTEXT_next)
	}
	{
		memw(NEXT+#CONTEXT_prev) = PREV
		memw(PREV+#CONTEXT_next) = NEXT
		p0 = cmp.eq(DEST,NEXT)
		FROMRING = addasl(READY,DESTPRIO,#2)
	}
	{
		FROMHEAD = memw(FROMRING)
		TMP = memw(READY_VALIDS+WIDX<<#2)
		if (p0) NEXT = #0
		TORING = addasl(READY,PRIO,#2)
	}
	{
		TMP = clrbit(TMP,BITNO)
		p1 = cmp.eq(FROMHEAD,DEST)
		if (p1.new) memw(FROMRING) = NEXT
		NEXT = memw(TORING)
	}
	{
		WIDX_BITNO = bitsplit(PRIO,#5)
		if (p0) memw(READY_VALIDS+WIDX<<#2) = TMP
		memw(TORING) = DEST
		p1 = cmp.eq(NEXT,#0)				// is TORING empty?
	}
	{
		TMP = memw(READY_VALIDS+WIDX<<#2)
		if (!p1) PREV = memw(NEXT+#CONTEXT_prev)	// TORING not empty, get PREV
		if (p1) PREV_NEXT = combine(DEST,DEST)		// TORING empty, point to ourselves
	}
	{
		TMP = setbit(TMP,BITNO)
		memw(NEXT+#CONTEXT_prev) = DEST
		memw(PREV+#CONTEXT_next) = DEST
	}
	{
		memw(READY_VALIDS+WIDX<<#2) = TMP
		memd(DEST+#CONTEXT_next) = PREV_NEXT
	}
	/* END FUTEX PI RAISE */
#undef FROMHEAD
#undef TORING
#undef FROMRING
#undef BITNO
#undef WIDX
#undef READY_VALIDS
#undef READY
	.falign
.Lraise_done:
#define FUTEXRING r0
#define HASH r1
#define PA r15:14
#define PA_HI r15
#define PA_LO r14
	/* EJP: maybe we can keep HASH around along with PA_LO_SAV? */
	PA = memd(ME+#CONTEXT_futex_ptr_64)			// reload PA from context
	{
		HASH = mpyi(PA_LO,##FUTEX_PRIME)
		FUTEXRING = add(H2K_GP,#KG_futexhash)
		PA_HI = and(PA_HI,#3)
	}
	{
		HASH ^= asl(PA_HI,#32-FUTEX_HASHBITS)
	}
	{
		r1 = ME
		FUTEXRING = tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		call H2K_futex_hash_add_ring
	}
#undef FUTEXRING
#undef HASH
	.falign
	{
		r1 = memub(ME+#CONTEXT_hthread)
		r0 = ME
		jump H2K_dosched
		deallocframe
	}
.Ldone:
	k0unlock
	{
		r0 = #0
		dealloc_return
	}
.Llpi_unlock_retfail:
	k0unlock
	{
		r0 = #-1
		dealloc_return
	}
.Llpi_unlock_retfail_vmwork:
	k0unlock
	{
		call H2K_vm_do_work
		r0 = ME
	}
	{
		r0 = #-1
		dealloc_return
	}
#undef ME
#undef ME_LOCK
#undef PA
#undef PA_HI
#undef PA_LO
#undef DEST
#undef PREV
#undef NEXT
#undef PREV_NEXT
#undef TMP
#undef RUNLIST
#undef HTHREAD
FUNC_END H2K_futex_lock_pi

/* s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me) */
FUNC_START H2K_futex_unlock_pi .text.core.futex .p2align 5
#define ME_LOCK r19:18
#define ME r19
#define LOCK r18
#define PA_LO_SAV r17
#define HTHREAD r16
#define PA r9:8
#define PA_HI r9
#define PA_LO r8
#define BASEPRIO r7
#define RUNLIST_PRIOS r6
#define RING r5
#define HASH r4
#define TMP r3
	{
		allocframe(#8)
		ME_LOCK = combine(r1,r0)
	}
	k0lock
	{
		r1 = #SAFEMEM_RW
		r3:2 = combine(ME,r29)
		call H2K_safemem_check_and_lock
	}
	{
		p0 = cmp.eq(r0,#0)
		if (p0.new) jump:nt .Llpi_unlock_retfail
		PA = memd(r29)
		RING = add(H2K_GP,#KG_futexhash)
	}
	/* EJP: maybe resched a bit? */
	{
		PA = lsr(PA,#2)
	}
	{
		HASH = mpyi(PA_LO,##FUTEX_PRIME)
		HTHREAD = memub(ME+#CONTEXT_hthread)
		PA_HI = and(PA_HI,#3)
	}
	{
		HASH ^= asl(PA_HI,#32-FUTEX_HASHBITS)
		PA_LO_SAV = PA_LO
	}
	{
		RING = tableidxw(HASH,#FUTEX_HASHBITS,#32-FUTEX_HASHBITS)
		r2 = r29
	}
	{
		TMP = memw(RING)
		memw(r29) = TMP.new
		r1:0 = combine(RING,PA_LO_SAV)
		call H2K_futex_hash_remove_one
	}
	{
		p0 = !cmp.eq(r0,#0)		// Did we find a thread?
		if (p0.new) r0 = memw(r0+#CONTEXT_id)	// Yes, get the ID
		deallocframe
	}
1:
	{
		if (p0) r0 = add(r0,#1)		// if we retry the swap, this will be FALSE
		TMP = memw_locked(LOCK)
	}
	{
		memw_locked(LOCK,p0) = r0
	}
	{
		if (!p0) jump 1b
		BASEPRIO = memub(ME+#CONTEXT_base_prio)
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
	}
	tlbunlock
	{
		memh(RUNLIST_PRIOS+HTHREAD<<#1) = BASEPRIO
		memb(ME+#CONTEXT_prio) = BASEPRIO
		r1:0 = #0
		jump H2K_check_sanity_unlock
	}
#undef TMP
#undef ME
#undef LOCK
#undef PA
#undef HTHREAD
#undef BASEPRIO
#undef RUNLIST_PRIOS
#undef RING
#undef HASH
FUNC_END H2K_futex_unlock_pi

