/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <context.h>
#include <hw.h>
#include <popup.h>
#include <stdio.h>
#include <stdlib.h>
#include <checker_kernel_locked.h>
#include <checker_ready.h>
#include <setjmp.h>
#include <globals.h>
#include <vmdefs.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static int TH_saw_do_work;

int H2K_vm_do_work_withlock(H2K_thread_context *me)
{
	TH_saw_do_work = 1;
	return -1;
}

H2K_thread_context *TB_in;

u32_t TB_saw_dosched = 0;

jmp_buf env;
static H2K_thread_context a,b,c;

#if 0
void H2K_dosched(H2K_thread_context *in, int hthread)
{
	if (in != TB_in) FAIL("Unexpected thread passed to dosched");
	if (TB_in && hthread != TB_in->hthread) FAIL("Unexpected hardware thread");
	if (H2K_gp->wait_mask != 0) FAIL("Set bit in wait_mask");
	TB_saw_dosched ++;
	checker_kernel_locked();
	BKL_UNLOCK();
	checker_runlist();
	checker_ready();
	longjmp(env,1);
}

void TH_resched(u32_t unused, H2K_thread_context *me, u32_t hwtnum)
{
	if (setjmp(env) == 0) {
		H2K_resched(unused,me,hwtnum);
	}
}
#endif

static int TH_saw_dosched  = 0;
static int TH_saw_switch  = 0;
static H2K_thread_context *TH_switch_old;
static H2K_thread_context *TH_switch_new;

void H2K_dosched(H2K_thread_context *old, int hthread)
{
	checker_kernel_locked();
	BKL_UNLOCK();
	TH_saw_dosched = 1;
	TH_switch_old = old;
	longjmp(env,0x1234);
}

void H2K_switch(H2K_thread_context *old, H2K_thread_context *new)
{
	checker_kernel_locked();
	BKL_UNLOCK();
	TH_saw_switch = 1;
	TH_switch_old = old;
	TH_switch_new = new;
	longjmp(env,0x1234);
}

/* 
 * Clear out all registered popups
 */
void TH_clear_popups()
{
	int i;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i].raw = 0;
	}
}

/*
 * Configure interrupt i to popup thread 
 */
void TH_set_popup(int i, H2K_thread_context *thread)
{
	if (i >= MAX_INTERRUPTS) FAIL("Harness called invalid interrupt");
	H2K_gp->inthandlers[i].handler = H2K_popup_int;
	H2K_gp->inthandlers[i].param = thread;
}

/*
 * set up flags, and call H2K_popup_wait
 * Return status, or zero if returned from H2K_dosched() or H2K_switch() 
 * via setjmp/longjmp
 */
int TH_call_popup_wait(int i, H2K_thread_context *current)
{
	if (setjmp(env) == 0) {
		return H2K_popup_wait(i,current);
	}
	return 0;
}

/*
 * Check to make sure that thread is waiting on the interrupt
 */
void TH_check_waiting(int i, H2K_thread_context *thread)
{
	if (H2K_gp->inthandlers[i].handler != H2K_popup_int) FAIL("Wrong handler");
	if (H2K_gp->inthandlers[i].param != thread) FAIL("Wrong thread");
	if (thread->status != H2K_STATUS_INTBLOCKED) FAIL("Wrong status");
}

/*
 * Check to make sure that thread is still running and not waiting on interrupt
 */
void TH_check_running(int i, H2K_thread_context *thread)
{
#if 0
	printf("Checking i=%d thread=%p inthandlers[i].handler=%p inthandlers[i].param=%p\n",
		i,thread,H2K_gp->inthandlers[i].handler,H2K_gp->inthandlers[i].param);
#endif
	if (H2K_gp->inthandlers[i].handler != NULL) FAIL("Handler set");
	if (H2K_gp->inthandlers[i].param == thread) FAIL("Handler thread set");
	if (thread->status != H2K_STATUS_RUNNING) FAIL("Wrong status");
}

