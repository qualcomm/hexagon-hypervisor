/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_IDTYPE_H
#define H2K_IDTYPE_H 1

#ifndef ASM
#include <c_std.h>
#endif

#define H2K_ID_VINT_BITS 10  // for mapping passthru interrupts
#define H2K_ID_CPUIDX_BITS 16
#define H2K_ID_CPUIDX_MASK (((1 << H2K_ID_CPUIDX_BITS) - 1) << H2K_ID_VINT_BITS)
#define H2K_ID_VMIDX_BITS 6
#define H2K_ID_MAX_VMS (1<<(H2K_ID_VMIDX_BITS))
#define H2K_ID_MAX_CPUS (1<<(H2K_ID_CPUIDX_BITS))

#if ((H2K_ID_VINT_BITS+H2K_ID_VMIDX_BITS+H2K_ID_CPUIDX_BITS) != 32)
#error bits do not add up
#endif

#ifndef ASM
typedef union {
	u32_t raw;
	struct {
		u32_t reserved:H2K_ID_VINT_BITS;
		u32_t cpuidx:H2K_ID_CPUIDX_BITS;
		u32_t vmidx:H2K_ID_VMIDX_BITS;
	};
} H2K_id_t;
#endif

#endif
