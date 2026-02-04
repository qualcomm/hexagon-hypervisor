/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_vmtraps.h"

int h2_vmtrap_newmap(void *newbase, translation_type type, tlb_invalidate_flag flag)
{
	return h2_vmtrap_newmap_extra(newbase,type,flag,0);
}
