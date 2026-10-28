#include <asm_offsets.h>
#include <asm_std.h>
#include <vmdefs.h>

/* s32_t H2K_vm_do_work_withlock(H2K_thread_context *me) */
#define ME r0				/* arg0: me */
#define VMSTATUS r2			/* me->vmstatus (byte) */
#define VMBLOCK r5			/* me->vmblock */
#define ATOMIC_STATUS_WORD_ADDR r6	/* &me->atomic_status_word */
#define ATOMIC_STATUS_WORD r4		/* locked copy of me->atomic_status_word */
FUNC_START H2K_vm_do_work_withlock .text.vm.work .p2align 2
	{
		VMSTATUS = memub(ME+#CONTEXT_vmstatus)
		VMBLOCK = memw(ME+#CONTEXT_vmblock)
	}
	/* KILL set: H2K_thread_stop_withlock(me->vmblock->status, me), no return */
	{
		if(!p0.new) r1 = ME
		p0 = bitsclr(VMSTATUS,#H2K_VMSTATUS_KILL)
		if (!p0.new) jump:nt H2K_thread_stop_withlock
		if (!p0.new) r0 = memw(VMBLOCK+#VMBLOCK_status)
	}
	/* no VMWORK pending: return -1 */
	{
		p1 = tstbit(VMSTATUS,#H2K_VMSTATUS_VMWORK_BIT)
		if (p1.new) ATOMIC_STATUS_WORD_ADDR = add(ME,#CONTEXT_atomic_status_word)
		if (!p1.new) r0 = #-0x1
		if (!p1.new) jumpr:nt r31
	}
	/* H2K_atomic_clrbit(&me->atomic_status_word, H2K_VMSTATUS_VMWORK_BIT) */
.L20:
	{
		ATOMIC_STATUS_WORD = memw_locked(ATOMIC_STATUS_WORD_ADDR)
	}
	{
		p1 = tstbit(ATOMIC_STATUS_WORD,#H2K_VMSTATUS_VMWORK_BIT)
		ATOMIC_STATUS_WORD = clrbit(ATOMIC_STATUS_WORD,#H2K_VMSTATUS_VMWORK_BIT)
	}
	{
		memw_locked(ATOMIC_STATUS_WORD_ADDR,p0) = ATOMIC_STATUS_WORD
	}
	{
		if (!p0) jump:nt .L20
	}
	/* return H2K_vm_check_interrupts(me); ME is still in r0 */
	{
		jump H2K_vm_check_interrupts
	}
FUNC_END H2K_vm_do_work_withlock
#undef ME
#undef VMSTATUS
#undef VMBLOCK
#undef ATOMIC_STATUS_WORD_ADDR
#undef ATOMIC_STATUS_WORD
