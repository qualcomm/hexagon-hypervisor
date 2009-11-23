/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <runlist.h>
#include <readylist.h>

void BLASTK_init_setup()
{
	BLASTK_runlist_init();
	BLASTK_readylist_init();
}

