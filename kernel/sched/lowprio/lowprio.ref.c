/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <lowprio.h>

void H2K_lowprio_init()
{
	H2K_gp->priomask = H2K_gp->wait_mask = 0;
}

