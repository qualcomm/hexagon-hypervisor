/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <runlist.h>
#include <readylist.h>

void init_setup()
{
	runlist_init();
	readylist_init();
}

