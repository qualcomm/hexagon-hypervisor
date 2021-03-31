/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

#include "h2_trap_constants.h"

FUNC_START h2_pmuctrl_trap .text.h2.pmu
	trap0(#H2_TRAP_PMUCTRL)
	jumpr r31
FUNC_END h2_pmuctrl_trap

DATA_START __h2_pmu_evtcfg__ .sdata.h2.pmu .p2align 2
	.word 0x0
DATA_END __h2_pmu_evtcfg__

DATA_START __h2_pmu_evtcfg1__ .sdata.h2.pmu .p2align 2
	.word 0x0
DATA_END __h2_pmu_evtcfg1__

DATA_START __h2_pmu_cfg__ .sdata.h2.pmu .p2align 2
	.word 0x0
DATA_END __h2_pmu_cfg__