void TH_check_old(H2K_thread_context *thread)
{
	if (thread == NULL) return;
	if (thread->status != H2K_STATUS_READY) FAIL("preempted wrong status");
	if (!H2K_ready_prio_valid(thread->prio)) FAIL("prio not set");
	if (H2K_gp->ready[thread->prio] != thread) FAIL("preempted not in list");
}

void TH_clear_ready(H2K_thread_context *thread)
{
	if (thread == NULL) return;
	H2K_gp->ready[thread->prio] = NULL;
	H2K_ready_set_prio(thread->prio);
}

/*
 * Generate a popup int call, running thread interrupted on hardware thread hthread, dest context new
 * Note that we setjmp before calling H2K_popup_int, and expect it to longjmp back to us from switch/sched
 */

void TH_popup_int(int i, H2K_thread_context *interrupted, int hthread, H2K_thread_context *new)
{
	TH_saw_dosched = TH_saw_switch = 0;
	TH_switch_old = TH_switch_new = NULL;
	TH_set_popup(i,new);
	if (setjmp(env) == 0) {
		H2K_popup_int(i,interrupted,hthread,new);
		if (new != NULL) FAIL("Expected thread, but popup_int didn't call switch");
	} else {
		if (TH_saw_dosched != 0) FAIL("saw dosched");
		if (TH_saw_switch == 0) FAIL("didn't see switch");
		if (TH_switch_old != interrupted) FAIL("Didn't switch from old thread");
		if (TH_switch_new != new) FAIL("Didn't switch to new thread");
	}
}

/*
 * setup a hardware thread as idle
 */
void TH_set_idle(int hthread)
{
#if CLUSTER_SCHED
	H2K_gp->wait_mask = 1<<hthread;
#endif
}

/*
 * Set up a thread as currently running 
 */
void TH_set_running(int hthread, H2K_thread_context *thread)
{
#if CLUSTER_SCHED
	H2K_gp->wait_mask &= ~(1<<hthread);
#endif
}

/*
 * Check priority mask and what thread is running.
 */
void TH_check_priowait_running(int hthread, H2K_thread_context *thread)
{
#if CLUSTER_SCHED
	if ((1<<(hthread)) & H2K_gp->wait_mask) FAIL("wait_mask still set");
#endif
	if ((get_imask(thread->hthread)) != 0) FAIL("IMASK set for hthread");
}

/*
 * Try to popup from running thread a to new thread b
 */
void TH_popup_try(H2K_thread_context *a, H2K_thread_context *b, int intno)
{
	TH_clear_popups();
	TH_set_popup(intno,b);
	TH_set_running(0,a);
	TH_popup_int(intno,a,0,b);
	TH_check_priowait_running(0,b);
	TH_check_old(a);
	TH_clear_ready(a);
}

/*
 * For each group of priorities, try to popup from running a to popup b @ int k
 */
void TH_popup_check_priorities(H2K_thread_context *a, H2K_thread_context *b)
{
	int i,j,k;
	int oldprio = a->prio;
	TH_clear_popups();
	for (i = 0; i < MAX_PRIOS; i += 32) {
		for (j = 0; j < MAX_PRIOS; j += 32) {
			for (k = 0; k < MAX_INTERRUPTS; k += 32) {
				a->prio = i;
				b->prio = j;
				TH_popup_try(a,b,k);
			}
		}
	}
	a->prio = b->prio = oldprio;
}

u32_t fakeint[0x200];

