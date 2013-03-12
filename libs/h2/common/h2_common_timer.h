/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_TIMER_H
#define H2_COMMON_TIMER_H 1

typedef enum {
	H2K_TIMER_TRAP_GET_FREQ,
	H2K_TIMER_TRAP_GET_RESOLUTION,
	H2K_TIMER_TRAP_GET_TIME,
	H2K_TIMER_TRAP_GET_TIMEOUT,
	H2K_TIMER_TRAP_SET_TIMEOUT,
	H2K_TIMER_TRAP_DELTA_TIMEOUT,
	H2K_TIMER_TRAP_INVALID
} timerop_type;

#endif
