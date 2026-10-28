/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * H2K_vmop_boot/_status/_free are tested elsewhere (thread/create/test).
 * Here we exercise H2K_vmop_kill_thread and H2K_vmop_kill_vm: permission
 * checks plus the wake-from-any-state machinery, with the cancel/IPI helpers
 * mocked out.
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <context.h>
#include <vm.h>
#include <vmop.h>
#include <vmdefs.h>
#include <hw.h>
#include <globals.h>
#include <h2_common_vm.h>

#define ME_VMIDX     1
#define TARGET_VMIDX 2
#define OTHER_VMIDX  3
#define GONE_VMIDX   5  /* must remain NULL in H2K_kg.vmblocks[] */

#define MAX_CPUS 2

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_kg_t H2K_kg;

H2K_vmblock_t me_block;
H2K_vmblock_t target_block;
H2K_vmblock_t other_block;

H2K_thread_context me_threads[MAX_CPUS];
H2K_thread_context target_threads[MAX_CPUS];
H2K_thread_context other_threads[MAX_CPUS];

u32_t TH_saw_ipi;
u32_t TH_saw_popup_cancel;
u32_t TH_saw_futex_cancel;

s32_t H2K_vm_ipi_send_withlock(H2K_thread_context *dest)
{
	TH_saw_ipi = 1;
	return 0;
}

void H2K_popup_cancel(H2K_thread_context *t)
{
	TH_saw_popup_cancel = 1;
}

void H2K_futex_cancel(H2K_thread_context *t)
{
	TH_saw_futex_cancel = 1;
}

static void TH_init_block(H2K_vmblock_t *blk, u32_t vmidx, u32_t parent_vmidx,
                          H2K_thread_context *ctx)
{
	int i;
	memset(blk, 0, sizeof(*blk));
	blk->vmidx = vmidx;
	blk->parent.vmidx = parent_vmidx;
	blk->max_cpus = MAX_CPUS;
	blk->contexts = ctx;
	for (i = 0; i < MAX_CPUS; i++) {
		memset(&ctx[i], 0, sizeof(ctx[i]));
		ctx[i].id.vmidx = vmidx;
		ctx[i].id.cpuidx = i;
		ctx[i].vmblock = blk;
	}
}

static void TH_reset(H2K_thread_context *t, u32_t status, u8_t vmstatus)
{
	t->status = status;
	t->vmstatus = vmstatus;
	t->next = NULL;
	t->prev = NULL;
	target_block.waiting_cpus = 0;
	TH_saw_ipi = 0;
	TH_saw_popup_cancel = 0;
	TH_saw_futex_cancel = 0;
}

