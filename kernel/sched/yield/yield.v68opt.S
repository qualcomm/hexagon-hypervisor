#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>

#define NEXT 0
#define PREV 4

#define ME r0			/* arg0: current thread */
#define HTHREAD r1		/* me->hthread; H2K_dosched() arg1 */
#define READY r2		/* &H2K_gp->ready[0] */
#define HEAD r2			/* ready[PRIO] */
#define PRIO r3			/* me->prio */

FUNC_START H2K_sched_yield .text.core.yield .p2align 2
	ASM_BKL_LOCK(r2, r3, r4, p0)
	{
		PRIO = memub(ME+#CONTEXT_prio)
		HTHREAD = memub(ME+#CONTEXT_hthread)
		READY = add(H2K_GP,#KG_ready)
	}
	{
		p0 = cmp.gtu(PRIO,#MAX_PRIO)
		if (p0.new) jump:nt .Lret
		if (!p0.new) HEAD = memw(READY+PRIO<<#2)
	}
#undef READY
#undef PRIO

#define TAIL r3			/* head->prev */
#define TAIL_HEAD r3:2

	{
		p0 = cmp.eq(HEAD,#0)
		if (p0.new) jump:nt .Lret
		if (!p0.new) TAIL = memw(HEAD+#PREV)
	}
	/* HEAD is the non-NULL ready-list head established above. */
	{
		memb(ME+#CONTEXT_status) = #STATUS_READY
		memw(HEAD+#PREV) = ME
	}
	{
		memd(ME+#NEXT) = TAIL_HEAD
		memw(TAIL+#NEXT) = ME
		jump H2K_dosched
	}
.Lret:
	ASM_BKL_UNLOCK(r2, r3, r4, p0)
	{
		jumpr r31
	}
FUNC_END H2K_sched_yield

#undef NEXT
#undef PREV
#undef ME
#undef HTHREAD
#undef HEAD
#undef TAIL
#undef TAIL_HEAD
