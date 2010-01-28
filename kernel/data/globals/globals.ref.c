/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <globals.h>

H2K_kg_t H2K_kg;

void H2K_traptab();
u64_t H2K_stacks;

void H2K_kg_init()
{
	H2K_kg.traptab_addr = H2K_traptab;
	H2K_kg.stacks_addr = &H2K_stacks;
}

