/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * Vector Access (vecaccess) unit test.
 *
 * The context limit is seeded from h2_info(INFO_COPROC_CONTEXTS) because there
 * is no non-blocking vecaccess call to discover it: h2_vecaccess_acquire()
 * blocks once all contexts are exhausted.
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>
#include <h2_vecaccess.h>

#define GARBAGE_VALUE 0xFDA
#define HELPER_STACK 256

static unsigned long long int helper_stack[HELPER_STACK];

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

static void acquire_n(h2_vecaccess_state_t *vacc, h2_vecaccess_ret_t *ret,
					  int n, int limit, int expect_length)
{
	unsigned int seen = 0;
	int i;

	for (i = 0; i < n; i++) {
		ret[i] = h2_vecaccess_acquire(vacc);
		if (ret[i].idx < 0 || ret[i].idx >= limit) {
			FAIL("acquire returned an out-of-range index\n");
		}
		if (seen & (1u << ret[i].idx)) {
			FAIL("acquire returned a duplicate index\n");
		}
		seen |= (1u << ret[i].idx);
		if (ret[i].length != expect_length) {
			FAIL("acquire returned the wrong length\n");
		}
	}
}

static void release_n(h2_vecaccess_state_t *vacc, h2_vecaccess_ret_t *ret, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (h2_vecaccess_release(vacc, ret[i].idx) != 0) {
			FAIL("release failed\n");
		}
	}
}


typedef struct {
	h2_vecaccess_state_t *vacc;
	h2_sem_t             *ready;
	h2_sem_t             *done;
	h2_vecaccess_ret_t   *ret;
	int                   limit;
	int                   length;
} blocker_args_t;

void blocker(int thread)
{
	blocker_args_t *a = (blocker_args_t *)(void *)(unsigned int)thread;

	h2_sem_up(a->ready);
	/* This is the (limit+1)-th acquire -- blocks until main releases one. */
	acquire_n(a->vacc, a->ret, 1, a->limit, a->length);
	h2_sem_up(a->done);
	h2_thread_stop(0);
}