int main()
{
	s32_t ret;
	H2K_thread_context *me;
	H2K_thread_context *target;
	H2K_id_t tid;

	__asm__ __volatile(GLOBAL_REG_STR " = %0" : : "r"(&H2K_kg));

	TH_init_block(&me_block, ME_VMIDX, 0, me_threads);
	TH_init_block(&target_block, TARGET_VMIDX, ME_VMIDX, target_threads);
	TH_init_block(&other_block, OTHER_VMIDX, 0xf, other_threads);
	H2K_kg.vmblocks[ME_VMIDX] = &me_block;
	H2K_kg.vmblocks[TARGET_VMIDX] = &target_block;
	H2K_kg.vmblocks[OTHER_VMIDX] = &other_block;
	/* H2K_kg.vmblocks[GONE_VMIDX] stays NULL */

	me = &me_threads[0];
	target = &target_threads[0];
	tid = target->id;

	puts("permission");

	puts("A");
	if (H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, NULL) != -1) FAIL("me==NULL");

	puts("C");
	/* VM gone (NULL vmblock) -- treat as success */
	{
		H2K_id_t gone;
		gone.raw = 0;
		gone.vmidx = GONE_VMIDX;
		if (H2K_vmop_kill_thread(0, gone.raw, 0,0,0,0, me) != 0) FAIL("vm-gone");
	}

	puts("D");
	/* Unrelated VM (caller is neither target nor target's parent) */
	{
		H2K_id_t oid = other_threads[0].id;
		other_threads[0].vmstatus = 0;
		if (H2K_vmop_kill_thread(0, oid.raw, 0,0,0,0, me) != -1) FAIL("unrelated ret");
		if (other_threads[0].vmstatus & H2K_VMSTATUS_KILL) FAIL("unrelated set bits");
	}

	puts("E");
	/* cpuidx out of range */
	{
		H2K_id_t bad = target->id;
		bad.cpuidx = MAX_CPUS;
		if (H2K_vmop_kill_thread(0, bad.raw, 0,0,0,0, me) != -1) FAIL("oob cpuidx");
	}

	puts("state handling");

	puts("F");
	/* DEAD -> success, no bits set, no helpers */
	TH_reset(target, H2K_STATUS_DEAD, 0);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("DEAD ret");
	if (target->vmstatus & (H2K_VMSTATUS_KILL | H2K_VMSTATUS_VMWORK)) FAIL("DEAD set bits");
	if (TH_saw_ipi || TH_saw_popup_cancel || TH_saw_futex_cancel) FAIL("DEAD helper");
	if (target->status != H2K_STATUS_DEAD) FAIL("DEAD status changed");

	puts("G");
	/* VMWAIT: clear waiting_cpus bit, push to ready */
	TH_reset(target, H2K_STATUS_VMWAIT, 0);
	target_block.waiting_cpus = (1ULL << target->id.cpuidx);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("VMWAIT ret");
	if (!(target->vmstatus & H2K_VMSTATUS_KILL)) FAIL("VMWAIT no KILL");
	if (!(target->vmstatus & H2K_VMSTATUS_VMWORK)) FAIL("VMWAIT no VMWORK");
	if (target->status != H2K_STATUS_READY) FAIL("VMWAIT not READY");
	if (target_block.waiting_cpus & (1ULL << target->id.cpuidx)) FAIL("VMWAIT waiting_cpus");
	if (TH_saw_ipi || TH_saw_popup_cancel || TH_saw_futex_cancel) FAIL("VMWAIT helper");

	puts("H");
	/* RUNNING with IE set -- IPI */
	TH_reset(target, H2K_STATUS_RUNNING, H2K_VMSTATUS_IE);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("RUN/IE ret");
	if (!(target->vmstatus & H2K_VMSTATUS_KILL)) FAIL("RUN/IE no KILL");
	if (TH_saw_ipi != 1) FAIL("RUN/IE no IPI");
	if (TH_saw_popup_cancel || TH_saw_futex_cancel) FAIL("RUN/IE helper");

	puts("H2");
	/* RUNNING with IE clear -- kill ignores IE, IPI still sent */
	TH_reset(target, H2K_STATUS_RUNNING, 0);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("RUN/!IE ret");
	if (TH_saw_ipi != 1) FAIL("RUN/!IE no IPI");

	puts("I");
	/* INTBLOCKED with IE clear -- kill ignores IE, popup_cancel still fires */
	TH_reset(target, H2K_STATUS_INTBLOCKED, 0);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("INTBL ret");
	if (TH_saw_popup_cancel != 1) FAIL("INTBL no popup_cancel");
	if (target->status != H2K_STATUS_READY) FAIL("INTBL not READY");
	if (TH_saw_ipi || TH_saw_futex_cancel) FAIL("INTBL helper");

	puts("J");
	/* BLOCKED with IE clear */
	TH_reset(target, H2K_STATUS_BLOCKED, 0);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("BLK ret");
	if (TH_saw_futex_cancel != 1) FAIL("BLK no futex_cancel");
	if (target->status != H2K_STATUS_READY) FAIL("BLK not READY");
	if (TH_saw_ipi || TH_saw_popup_cancel) FAIL("BLK helper");

	puts("K");
	/* READY -- no helper, but KILL+VMWORK still set */
	TH_reset(target, H2K_STATUS_READY, 0);
	ret = H2K_vmop_kill_thread(0, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("RDY ret");
	if (!(target->vmstatus & H2K_VMSTATUS_KILL)) FAIL("RDY no KILL");
	if (!(target->vmstatus & H2K_VMSTATUS_VMWORK)) FAIL("RDY no VMWORK");
	if (TH_saw_ipi || TH_saw_popup_cancel || TH_saw_futex_cancel) FAIL("RDY helper");
	if (target->status != H2K_STATUS_READY) FAIL("RDY status changed");

	puts("self-kill");

	puts("L");
	/* target == me: permission must pass (caller's vmidx == target's vmidx) */
	TH_reset(me, H2K_STATUS_RUNNING, H2K_VMSTATUS_IE);
	ret = H2K_vmop_kill_thread(0, me->id.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("self ret");
	if (!(me->vmstatus & H2K_VMSTATUS_KILL)) FAIL("self no KILL");
	if (TH_saw_ipi != 1) FAIL("self no IPI");

	puts("kill child");

	puts("M");
	/* NULL me */
	if (H2K_vmop_kill_vm(0, TARGET_VMIDX, 0,0,0,0, NULL) != -1) FAIL("kc me==NULL");

	puts("N");
	/* vm out of range */
	if (H2K_vmop_kill_vm(0, H2K_ID_MAX_VMS, 0,0,0,0, me) != -1) FAIL("kc oob vm");

	puts("O");
	/* VM gone (NULL vmblock) -- treat as success */
	if (H2K_vmop_kill_vm(0, GONE_VMIDX, 0,0,0,0, me) != 0) FAIL("kc vm-gone");

	puts("P");
	/* Caller is a thread of the target VM -- kill proceeds including the
	 * caller; it gets KILL|VMWORK and an IPI, same as all other RUNNING
	 * threads.  The IPI fires on return to userspace. */
	{
		H2K_thread_context *tme = &target_threads[0];
		tme->status = H2K_STATUS_RUNNING;
		tme->vmstatus = 0;
		target_block.status = 0;
		TH_saw_ipi = 0;
		if (H2K_vmop_kill_vm(0, TARGET_VMIDX, 0x42,0,0,0, tme) != 0) FAIL("kc self ret");
		if (!(tme->vmstatus & H2K_VMSTATUS_KILL)) FAIL("kc self: caller no KILL");
		if (TH_saw_ipi != 1) FAIL("kc self: caller no IPI");
		if (target_block.status != 0x42) FAIL("kc self: vmblock->status not stamped");
	}

	puts("Q");
	/* Unrelated caller (neither parent nor target) -- reject */
	{
		H2K_thread_context *oth = &other_threads[0];
		oth->vmstatus = 0;
		if (H2K_vmop_kill_vm(0, TARGET_VMIDX, 0,0,0,0, oth) != -1) FAIL("kc unrelated");
	}

	puts("R");
	/* Mixed states: cpu0 RUNNING (IE clear), cpu1 DEAD.
	 * Expect cpu0 to get KILL+VMWORK + IPI, cpu1 untouched. */
	target_threads[0].status = H2K_STATUS_RUNNING;
	target_threads[0].vmstatus = 0;
	target_threads[1].status = H2K_STATUS_DEAD;
	target_threads[1].vmstatus = 0;
	target_block.waiting_cpus = 0;
	TH_saw_ipi = TH_saw_popup_cancel = TH_saw_futex_cancel = 0;
	ret = H2K_vmop_kill_vm(0, TARGET_VMIDX, 0,0,0,0, me);
	if (ret != 0) FAIL("kc mixed ret");
	if (!(target_threads[0].vmstatus & H2K_VMSTATUS_KILL)) FAIL("kc cpu0 no KILL");
	if (!(target_threads[0].vmstatus & H2K_VMSTATUS_VMWORK)) FAIL("kc cpu0 no VMWORK");
	if (TH_saw_ipi != 1) FAIL("kc cpu0 no IPI");
	if (target_threads[1].vmstatus & (H2K_VMSTATUS_KILL | H2K_VMSTATUS_VMWORK)) FAIL("kc cpu1 (DEAD) touched");
	if (target_threads[1].status != H2K_STATUS_DEAD) FAIL("kc cpu1 status changed");

	puts("S");
	/* All-live: cpu0 BLOCKED, cpu1 VMWAIT.  Both must end up READY with bits set. */
	target_threads[0].status = H2K_STATUS_BLOCKED;
	target_threads[0].vmstatus = 0;
	target_threads[0].next = target_threads[0].prev = NULL;
	target_threads[1].status = H2K_STATUS_VMWAIT;
	target_threads[1].vmstatus = 0;
	target_threads[1].next = target_threads[1].prev = NULL;
	target_block.waiting_cpus = (1ULL << target_threads[1].id.cpuidx);
	TH_saw_ipi = TH_saw_popup_cancel = TH_saw_futex_cancel = 0;
	ret = H2K_vmop_kill_vm(0, TARGET_VMIDX, 0,0,0,0, me);
	if (ret != 0) FAIL("kc all-live ret");
	if (target_threads[0].status != H2K_STATUS_READY) FAIL("kc cpu0 not READY");
	if (target_threads[1].status != H2K_STATUS_READY) FAIL("kc cpu1 not READY");
	if (TH_saw_futex_cancel != 1) FAIL("kc no futex_cancel");
	if (target_block.waiting_cpus & (1ULL << target_threads[1].id.cpuidx)) FAIL("kc waiting_cpus not cleared");

	puts("T");
	/* VMOP_KILL_VM_SELF: caller (cpu0, RUNNING) is now included in the kill
	 * loop -- it gets KILL|VMWORK and an IPI, same as any other RUNNING
	 * thread.  The IPI fires on the first instruction after returning to
	 * userspace, routing through vm_do_work -> thread_stop.  cpu1 BLOCKED
	 * also gets KILL via futex_cancel + ready_append. */
	{
		H2K_thread_context *sme = &me_threads[0];
		me_threads[0].status = H2K_STATUS_RUNNING;
		me_threads[0].vmstatus = 0;
		me_threads[1].status = H2K_STATUS_BLOCKED;
		me_threads[1].vmstatus = 0;
		me_threads[1].next = me_threads[1].prev = NULL;
		me_block.status = 0;
		TH_saw_ipi = TH_saw_futex_cancel = 0;
		ret = H2K_vmop_kill_vm(0, VMOP_KILL_VM_SELF, 0x55,0,0,0, sme);
		if (ret != 0) FAIL("self-sentinel ret");
		if (!(me_threads[0].vmstatus & H2K_VMSTATUS_KILL)) FAIL("self-sentinel: caller (cpu0) no KILL");
		if (!(me_threads[1].vmstatus & H2K_VMSTATUS_KILL)) FAIL("self-sentinel: cpu1 no KILL");
		if (TH_saw_ipi != 1) FAIL("self-sentinel: caller (cpu0) no IPI");
		if (TH_saw_futex_cancel != 1) FAIL("self-sentinel: no futex_cancel for cpu1");
		if (me_block.status != 0x55) FAIL("self-sentinel: vmblock->status not stamped");
	}

	puts("dispatch");

	puts("U");
	/* OOB op codes must return -1 without dispatching */
	if (H2K_vmop((vmop_t)-1,   0,0,0,0,0, me) != -1) FAIL("op=-1 not rejected");
	if (H2K_vmop((vmop_t)100,  0,0,0,0,0, me) != -1) FAIL("op=100 not rejected");
	if (H2K_vmop(VMOP_MAX,     0,0,0,0,0, me) != -1) FAIL("op=VMOP_MAX not rejected");

	puts("V");
	/* Dispatch VMOP_KILL_THREAD through the table: target READY, same result as case K */
	TH_reset(target, H2K_STATUS_READY, 0);
	ret = H2K_vmop(VMOP_KILL_THREAD, tid.raw, 0,0,0,0, me);
	if (ret != 0) FAIL("disp KILL_THREAD ret");
	if (!(target->vmstatus & H2K_VMSTATUS_KILL)) FAIL("disp KILL_THREAD no KILL");
	if (!(target->vmstatus & H2K_VMSTATUS_VMWORK)) FAIL("disp KILL_THREAD no VMWORK");

	puts("W");
	/* Dispatch VMOP_KILL_VM through the table: cpu0 RUNNING, cpu1 DEAD */
	target_threads[0].status = H2K_STATUS_RUNNING;
	target_threads[0].vmstatus = 0;
	target_threads[1].status = H2K_STATUS_DEAD;
	target_threads[1].vmstatus = 0;
	target_block.waiting_cpus = 0;
	target_block.status = 0;
	TH_saw_ipi = TH_saw_popup_cancel = TH_saw_futex_cancel = 0;
	ret = H2K_vmop(VMOP_KILL_VM, TARGET_VMIDX, 0x77, 0,0,0, me);
	if (ret != 0) FAIL("disp KILL_VM ret");
	if (!(target_threads[0].vmstatus & H2K_VMSTATUS_KILL)) FAIL("disp KILL_VM cpu0 no KILL");
	if (TH_saw_ipi != 1) FAIL("disp KILL_VM cpu0 no IPI");
	if (target_threads[1].vmstatus & H2K_VMSTATUS_KILL) FAIL("disp KILL_VM cpu1 (DEAD) touched");
	if (target_block.status != 0x77) FAIL("disp KILL_VM status not stamped");

	puts("TEST PASSED");
	return 0;
}
