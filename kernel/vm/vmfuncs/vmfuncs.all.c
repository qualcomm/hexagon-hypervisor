/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pmu.h>

void H2K_vmtrap_pmuconfig(H2K_thread_context *me) {

	me->r00 = H2K_trap_pmuconfig(me->r00, me->r01, me->r02, me->r03, me);
}
