/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ID_H
#define H2K_ID_H 1

#include <globals.h>
#include <context.h>
#include <idtype.h>

static inline H2K_thread_context *H2K_id_cpuidx_to_context(struct H2K_vmblock_struct *vmblock, u32_t cpuidx)
{
	if (cpuidx > vmblock->max_cpus - 1) return NULL;
	return &vmblock->contexts[cpuidx];
}

static inline H2K_thread_context *H2K_id_to_context(H2K_id_t id)
{
	u32_t cpuidx = id.cpuidx;
	u32_t vmidx = id.vmidx;
	struct H2K_vmblock_struct *vmblock;
	vmblock = H2K_gp->vmblocks[vmidx];
	if (vmblock == NULL) return NULL;
	return H2K_id_cpuidx_to_context(vmblock,cpuidx);
}

static inline H2K_id_t H2K_id_from_context(H2K_thread_context *t)
{
	return t->id;
}

IN_SECTION(".text.core.id") H2K_id_t H2K_thread_id(H2K_thread_context *me);

#endif