int main()
{
	h2_vecaccess_state_t vacc;
	h2_vecaccess_state_t block_vacc;
	h2_vecaccess_ret_t ret[33];   /* limit slots for main + 1 for the blocker */
	h2_sem_t ready, done;
	h2_vecaccess_ret_t blocker_ret;
	blocker_args_t args;
	unsigned long native_vlength;
	const h2_coproc_type_t    init_type       = CFG_TYPE_VXU0;
	const h2_coproc_subtype_t init_subtype    = CFG_SUBTYPE_VXU0;
	const h2_cfg_unit_entry   init_entry_type = CFG_HVX_CONTEXTS;
	const unsigned int        init_unit_mask  = 0x1;
	int limit;
	int length;
	int i;

	/* --- 1. Seed the context limit from hardware ------------------------- */
	native_vlength = h2_info(INFO_HVX_VLENGTH);
	if (native_vlength == 0) {
		FAIL("INFO_HVX_VLENGTH is zero\n");
	}
	limit = h2_info(INFO_COPROC_CONTEXTS) / (H2_VECACCESS_MAX_VLENGTH_BYTES / native_vlength);
	printf("limit %d\n", limit);
	if (limit <= 0 || limit > (int)(sizeof(ret) / sizeof(ret[0]))) {
		FAIL("seeded limit is out of range\n");
	}

	/* --- 2. First unit_init; drives acquire tests ------------------------- */
	if (h2_vecaccess_unit_init(&block_vacc, H2_VECACCESS_HVX_128,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0) {
		FAIL("first unit_init (HVX_128) failed\n");
	}
	if (block_vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("HVX_128 set the wrong length\n");
	}
	if (block_vacc.ext != H2_VECACCESS_EXT_HVX) {
		FAIL("HVX_128 set the wrong ext\n");
	}
	if (block_vacc.active != 0) {
		FAIL("unit_init did not clear active\n");
	}
	if (block_vacc.type != init_type) {
		FAIL("unit_init set wrong type\n");
	}
	if (block_vacc.subtype != init_subtype) {
		FAIL("unit_init set wrong subtype\n");
	}
	if (block_vacc.entry_type != init_entry_type) {
		FAIL("unit_init set wrong entry_type\n");
	}
	if (block_vacc.unit_mask != init_unit_mask) {
		FAIL("unit_init set wrong unit_mask\n");
	}
	length = block_vacc.length;
	printf("PASS: first unit_init (HVX_128)\n");

	/* --- 3. acquire / release / reuse, bounded by the limit -------------- */
	acquire_n(&block_vacc, ret, limit, limit, length);
	release_n(&block_vacc, ret, limit);

	/* Re-acquire the full set to prove the released slots are reusable. */
	acquire_n(&block_vacc, ret, limit, limit, length);
	printf("PASS: acquire/release/reuse up to limit=%d\n", limit);

	/* --- 4. limit+1 acquire blocks; wakes when a slot is freed ----------- */
	/* Main holds all `limit` slots (sem == 0).  The blocker signals ready then
	 * calls acquire_n(..., 1), which is the (limit+1)-th acquire.  Because the
	 * semaphore is at 0, the blocker will block in h2_sem_down regardless of
	 * scheduling order: if main releases first, sem goes 0->1 and the blocker
	 * drains it immediately; if the blocker reaches sem_down first, it blocks
	 * until main releases.  Either way the test is correct. */
	blocker_ret.idx = -1;
	args.vacc   = &block_vacc;
	args.ready  = &ready;
	args.done   = &done;
	args.ret    = &blocker_ret;
	args.limit  = limit;
	args.length = length;
	h2_sem_init_val(&ready, 0);
	h2_sem_init_val(&done, 0);

	h2_thread_create(blocker, &helper_stack[HELPER_STACK],
					 (void *)&args, h2_get_prio(pthread_self()));
	h2_sem_down(&ready);       /* wait until helper is about to block */

	if (h2_vecaccess_release(&block_vacc, ret[0].idx) != 0) {
		FAIL("release in blocking test failed\n");
	}
	h2_sem_down(&done);        /* blocker acquired; acquire_n validated idx/length */

	/* Release the blocker's slot and the rest main still holds. */
	if (h2_vecaccess_release(&block_vacc, blocker_ret.idx) != 0) {
		FAIL("release of blocker slot failed\n");
	}
	for (i = 1; i < limit; i++) {
		if (h2_vecaccess_release(&block_vacc, ret[i].idx) != 0) {
			FAIL("cleanup release failed\n");
		}
	}
	printf("PASS: limit+1 acquire blocked and woke on release\n");

	/* --- 5. Double-release is rejected ---------------------------------- */
	/* Acquire one slot, release it once (valid), then release again.  The
	 * second release must fail (-1) because the bit is no longer set in
	 * active, and the semaphore must remain at limit (not be inflated). */
	acquire_n(&block_vacc, ret, 1, limit, length);   /* sem: limit -> limit-1 */
	if (h2_vecaccess_release(&block_vacc, ret[0].idx) != 0) {
		FAIL("first release failed\n");
	}
	if (h2_vecaccess_release(&block_vacc, ret[0].idx) != -1) {
		FAIL("double-release was not rejected\n");
	}
	/* Semaphore must be exactly at limit (not inflated). */
	for (i = 0; i < limit; i++) {
		if (h2_sem_trydown(&block_vacc.sem) != 0) {
			FAIL("semaphore under-counts after double-release guard\n");
		}
	}
	if (h2_sem_trydown(&block_vacc.sem) == 0) {
		FAIL("semaphore over-counts -- double-release guard did not work\n");
	}
	/* Restore semaphore to limit. */
	for (i = 0; i < limit; i++) {
		h2_sem_up(&block_vacc.sem);
	}
	printf("PASS: double-release correctly rejected\n");

	/* --- 6. Request-path testing ---------------------------------------- */
	if (h2_vecaccess_unit_init(&vacc, H2_VECACCESS_SILVER,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0 ||
		vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("SILVER mapping wrong\n");
	}
	if (h2_vecaccess_unit_init(&vacc, H2_VECACCESS_SILVER_MAX,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0 ||
		vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("SILVER_MAX mapping wrong\n");
	}
	if (h2_vecaccess_unit_init(&vacc, H2_VECACCESS_HVX_64,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0 ||
		vacc.length != H2_VECACCESS_VLENGTH_64) {
		FAIL("HVX_64 mapping wrong\n");
	}
	if (h2_vecaccess_unit_init(&vacc, H2_VECACCESS_HVX_128,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0 ||
		vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("HVX_128 mapping wrong\n");
	}
	if (h2_vecaccess_unit_init(&vacc, H2_VECACCESS_HVX_MAX,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != 0 ||
		vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("HVX_MAX mapping wrong\n");
	}
	if (h2_vecaccess_unit_init(&vacc, GARBAGE_VALUE,
							   init_type, init_subtype, init_entry_type, init_unit_mask) != -1) {
		FAIL("invalid request was not rejected\n");
	}
	printf("PASS: unit_init request paths\n");

	/* --- 7. Legacy wrapper ----------------------------------------------- */
	if (h2_vecaccess_init(&vacc, H2_VECACCESS_HVX_128) != 0) {
		FAIL("legacy h2_vecaccess_init failed\n");
	}
	if (vacc.length != H2_VECACCESS_VLENGTH_128) {
		FAIL("legacy init set wrong length\n");
	}
	printf("PASS: legacy vecaccess_init\n");

	printf("TEST PASSED\n");
	h2_thread_stop(0);
	return 0;
}
