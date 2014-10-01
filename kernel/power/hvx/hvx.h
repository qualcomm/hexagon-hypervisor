/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_POWER_HVX_H
#define H2K_POWER_HVX_H 1

#include <c_std.h>

#define H2K_HVX_STATE_OFF 0
#define H2K_HVX_STATE_ON  1

void H2K_hvx_poweron(void) IN_SECTION(".text.power.hvx");
void H2K_hvx_poweroff(void) IN_SECTION(".text.power.hvx");

#endif
