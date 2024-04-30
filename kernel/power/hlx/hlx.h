/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_POWER_HLX_H
#define H2K_POWER_HLX_H 1

#define H2K_HLX_STATE_OFF 0
#define H2K_HLX_STATE_ON  1

#ifndef ASM
#include <c_std.h>
void H2K_hlx_poweron(void) IN_SECTION(".text.power.hlx");
void H2K_hlx_poweroff(void) IN_SECTION(".text.power.hlx");
#endif

#endif
