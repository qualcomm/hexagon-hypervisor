/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_cache_imp.ref.c
 * 
 * @brief Cache operations - Implementation
 */

#include "h2_cache.h"

void h2_icinva(void *ptr)
{
	asm volatile (" icinva(%0) " : :"r"(ptr) : "memory");
}

void h2_dccleana(void *ptr)
{
	asm volatile (" dccleana(%0) " : :"r"(ptr) : "memory");
}

void h2_dccleaninva(void *ptr)
{
	asm volatile (" dccleaninva(%0) " : :"r"(ptr) : "memory");
}

void h2_dcinva(void *ptr)
{
	asm volatile (" dcinva(%0) " : :"r"(ptr) : "memory");
}
