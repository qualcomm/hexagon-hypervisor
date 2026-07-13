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
#include <log.h>
#include <hw.h>

static inline void H2K_runlist_push(H2K_thread_context *newthread)
{
	u32_t hthread = newthread->hthread;
	u32_t prio = newthread->prio;
	newthread->status = H2K_STATUS_RUNNING;
	H2K_gp->wip_dummy_runlist[hthread] = newthread;
	H2K_gp->wip_dummy_runlist_prios[hthread] = (s16_t)prio;
}

static inline void H2K_runlist_remove(H2K_thread_context *thread)
{
	u32_t hthread = thread->hthread;
	H2K_gp->wip_dummy_runlist[hthread] = NULL;
	H2K_gp->wip_dummy_runlist_prios[hthread] = -1;
}

static inline void H2K_runlist_set_thread_prio(H2K_thread_context *thread, u32_t new_prio)
{
	thread->prio = (u8_t)new_prio;
	H2K_gp->wip_dummy_runlist_prios[thread->hthread] = (s16_t)new_prio;
}

void H2K_runlist_init(void) IN_SECTION(".text.init.runlist");

#endif

