#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

/*
 * Shared tail of every IPI path (C: iassignw(VM_IPI_INT, ~H2K_gp->mask_for_ipi)):
 *   IPI_MASK    = H2K_gp->mask_for_ipi (after the atomic update)
 *   IASSIGN_VAL = ~IPI_MASK, then masked to the low halfword so the high
 *                 halfword is VM_IPI_INT (0): iassignw takes (intno<<16)|threadmask
 */

/* BKL ticket-lock scratch registers (clobbered by ASM_BKL_LOCK/UNLOCK, with p0) */
#define BKL_TMP0 r6
#define BKL_TMP1 r7
#define BKL_TMP2 r8

/* void H2K_vm_ipi_do(u32_t intno, H2K_thread_context *me, u32_t hthread) */
#define ME r1			/* arg1: me (NULL: no vm work) */
#define HTHREAD_IN r2		/* arg2: hthread */
#define HTHREAD r0		/* hthread, bit to clear in mask_for_ipi; arg0 of H2K_sample */
#define IPI_INTMASK r3		/* VM_IPI_INTMASK, for ciad and swi */
#define IPI_MASK_ADDR r2	/* &H2K_gp->mask_for_ipi */
#define LOCKED r4		/* locked copy of H2K_gp->mask_for_ipi */
FUNC_START H2K_vm_ipi_do .text.vm.ipi .p2align 2
	{
		IPI_INTMASK = #VM_IPI_INTMASK
		HTHREAD = HTHREAD_IN
	}
