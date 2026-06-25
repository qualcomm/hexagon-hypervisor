/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <hw.h>
#include <thread.h>
#include <dosched.h>
#include <runlist.h>
#include <readylist.h>
#include <asid.h>
#include <stop.h>
#include <timer.h>
#include <vm.h>
#include <cpuint.h>
#include <id.h>
#include <alloc.h>
#include <futex.h>
#include <intpool.h>
#include <max.h>

void H2K_vmblock_finalize_if_done_locked(H2K_vmblock_t *vmblock)
{
	H2K_thread_context *parent_context;
	H2K_vmblock_t *parent_vmblock;

	if (vmblock->num_cpus != 0 && vmblock->status == 0) return;

	parent_context = H2K_id_to_context(vmblock->parent);
	if (parent_context != NULL
			&& parent_context->status != H2K_STATUS_DEAD) {
		parent_vmblock = parent_context->vmblock;
		H2K_vm_cpuint_post_locked(parent_vmblock, parent_context, H2K_VM_CHILDINT, parent_vmblock->intinfo);
	} else if (vmblock->num_cpus == 0) {
		/* Can't free immediately because H2K_switch reads from *me */
		/* EJP: I think this is OK now if we dosched(NULL,htnum)? */
		H2K_mem_alloc_free(vmblock);
	}
}

/* See stop.h.  The five-step "return a context to its free list" tail shared
 * by reap_one_locked, vm_stop_locked, H2K_thread_stop, and resched's
 * self_reap_locked. */
void H2K_free_context_locked(H2K_vmblock_t *vmblock, H2K_thread_context *ctx)
{
	H2K_asid_table_dec(ctx->ssr_asid);
	H2K_thread_context_clear(ctx);  /* preserves vmblock_id */
	ctx->next = vmblock->free_threads;
	vmblock->free_threads = ctx;
	vmblock->num_cpus--;
}

/* Cancel pending waits and return one context to its vmblock's free list.
 * Caller holds BKL.  Decrements num_cpus on success.
 * Skips H2K_STATUS_RUNNING contexts: those are executing on another HW thread
 * and must self-reap via the exiting-vmblock path in resched. */
static int reap_one_locked(H2K_thread_context *ctx)
{
	H2K_vmblock_t *vmblock = ctx->vmblock;
	u8_t s = ctx->status;

	switch (s) {
	case H2K_STATUS_DEAD:
	case H2K_STATUS_RUNNING:
		return 0;
	case H2K_STATUS_BLOCKED:
		H2K_timer_cancel_withlock(ctx);
		H2K_futex_cancel(ctx);
		break;
	case H2K_STATUS_INTBLOCKED:
		H2K_timer_cancel_withlock(ctx);
		/* TODO: H2K_popup_wait shares INTBLOCKED with H2K_intpool_wait;
		 * H2K_intpool_cancel only handles the latter.  Distinguish before
		 * canceling once popup_wait grows a state bit. */
		H2K_intpool_cancel(ctx);
		break;
	case H2K_STATUS_READY:
		H2K_ready_remove(ctx);
		H2K_timer_cancel_withlock(ctx);
		break;
	case H2K_STATUS_VMWAIT:
		if (ctx->id.cpuidx < (sizeof(long_bitmask_t) * 8))
			vmblock->waiting_cpus &= ~(0x1ULL << ctx->id.cpuidx);
		H2K_timer_cancel_withlock(ctx);
		break;
	default:
		return 0;
	}
	H2K_free_context_locked(vmblock, ctx);
	return 1;
}

/* Tear down the calling thread's entire vmblock: reap every non-DEAD context,
 * IPI any RUNNING peers so they self-reap, and run the parent-signal /
 * vmblock-free finalizer.  Caller holds BKL.  POSIX exit() semantics. */
static void vm_stop_locked(s32_t status, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;
	u32_t i;

	vmblock->exiting = 1;

	/* Per-me cleanup. */
	H2K_timer_cancel_withlock(me);
	H2K_runlist_remove(me);
	H2K_free_context_locked(vmblock, me);
	vmblock->status = status;

	/* Pass 1: reap every non-DEAD, non-RUNNING context immediately. */
	for (i = 0; i < vmblock->max_cpus; i++) {
		reap_one_locked(&vmblock->contexts[i]);
	}
	/* Pass 2: kick any RUNNING contexts via CLUSTER_RESCHED_INT steered to
	 * their HW thread.  Each target enters H2K_resched_cluster, sees
	 * vmblock->exiting and self-reaps. */
	for (i = 0; i < vmblock->max_cpus; i++) {
		H2K_thread_context *ctx = &vmblock->contexts[i];
		if (ctx->status != H2K_STATUS_RUNNING) continue;
		if (ctx == me) continue;
		iassignw(CLUSTER_RESCHED_INT, ~(0x1u << ctx->hthread));
		cluster_resched_int();
	}

	H2K_vmblock_finalize_if_done_locked(vmblock);
}

void H2K_thread_stop(s32_t status, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = me->vmblock;
	u32_t i;

	BKL_LOCK(&H2K_bkl);

	/*
	 * Per-thread cleanup. Main and worker threads share this path:
	 * pthread_exit on main must NOT tear down the VM (POSIX semantics --
	 * workers keep running). VM-wide teardown happens only via
	 * H2K_vm_stop (H2_TRAP_VM_STOP), which exit()/sys_exit() invokes.
	 */
	H2K_timer_cancel_withlock(me);
	H2K_runlist_remove(me);
	H2K_free_context_locked(vmblock, me);
	vmblock->status = status;

	if (!vmblock->exiting && status == 0 && vmblock->num_cpus > 0) {
		/* A thread exited cleanly with siblings remaining (main and
		 * workers alike): keep the conservative all-blocked-with-no-
		 * armed-timer reaper.  A non-zero ctx->timeout means a timer is
		 * queued; that thread will be woken when it fires, so do not reap. */
		int all_blocked = 1;
		for (i = 0; i < vmblock->max_cpus && all_blocked; i++) {
			H2K_thread_context *ctx = &vmblock->contexts[i];
			u8_t s = ctx->status;
			if (s == H2K_STATUS_DEAD)
				continue;
			if ((s == H2K_STATUS_BLOCKED || s == H2K_STATUS_INTBLOCKED) && ctx->timeout == 0)
				continue;
			all_blocked = 0;
		}
		if (all_blocked) {
			for (i = 0; i < vmblock->max_cpus; i++) {
				reap_one_locked(&vmblock->contexts[i]);
			}
		}
	}

	H2K_vmblock_finalize_if_done_locked(vmblock);

	/* If we dosched(NULL,get_hwtnum()) I think we can remove special cases in free() */
	H2K_dosched(NULL,get_hwtnum());
}

void H2K_vm_stop(s32_t status, H2K_thread_context *me)
{
	BKL_LOCK(&H2K_bkl);
	vm_stop_locked(status, me);
	H2K_dosched(NULL, get_hwtnum());
	/* unreachable */
}
