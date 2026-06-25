/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STOP_H
#define H2K_STOP_H 1

#include <context.h>
#include <vmblock.h>

void H2K_thread_stop(s32_t status, H2K_thread_context *me) __attribute((noreturn)) IN_SECTION(".text.misc.stop");

/* POSIX exit() / process termination from any thread.  Tears down the entire
 * vmblock: reaps every non-DEAD context (including blocked, ready, vmwait,
 * and via IPI any RUNNING peers), then signals the parent VM. */
void H2K_vm_stop(s32_t status, H2K_thread_context *me) __attribute((noreturn)) IN_SECTION(".text.misc.stop");

/* Signal the parent VM (if any) and free the vmblock if num_cpus has reached
 * zero.  Caller must hold BKL.  Used by both thread_stop and the resched
 * self-reap path to avoid leaking the vmblock when the last context exits. */
void H2K_vmblock_finalize_if_done_locked(H2K_vmblock_t *vmblock) IN_SECTION(".text.misc.stop");

/* Common teardown tail: drop the context's ASID ref, clear it (preserving
 * vmblock_id), push it onto its vmblock's free list, and decrement num_cpus.
 * Caller holds BKL and is responsible for the path-specific cancel/remove
 * steps (timer/futex/intpool/ready/runlist) BEFORE calling this. */
void H2K_free_context_locked(H2K_vmblock_t *vmblock,
			     H2K_thread_context *ctx) IN_SECTION(".text.misc.stop");

#endif

