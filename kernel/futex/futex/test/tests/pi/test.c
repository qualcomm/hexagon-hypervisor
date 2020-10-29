/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <h2.h>
#include <h2_vm.h>
#include <max.h>
#include <stdio.h>
#include <tlbfmt.h>
#include <tlbmisc.h>
#include <hw.h>

/*
 * This test checks the following functionality:
 * Futex_pass_pi clears lock on no waiters
 * Futex_pass_pi sets lock on no waiters
 * One low priority thread gets mutex
 * Launch 8 medium priority thread
 * Launch incrementally higher proirity threads
 * When priority > medium priority thread blocks, holder thread should run
 * When holder releases, should stop execution
 * Shutdown medium priority threads
 * Other blocking threads should resume in priority order
 * 
 */

#define MAX_DUMMY_THREADS 32
#define MAX_TEST_THREADS  64
#define THREAD_STACK_SIZE 256
#define LOCK_PAGES 2
#define NUM_TOTAL_THREADS (MAX_TEST_THREADS+1)

u64_t			stack_space[MAX_TEST_THREADS][THREAD_STACK_SIZE];

h2_sem_t startup_sem;

int TH_expected_futex_wakeup_thread;
int TH_expected_futex_wakeup_val;

#define FUTEX_PASS 0xcafe
#define FUTEX_EMPTY 0xbabe

volatile int spinner_arr[MAX_TEST_THREADS];

int thread_ids[MAX_TEST_THREADS];

int futex0;
int futex1;

volatile int TH_nextstep_id = 0;
volatile int TH_shutdown_now = 0;
volatile int TH_done = 0;
volatile int TH_caller_woke = 0;

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static void wait(unsigned int time)
{
	int i;
	for (i = 0; i < time; i++) {
		asm volatile (" nop ");
	}
}

static int still_spinning(volatile int *counter)
{
	int test0,test1;
	test0 = *counter;
	wait(1000);
	test1 = *counter;
	return (test0 != test1);
}

void spinner_thread(void * v_threadid)
{
	volatile int *counter;
	int threadid = (int)(v_threadid);
	int myid = h2_thread_myid();
	info("spinner thread %d is H2 TID 0x%x\n",threadid,myid);
	counter = (volatile int *)(&spinner_arr[threadid]);
	h2_sem_up(&startup_sem);
	while (1) {
		while ((TH_nextstep_id != myid) && !TH_done) {
			(*counter)++;
			h2_yield();
		}
		TH_nextstep_id = 0;
		if (TH_shutdown_now) {
			h2_thread_stop(0);
		}
		h2_futex_unlock_pi(&futex0);
	}
	h2_thread_stop(0);
}

typedef struct {
	int threadid;
	h2_sem_t sem;
} blocker_thread_struct;

void blocker_thread(blocker_thread_struct *info)
{
	int myid = h2_thread_myid();
	volatile int *counter;

	info("blocker thread %d is H2 TID 0x%x\n", info->threadid, myid);
	counter = (volatile int *)(&spinner_arr[info->threadid]);
	h2_sem_up(&startup_sem);
	h2_sem_down(&info->sem);
	while ((TH_nextstep_id != myid) && !TH_done) {
		(*counter)++;
		h2_yield();
	}
	TH_nextstep_id = 0;
	h2_thread_stop(0);
}

void pi_caller(int *futex_addr)
{
	int myid = h2_thread_myid();
	int ret;

	info("pi_caller thread is H2 TID 0x%x\n", myid);
	h2_sem_up(&startup_sem);
	ret = h2_futex_lock_pi(futex_addr);
	if (ret != 0) {
		info("Ret: 0x%x\n",ret);
		FAIL("pi caller fails");
	}
	if (myid != TH_expected_futex_wakeup_thread) FAIL("Wrong thread woken");
	if ((*futex_addr & -32) != myid) {
		printf("*futex_addr: %x myid: %x\n",*futex_addr,myid);
		FAIL("Wrong futex val");
	}
	TH_caller_woke = 1;
	h2_thread_stop(0);
}

void spawn_spinner(int tnum, int prio)
{
	thread_ids[tnum] = h2_thread_create(spinner_thread,&stack_space[tnum][THREAD_STACK_SIZE],(void *)tnum,prio);
	if (thread_ids[tnum] == -1) FAIL("Couldn't create spinner");
	h2_sem_down(&startup_sem);
}

void spawn_blocker(int tnum, int prio, blocker_thread_struct *info)
{
	thread_ids[tnum] = h2_thread_create(blocker_thread,&stack_space[tnum][THREAD_STACK_SIZE],info,prio);
	if (thread_ids[tnum] == -1) FAIL("Couldn't create blocker");
	h2_sem_down(&startup_sem);
}

void spawn_pi_caller(int tnum, int prio, int *futex_addr)
{
	thread_ids[tnum] = h2_thread_create(pi_caller,&stack_space[tnum][THREAD_STACK_SIZE],futex_addr,prio);
	if (thread_ids[tnum] == -1) FAIL("Couldn't create pi_caller");
	h2_sem_down(&startup_sem);
}

