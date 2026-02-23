/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_cycles_imp.ref.c
 * 
 * @brief Cycle Information - Implementation
 */

#include "h2_cycles.h"

unsigned long long int h2_get_tcycles(void) { 
	return h2_get_pcycles()/H2_CYCLES__PER_THREAD; 
}

unsigned long long int h2_get_core_pcycles(void) {
#if ARCHV < 60
	return h2_get_core_pcycles_trap();
#else
	unsigned long long int ret;
	asm volatile ( " %0 = c15:14 // READ UPCYCLES \n" : "=r"(ret));
	return ret;
#endif
}
