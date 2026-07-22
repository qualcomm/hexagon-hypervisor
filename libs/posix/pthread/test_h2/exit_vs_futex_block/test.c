/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Race reproducer: exit() (whole-VM teardown) vs a peer blocking on a futex.
 *
 * Thread A calls exit(0), which traps into H2K_vm_stop / vm_stop_locked: it
 * grabs the BKL, sets vmblock->exiting = 1, reaps every non-RUNNING peer, then
 * IPIs the RUNNING peers (CLUSTER_RESCHED_INT) so they self-reap. Thread B is a
 * RUNNING peer racing into H2K_futex_wait, where it will grab the BKL, remove
 * itself from the runlist and commit to H2K_STATUS_BLOCKED.
 *
 * The window this exercises: A's reap scan runs while B is still RUNNING, so
 * reap_one_locked skips B (the RUNNING case is a no-op). B only commits to
 * BLOCKED *after* A drops the BKL. The fix makes that safe two ways, both new
 * on the kernel_thread_stop_reap_blocked branch:
 *   1. A's finalize (H2K_vmblock_finalize_if_done_locked) early-returns while
 *      num_cpus != 0 -- B's un-reaped slot keeps num_cpus == 1, so the vmblock
 *      is NOT freed out from under B while it is still about to touch it.
 *   2. B is subsequently reaped via the vmblock->exiting gate in resched():
 *      reached either when B voluntarily blocks (H2K_dosched -> resched) or via
 *      A's Pass-2 CLUSTER_RESCHED_INT. resched sees exiting, self-reaps B, and
 *      finalize then completes teardown (num_cpus reaches 0).
 * Without this branch's teardown machinery exit() had no whole-VM reaper at
 * all: a peer that blocked on a futex during another thread's exit was stranded
 * as a permanently BLOCKED context on a VM that was supposed to be gone (a
 * leaked/missed-wakeup thread), and freeing the vmblock while B was mid-commit
 * would have been a use-after-free -- the hazard the num_cpus guard prevents.
 *
 * A signed spin count is a RUNTIME parameter (argv[1]); its SIGN selects which
 * racing thread absorbs the delay, sliding one side's timing across the other's
 * teardown so the reproducer can land inside the race window:
 *   argv[1] >= 0 : delay the BLOCKER by |argv[1]| spins (exiter tears the VM
 *                  down immediately) so B commits to the futex wait late,
 *                  inside the teardown window.
 *   argv[1] <  0 : delay the EXITER by |argv[1]| spins (blocker races into its
 *                  futex wait immediately) so the VM teardown lands after B is
 *                  already BLOCKED.
 * It falls back to 0 spins on both sides when no argument is supplied, so the
 * test also runs under the plain harness invocation. On v81/opt the tightest
 * race (A and B acquiring the BKL within one spin iteration of each other) sits
 * near blocker_spins == 653: the exiter's puts("TEST PASSED") gives B a large
 * head start, so it takes ~653 spins for A's teardown to catch B still RUNNING.
 *
 * On a correct kernel the VM tears down cleanly, A prints TEST PASSED and exits
 * 0. On the buggy kernel the teardown of the blocking/blocked peer crashes or
 * hangs, so TEST PASSED is never printed and the exit status is non-zero.
 */

#include <stdio.h>
#include <stdlib.h>
#include <h2.h>
#include <h2_futex.h>
#include <pthread.h>

static volatile int futex_word;	/* stays 0 so h2_futex_wait blocks */
static h2_sem_t started;
static h2_sem_t go;
static h2_sem_t main_park;	/* never posted */

/* Spin counts split by sign of argv[1]; only one side is delayed at a time.
 * Default is 0 spins on both sides -- no artificial delay. */
static volatile int blocker_spins = 0;
static volatile int exiter_spins = 0;

static void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

/*
 * Thread B -- the "waiting for BLK" side. Parks until released, spins the
 * parameterized amount, then blocks on the futex and never gets woken.
 */
static void *blocker(void *arg)
{
	int i;

	(void)arg;
	h2_sem_up(&started);
	h2_sem_down(&go);
	for (i = 0; i < blocker_spins; i++) asm volatile ("nop");
	/* Blocks: futex_word == 0 matches the expected value. Never woken. */
	h2_futex_wait((void *)&futex_word, 0);
	FAIL("blocker returned from h2_futex_wait");
	return NULL;
}

/*
 * Thread A -- the teardown side. Releases B, then exits the whole VM while B is
 * racing into its futex block.
 */
static void *exiter(void *arg)
{
	int i;

	(void)arg;
	h2_sem_up(&go);
	for (i = 0; i < exiter_spins; i++) asm volatile ("nop");
	puts("TEST PASSED");
	exit(0);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t t_b, t_a;

	if (argc > 1) {
		long d = strtol(argv[1], NULL, 0);
		if (d >= 0) {
			blocker_spins = (int)d;
			exiter_spins = 0;
		} else {
			exiter_spins = (int)(-d);
			blocker_spins = 0;
		}
	}

	h2_sem_init_val(&started, 0);
	h2_sem_init_val(&go, 0);
	h2_sem_init_val(&main_park, 0);
	h2_handle_errors(1);
	printf("Starting exit_vs_futex_block blocker_spins=%d exiter_spins=%d\n",
	       blocker_spins, exiter_spins);

	if (pthread_create(&t_b, NULL, blocker, NULL) != 0) FAIL("blocker create");
	h2_sem_down(&started);

	if (pthread_create(&t_a, NULL, exiter, NULL) != 0) FAIL("exiter create");

	/* Main is not an actor in the race; park so the two peers drive it. */
	h2_sem_down(&main_park);
	FAIL("main unparked");
	return 0;
}