int main(int argc, char **argv)
{
	h2_handle_errors(0);
	futex1 = futex0 = h2_thread_myid();

	h2_sem_init_val(&startup_sem,0);
	blocker_thread_struct blocker;
	blocker_thread_struct blocker2;
	h2_sem_init_val(&blocker.sem,0);
	h2_sem_init_val(&blocker2.sem,0);
	info("main() starting\n");

	/* Start up two low priority spinner threads */
	spawn_spinner(0,31);
	spawn_spinner(1,30);
	/* Start up a blocking spinner */
	blocker.threadid = 2;
	spawn_blocker(2,29,&blocker);
	blocker2.threadid = 3;
	spawn_blocker(3,28,&blocker2);

	/* Check spin status */
	if (!still_spinning(&spinner_arr[0])) FAIL("t0 not spinning");
	if (!still_spinning(&spinner_arr[1])) FAIL("t1 not spinning");
	if (still_spinning(&spinner_arr[2])) FAIL("t2 spinning");
	if (still_spinning(&spinner_arr[3])) FAIL("t3 spinning");

	/* Priority inherit blocked thread */
	futex0 = thread_ids[2];
	spawn_pi_caller(4,4,&futex0);

	/* Priority inherit running thread */
	futex1 = thread_ids[1];
	spawn_pi_caller(5,5,&futex1);

	/* Generate middlish spinners */

	spawn_spinner(8,16);
	spawn_spinner(9,16);
	spawn_spinner(10,16);
	spawn_spinner(11,16);
	spawn_spinner(12,16);
	spawn_spinner(13,16);
	spawn_spinner(14,16);
	spawn_spinner(15,16);

	info("Middle Spinners Launched\n");

	/* Check that previous priority inheritance worked */
	
	if (still_spinning(&spinner_arr[0])) FAIL("t0 spinning");
	if (!still_spinning(&spinner_arr[1])) FAIL("t1 not spinning");
	if (still_spinning(&spinner_arr[2])) FAIL("t2 spinning");
	if (still_spinning(&spinner_arr[3])) FAIL("t3 spinning");

	info("Running thread inherited OK, shutting down that test thread\n");

	TH_shutdown_now = 1;
	TH_nextstep_id = thread_ids[1];
	while (TH_nextstep_id != 0) /* SPIN */;

	h2_sem_up(&blocker.sem);
	wait(1000);
	if (!still_spinning(&spinner_arr[2])) FAIL("t2 not spinning");
	if (still_spinning(&spinner_arr[3])) FAIL("t3 spinning");
	

	info("Blocked thread inherited OK, shutting down that test thread\n");
	/* EJP: Add extra test case: additional blocked thread in same bin */

	TH_shutdown_now = 1;
	TH_nextstep_id = thread_ids[2];
	while (TH_nextstep_id != 0) /* SPIN */;

	/* priority inherit ready thread */
	futex0 = thread_ids[0];
	spawn_pi_caller(6,6,&futex0);
	wait(1000);

	if (!still_spinning(&spinner_arr[0])) FAIL("t0 not spinning");

	info("Waiting thread inherited OK, going to next step (handoff)\n");

	TH_shutdown_now = 0;
	TH_expected_futex_wakeup_thread = thread_ids[4];
	TH_expected_futex_wakeup_val = FUTEX_PASS;

	TH_nextstep_id = thread_ids[0];
	while (TH_nextstep_id != 0) /* SPIN */;

	while (TH_caller_woke == 0) /* SPIN */;
	TH_caller_woke = 0;

	wait(1000);

	if (still_spinning(&spinner_arr[0])) FAIL("t0 prio not restored");
	info("Handoff Successful, priority restored\n");

	TH_expected_futex_wakeup_thread = thread_ids[6];
	TH_expected_futex_wakeup_val = FUTEX_PASS;
	h2_futex_unlock_pi(&futex0);
	while (TH_caller_woke == 0) /* SPIN */;
	TH_caller_woke = 0;

	TH_expected_futex_wakeup_thread = thread_ids[6];
	TH_expected_futex_wakeup_val = FUTEX_EMPTY;
	h2_futex_unlock_pi(&futex0);
	if (futex0 != 0) FAIL("Futex should be empty");

	TH_expected_futex_wakeup_thread = thread_ids[5];
	TH_expected_futex_wakeup_val = FUTEX_PASS;
	h2_futex_unlock_pi(&futex1);
	while (TH_caller_woke == 0) /* SPIN */;
	TH_caller_woke = 0;
	if ((futex1 & -2) != TH_expected_futex_wakeup_thread) FAIL("Futex should PASS");

	h2_sem_up(&blocker2.sem);
	TH_shutdown_now = 1;
	TH_done = 1;
	puts("TEST PASSED");
	exit(0);
}