int main() 
{
	int i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

#if ARCHV >= 4
	H2K_gp->l2_int_base = fakeint;
	H2K_gp->l2_ack_base = fakeint+(0x200/sizeof(u32_t));
#endif
	H2K_readylist_init();
	a.prio = b.prio = c.prio = MAX_PRIOS - 30;

	/* First, test wait */
	a.hthread = 0;
	b.hthread = 0;

	/* Check bad cases */
	if ((TH_call_popup_wait(-1,&a)) >= 0) FAIL("Invalid interrupt didn't fail");
	if ((TH_call_popup_wait(MAX_INTERRUPTS,&a)) >= 0) FAIL("Invalid interrupt didn't fail");
	if ((TH_call_popup_wait(MAX_INTERRUPTS+1,&a)) >= 0) FAIL("Invalid interrupt didn't fail");
	if ((TH_call_popup_wait(L2_CORE_INTERRUPT, &a)) >= 0) FAIL("V4 L2 interrupt registration shouldn't pass");

	TH_popup_check_priorities(&a,&b);

	for (i = 0; i < MAX_INTERRUPTS; i++) {
#if ARCHV >= 4
		if (L2_CORE_INTERRUPT == i) continue;
#endif
		TH_clear_popups();
		if (TH_call_popup_wait(i,&a) != 0) {
			FAIL("Couldn't wait");
		}
		TH_check_waiting(i,&a);
		if (TH_call_popup_wait(i,&b) == 0) {
			FAIL("Set popup for int twice");
		}
		TH_clear_popups();
		TH_check_running(i,&b);
	}

	for (i = 0; i < MAX_INTERRUPTS; i++) {
#if ARCHV >= 4
		if (i == 31) continue;
#endif
		TH_clear_popups();
		TH_set_idle(0);
		TH_popup_int(i,NULL,0,NULL);
		TH_set_running(0,&a);
		TH_popup_int(i,&a,0,NULL);

		TH_clear_popups();
		TH_set_popup(i,&b);
		TH_set_idle(0);
		TH_popup_int(i,NULL,0,&b);
		TH_check_priowait_running(0,&b);

		TH_clear_popups();
		TH_set_popup(i,&b);
		TH_set_running(0,&a);
		TH_popup_int(i,&a,0,&b);
		TH_check_priowait_running(0,&b);
		TH_check_old(&a);
		TH_clear_ready(&a);
	}
	/* Case B: VMWORK alone -- gate must fire and return -1 without INTBLOCKED */
	a.status = H2K_STATUS_RUNNING;
	a.vmstatus = H2K_VMSTATUS_VMWORK;
	TH_saw_do_work = 0;
	if (TH_call_popup_wait(0, &a) >= 0) FAIL("B: popup_wait should fail with VMWORK set");
	if (!TH_saw_do_work) FAIL("B: VMWORK alone must call vm_do_work");
	if (a.status == H2K_STATUS_INTBLOCKED) FAIL("B: must not set INTBLOCKED");
	a.vmstatus = 0;

	/* Case C: VMWORK|KILL -- killed thread racing into popup_wait */
	a.status = H2K_STATUS_RUNNING;
	a.vmstatus = H2K_VMSTATUS_VMWORK | H2K_VMSTATUS_KILL;
	TH_saw_do_work = 0;
	if (TH_call_popup_wait(0, &a) >= 0) FAIL("C: popup_wait should fail with VMWORK|KILL");
	if (!TH_saw_do_work) FAIL("C: VMWORK|KILL must call vm_do_work");
	if (a.status == H2K_STATUS_INTBLOCKED) FAIL("C: must not set INTBLOCKED");
	a.vmstatus = 0;

	/* popup_cancel: thread was waiting on interrupt i; cancel must clear the
	 * handler slot and reset r00 to -1. */
	{
		int i;
		for (i = 0; i < MAX_INTERRUPTS; i++) {
#if ARCHV >= 4
			if (i == L2_CORE_INTERRUPT) continue;
#endif
			TH_clear_popups();
			/* Simulate the state popup_wait leaves: INTBLOCKED, r00=intnum, handler set */
			a.status = H2K_STATUS_INTBLOCKED;
			a.r0100 = i;
			H2K_gp->inthandlers[i].handler = H2K_popup_int;
			H2K_gp->inthandlers[i].param = &a;

			BKL_LOCK();
			H2K_popup_cancel(&a);
			BKL_UNLOCK();

			if (H2K_gp->inthandlers[i].raw != 0) FAIL("cancel: handler not cleared");
			if (a.r00 != (u32_t)-1) FAIL("cancel: r00 not reset");
		}
	}

	puts("TEST PASSED\n");
	return 0;
}
