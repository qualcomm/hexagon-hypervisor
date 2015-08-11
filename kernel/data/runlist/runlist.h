/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RUNLIST_H
#define RUNLIST_H 1

#include <context.h>
#include <max.h>
#include <hexagon_protos.h>
#include <globals.h>

static inline void H2K_runlist_push(H2K_thread_context *newthread)
{
	u32_t hthread = newthread->hthread;
	u32_t prio = newthread->prio;
	newthread->status = H2K_STATUS_RUNNING;
	H2K_gp->runlist[hthread] = newthread;
	H2K_gp->runlist_prios[hthread] = prio;
}

static inline u32_t H2K_runlist_worst_prio()
{
	s32_t worst_prio = -1;
	s32_t hthread = -1;
	s32_t i;
	for (i = 0; i < H2K_gp->hthreads; i++) {
		if (H2K_gp->runlist_prios[i] IS_WORSE_THAN worst_prio) {
			worst_prio = H2K_gp->runlist_prios[i];
			hthread = i;
		}
	}
	return hthread == -1 ? MAX_PRIOS : worst_prio;
}

static inline u32_t H2K_runlist_worst_prio_hthread()
{
	s32_t worst_prio = -1;
	s32_t hthread = -1;
	s32_t i;
	for (i = 0; i < H2K_gp->hthreads; i++) {
		if (H2K_gp->runlist_prios[i] IS_WORSE_THAN worst_prio) {
			worst_prio = H2K_gp->runlist_prios[i];
			hthread = i;
		}
	}
	return hthread;
}

static inline void H2K_runlist_remove(H2K_thread_context *thread)
{
	u32_t hthread = thread->hthread;
	H2K_gp->runlist[hthread] = NULL;
	H2K_gp->runlist_prios[hthread] = -1;
}

void H2K_runlist_init(void) IN_SECTION(".text.init.runlist");

#endif

