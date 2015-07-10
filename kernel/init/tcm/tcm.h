/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TCM_H
#define TCM_H 1

void H2K_tcm_copy(u32_t l2_tags, u32_t last_tlb_index) IN_SECTION(".text.init.tcm");
void H2K_tcm_crash_copy() IN_SECTION(".text.crash.tcm");

#endif
