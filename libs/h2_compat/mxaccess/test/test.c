/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * HMX Access (mxaccess) unit test.
 *
 * The context limit is seeded from h2_info(INFO_HMX_INSTANCES).
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>
#include <h2_mxaccess.h>

#define HELPER_STACK 256

static unsigned long long int helper_stack[HELPER_STACK];

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

static void acquire_n(h2_mxaccess_state_t *mxacc, int *ret, int n, int limit)
{
	unsigned int seen = 0;
	int i;

	for (i = 0; i < n; i++) {
		ret[i] = h2_mxaccess_acquire(mxacc);
		if (ret[i] < 0 || ret[i] >= limit) {
			FAIL("acquire returned an out-of-range index\n");
		}
		if (seen & (1u << ret[i])) {
			FAIL("acquire returned a duplicate index\n");
		}
		seen |= (1u << ret[i]);
	}
}

static void release_n(h2_mxaccess_state_t *mxacc, int *ret, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (h2_mxaccess_release(mxacc, ret[i]) != 0) {
			FAIL("release failed\n");
		}
	}
}

typedef struct {
	h2_mxaccess_state_t *mxacc;
	h2_sem_t            *ready;
	h2_sem_t            *done;
	int                 *ret;
	int                  limit;
} blocker_args_t;

void blocker(int thread)
{
	blocker_args_t *a = (blocker_args_t *)(void *)(unsigned int)thread;

	h2_sem_up(a->ready);
	acquire_n(a->mxacc, a->ret, 1, a->limit);
	h2_sem_up(a->done);
	h2_thread_stop(0);
}

int main()
{
	h2_mxaccess_state_t mxacc;
	int ret[33];
	int blocker_ret;
	h2_sem_t ready, done;
	blocker_args_t args;
	const h2_coproc_type_t    init_type       = CFG_TYPE_VXU0;
	const h2_coproc_subtype_t init_subtype    = CFG_SUBTYPE_VXU0;
	const h2_cfg_unit_entry   init_entry_type = CFG_HMX_CONTEXTS;
	const unsigned int        init_unit_mask  = 0x1;
	int limit;
	int i;

	/* --- 1. unit_init and derive limit ----------------------------------- */
	if (h2_mxaccess_unit_init(&mxacc, init_type, init_subtype,
							  init_entry_type, init_unit_mask) != 0) {
		FAIL("unit_init failed\n");
	}
	if (mxacc.active != 0) {
		FAIL("unit_init did not clear active\n");
	}
	if (mxacc.type != init_type) {
		FAIL("unit_init set wrong type\n");
	}
	if (mxacc.subtype != init_subtype) {
		FAIL("unit_init set wrong subtype\n");
	}
	if (mxacc.entry_type != init_entry_type) {
		FAIL("unit_init set wrong entry_type\n");
	}
	if (mxacc.unit_mask != init_unit_mask) {
		FAIL("unit_init set wrong unit_mask\n");
	}
	{
		int count = h2_coproc_count(init_type, init_subtype, init_entry_type, init_unit_mask);
		limit = (count < 0) ? h2_info(INFO_HMX_INSTANCES) : count;
	}
	printf("limit %d\n", limit);
	if (limit <= 0 || limit > (int)(sizeof(ret) / sizeof(ret[0]) - 1)) {
		FAIL("seeded limit is out of range\n");
	}
	printf("PASS: unit_init\n");

	/* --- 2. acquire / release / reuse ------------------------------------ */
	acquire_n(&mxacc, ret, limit, limit);
	release_n(&mxacc, ret, limit);

	acquire_n(&mxacc, ret, limit, limit);
	printf("PASS: acquire/release/reuse up to limit=%d\n", limit);

	/* --- 3. limit+1 acquire blocks; wakes when a slot is freed ----------- */
	blocker_ret = -1;
	args.mxacc  = &mxacc;
	args.ready  = &ready;
	args.done   = &done;
	args.ret    = &blocker_ret;
	args.limit  = limit;
	h2_sem_init_val(&ready, 0);
	h2_sem_init_val(&done, 0);

	h2_thread_create(blocker, &helper_stack[HELPER_STACK],
					 (void *)&args, h2_get_prio(pthread_self()));
	h2_sem_down(&ready);

	if (h2_mxaccess_release(&mxacc, ret[0]) != 0) {
		FAIL("release in blocking test failed\n");
	}
	h2_sem_down(&done);

	if (h2_mxaccess_release(&mxacc, blocker_ret) != 0) {
		FAIL("release of blocker slot failed\n");
	}
	for (i = 1; i < limit; i++) {
		if (h2_mxaccess_release(&mxacc, ret[i]) != 0) {
			FAIL("cleanup release failed\n");
		}
	}
	printf("PASS: limit+1 acquire blocked and woke on release\n");

	/* --- 4. Double-release is rejected ---------------------------------- */
	acquire_n(&mxacc, ret, 1, limit);
	if (h2_mxaccess_release(&mxacc, ret[0]) != 0) {
		FAIL("first release failed\n");
	}
	if (h2_mxaccess_release(&mxacc, ret[0]) != -1) {
		FAIL("double-release was not rejected\n");
	}
	for (i = 0; i < limit; i++) {
		if (h2_sem_trydown(&mxacc.sem) != 0) {
			FAIL("semaphore under-counts after double-release guard\n");
		}
	}
	if (h2_sem_trydown(&mxacc.sem) == 0) {
		FAIL("semaphore over-counts -- double-release guard did not work\n");
	}
	for (i = 0; i < limit; i++) {
		h2_sem_up(&mxacc.sem);
	}
	printf("PASS: double-release correctly rejected\n");

	/* --- 5. Legacy wrapper ----------------------------------------------- */
	if (h2_mxaccess_init(&mxacc) != 0) {
		FAIL("legacy h2_mxaccess_init failed\n");
	}
	printf("PASS: legacy mxaccess_init\n");

	printf("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}