#undef HTHREAD_IN
	{
		IPI_MASK_ADDR = add(H2K_GP,#KG_mask_for_ipi)
		ciad(IPI_INTMASK)
	}
	/* H2K_atomic_clrbit(&H2K_gp->mask_for_ipi, hthread) */
.Latomic_clrbit_retry:
	{
		LOCKED = memw_locked(IPI_MASK_ADDR)
	}
	{
		LOCKED = clrbit(LOCKED,HTHREAD)
	}
	{
		memw_locked(IPI_MASK_ADDR,p0) = LOCKED
	}
#undef LOCKED
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi */
	{
		if (!p0) jump:nt .Latomic_clrbit_retry
		if (p0) IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
	}
#undef IPI_MASK_ADDR
#define IASSIGN_VAL r2		/* (VM_IPI_INT<<16) | (~mask_for_ipi & 0xffff) */
	{
		IASSIGN_VAL = sub(#-0x1,IPI_MASK)
	}
#undef IPI_MASK
	{
		IASSIGN_VAL = and(IASSIGN_VAL,##0xffff)
	}
	{
		iassignw(IASSIGN_VAL)
	}
#undef IASSIGN_VAL
	/* if (H2K_gp->mask_for_ipi != 0) swi(VM_IPI_INTMASK) */
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi, re-read */
	{
		IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
		if (cmp.eq(IPI_MASK.new,#0x0)) jump:nt .Lcont
	}
#undef IPI_MASK
	{
		swi(IPI_INTMASK)
	}
#undef IPI_INTMASK
.Lcont:
#ifdef DO_PROFILE
	/* H2K_sample(hthread, me): keep me in callee-saved ME_SAVED across the call */
#define ME_SAVED r16
	{
		memw(r29+#-0x10) = r16			// save callee-saved r16
		call H2K_sample
		ME_SAVED = ME
		allocframe(#0x8)
	}
#endif
	/* me == NULL: return; else r0 = me for H2K_vm_do_work_withlock */
	{
#ifdef DO_PROFILE
		p0 = cmp.eq(ME_SAVED,#0x0)
		if (!p0.new) r0 = ME_SAVED
		if (p0.new) r16 = memw(r29+#0x0)	// restore r16
		if (p0.new) dealloc_return:nt
#else
		p0 = cmp.eq(ME,#0x0)
		if (!p0.new) r0 = ME
		if (!p0.new) memw(r29+#-0x10) = r16	// save callee-saved r16
		if (p0.new) jumpr:nt r31
#endif
	}
#undef HTHREAD
#undef ME
#ifdef DO_PROFILE
#undef ME_SAVED
#endif
	ASM_BKL_LOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
	{
#ifndef DO_PROFILE
		allocframe(#0x8)
#endif
		call H2K_vm_do_work_withlock
	}
	ASM_BKL_UNLOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
	{
		r16 = memw(r29+#0x0)			// restore r16
		dealloc_return
	}
FUNC_END H2K_vm_ipi_do

/* s32_t H2K_vm_ipi_send_withlock_mask(u32_t mask) */
#define MASK r0			/* arg0: hthread bits to set in mask_for_ipi */
#define IPI_MASK_ADDR r2	/* &H2K_gp->mask_for_ipi */
#define LOCKED r3		/* locked copy of H2K_gp->mask_for_ipi */
FUNC_START H2K_vm_ipi_send_withlock_mask .text.vm.ipi .p2align 2
	{
		IPI_MASK_ADDR = add(H2K_GP,#KG_mask_for_ipi)
	}
	/* H2K_atomic_or(&H2K_gp->mask_for_ipi, mask) */
.Latomic_or_retry1:
	{
		LOCKED = memw_locked(IPI_MASK_ADDR)
	}
	{
		LOCKED = or(LOCKED,MASK)
	}
	{
		memw_locked(IPI_MASK_ADDR,p0) = LOCKED
	}
#undef LOCKED
#undef MASK
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi */
	{
		if (!p0) jump:nt .Latomic_or_retry1
		if (p0) IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
	}
#undef IPI_MASK_ADDR
#define IASSIGN_VAL r2		/* (VM_IPI_INT<<16) | (~mask_for_ipi & 0xffff) */
	{
		IASSIGN_VAL = sub(#-0x1,IPI_MASK)
	}
#undef IPI_MASK
	{
		IASSIGN_VAL = and(IASSIGN_VAL,##0xffff)
	}
#define IPI_INTMASK r2		/* VM_IPI_INTMASK, for swi */
	{
		iassignw(IASSIGN_VAL)
		IPI_INTMASK = #VM_IPI_INTMASK
	}
#undef IASSIGN_VAL
	{
		swi(IPI_INTMASK)
	}
#undef IPI_INTMASK
	/* return 0 */
	{
		r0 = #0x0
		jumpr r31
	}
FUNC_END H2K_vm_ipi_send_withlock_mask


/* s32_t H2K_vm_ipi_send_withlock(H2K_thread_context *dest) */
#define DEST r0			/* arg0: dest */
#define STATUS r2		/* dest->status */
#define HTHREAD_MASK r5		/* dest->hthread, then 1 << dest->hthread */
#define IPI_MASK_ADDR r3	/* &H2K_gp->mask_for_ipi */
#define LOCKED r4		/* locked copy of H2K_gp->mask_for_ipi */
FUNC_START H2K_vm_ipi_send_withlock .text.vm.ipi .p2align 2
	{
		STATUS = memb(DEST+#CONTEXT_status)
		HTHREAD_MASK = memub(DEST+#CONTEXT_hthread)
		IPI_MASK_ADDR = add(H2K_GP,#KG_mask_for_ipi)
	}
	/* return 0 in both paths; dest not running: nothing to do */
	{
		r0 = #0x0
		p0 = cmp.eq(STATUS,#STATUS_RUNNING)
		if (!p0.new) jumpr:nt r31
		HTHREAD_MASK = lsl(#0x1,HTHREAD_MASK)
	}
#undef DEST
#undef STATUS
	/* inlined H2K_vm_ipi_send_withlock_mask(1 << dest->hthread) */
.Latomic_or_retry2:
	{
		LOCKED = memw_locked(IPI_MASK_ADDR)
	}
	{
		LOCKED = or(LOCKED,HTHREAD_MASK)
	}
	{
		memw_locked(IPI_MASK_ADDR,p0) = LOCKED
	}
#undef LOCKED
#undef HTHREAD_MASK
#undef IPI_MASK_ADDR
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi */
	{
		if (!p0) jump:nt .Latomic_or_retry2
		if (p0) IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
	}
#define IASSIGN_VAL r2		/* (VM_IPI_INT<<16) | (~mask_for_ipi & 0xffff) */
	{
		IASSIGN_VAL = sub(#-0x1,IPI_MASK)
	}
#undef IPI_MASK
	{
		IASSIGN_VAL = and(IASSIGN_VAL,##0xffff)
	}
#define IPI_INTMASK r2		/* VM_IPI_INTMASK, for swi */
	{
		iassignw(IASSIGN_VAL)
		IPI_INTMASK = #VM_IPI_INTMASK
	}
#undef IASSIGN_VAL
	{
		swi(IPI_INTMASK)
	}
#undef IPI_INTMASK
	{
		jumpr r31
	}
FUNC_END H2K_vm_ipi_send_withlock

/* void H2K_vm_ipi_send_mask(u32_t mask) */
#define MASK r0			/* arg0: hthread bits to set in mask_for_ipi */
#define IPI_MASK_ADDR r2	/* &H2K_gp->mask_for_ipi */
#define LOCKED r3		/* locked copy of H2K_gp->mask_for_ipi */
FUNC_START H2K_vm_ipi_send_mask .text.vm.ipi .p2align 2
	{
		IPI_MASK_ADDR = add(H2K_GP,#KG_mask_for_ipi)
	}
	ASM_BKL_LOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
	/* inlined H2K_vm_ipi_send_withlock_mask(mask) */
.Latomic_or_retry3:
	{
		LOCKED = memw_locked(IPI_MASK_ADDR)
	}
	{
		LOCKED = or(LOCKED,MASK)
	}
	{
		memw_locked(IPI_MASK_ADDR,p0) = LOCKED
	}
#undef LOCKED
#undef MASK
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi */
	{
		if (!p0) jump:nt .Latomic_or_retry3
		if (p0) IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
	}
#undef IPI_MASK_ADDR
#define IASSIGN_VAL r2		/* (VM_IPI_INT<<16) | (~mask_for_ipi & 0xffff) */
	{
		IASSIGN_VAL = sub(#-0x1,IPI_MASK)
	}
#undef IPI_MASK
	{
		IASSIGN_VAL = and(IASSIGN_VAL,##0xffff)
	}
#define IPI_INTMASK r2		/* VM_IPI_INTMASK, for swi */
	{
		iassignw(IASSIGN_VAL)
		IPI_INTMASK = #VM_IPI_INTMASK
	}
#undef IASSIGN_VAL
	{
		swi(IPI_INTMASK)
	}
#undef IPI_INTMASK
	ASM_BKL_UNLOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
	{
		jumpr r31
	}
FUNC_END H2K_vm_ipi_send_mask

/* void H2K_vm_ipi_send(H2K_thread_context *dest) */
#define DEST r0			/* arg0: dest */
#define STATUS r2		/* dest->status */
#define HTHREAD_MASK r5		/* dest->hthread, then 1 << dest->hthread */
#define IPI_MASK_ADDR r3	/* &H2K_gp->mask_for_ipi */
#define LOCKED r4		/* locked copy of H2K_gp->mask_for_ipi */
FUNC_START H2K_vm_ipi_send .text.vm.ipi .p2align 2
	{
		STATUS = memb(DEST+#CONTEXT_status)
		HTHREAD_MASK = memub(DEST+#CONTEXT_hthread)
		IPI_MASK_ADDR = add(H2K_GP,#KG_mask_for_ipi)
	}
	/* dest not running: nothing to do */
	{
		r0 = #0x0
		p0 = cmp.eq(STATUS,#STATUS_RUNNING)
		if (!p0.new) jumpr:nt r31
		HTHREAD_MASK = lsl(#0x1,HTHREAD_MASK)
	}
#undef DEST
#undef STATUS
	/* inlined H2K_vm_ipi_send_mask(1 << dest->hthread) */
	ASM_BKL_LOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
.Latomic_or_retry4:
	{
		LOCKED = memw_locked(IPI_MASK_ADDR)
	}
	{
		LOCKED = or(LOCKED,HTHREAD_MASK)
	}
	{
		memw_locked(IPI_MASK_ADDR,p0) = LOCKED
	}
#undef LOCKED
#undef HTHREAD_MASK
#undef IPI_MASK_ADDR
#define IPI_MASK r2		/* H2K_gp->mask_for_ipi */
	{
		if (!p0) jump:nt .Latomic_or_retry4
		if (p0) IPI_MASK = memw(H2K_GP+#KG_mask_for_ipi)
	}
#define IASSIGN_VAL r2		/* (VM_IPI_INT<<16) | (~mask_for_ipi & 0xffff) */
	{
		IASSIGN_VAL = sub(#-0x1,IPI_MASK)
	}
#undef IPI_MASK
	{
		IASSIGN_VAL = and(IASSIGN_VAL,##0xffff)
	}
#define IPI_INTMASK r2		/* VM_IPI_INTMASK, for swi */
	{
		iassignw(IASSIGN_VAL)
		IPI_INTMASK = #VM_IPI_INTMASK
	}
#undef IASSIGN_VAL
	{
		swi(IPI_INTMASK)
	}
#undef IPI_INTMASK
	ASM_BKL_UNLOCK(BKL_TMP0, BKL_TMP1, BKL_TMP2, p0)
	{
		jumpr r31
	}
FUNC_END H2K_vm_ipi_send

#undef BKL_TMP0
#undef BKL_TMP1
#undef BKL_TMP2
