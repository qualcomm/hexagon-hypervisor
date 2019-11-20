/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pmu.h>
#include <vmblock.h>
#include <globals.h>
#include <info.h>

void H2K_vmtrap_pmuctrl(H2K_thread_context *me) {

	u32_t op = me->r00;
	H2K_id_t id;
	u32_t turnon = me->r02;
	H2K_vmblock_t *vmblock = me->vmblock;
	u32_t i;
	u32_t ret = 0;

	id.raw = me->r01;

	if (op == PMUCTRL_THREADSET && id.raw == 0) {
		id.vmidx = vmblock->vmidx;
		// Turn on/off for all vcpus
		for (i = 0; i < vmblock->max_cpus; i++) {
			id.cpuidx = i;
			/* Ignore error from dead CPUs */
			if (H2K_trap_pmuctrl((pmuop_type)op, id.raw, turnon, 0, me) == 0) {
				ret++;
			}
		}
		me ->r00 = ret;
	} else {
		me->r00 = H2K_trap_pmuctrl((pmuop_type)me->r00, me->r01, me->r02, me->r03, me);
	}
}

void H2K_vmtrap_version(H2K_thread_context *me)
{
	me->r00 = H2K_VM_VERSION;
}

void H2K_vmtrap_info(H2K_thread_context *me) {

	me->r00 = H2K_trap_info((info_type)(me->r00), me);
}

