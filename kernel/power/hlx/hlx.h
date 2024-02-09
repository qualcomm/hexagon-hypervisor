/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_POWER_HLX_H
#define H2K_POWER_HLX_H 1

#define H2K_HLX_STATE_OFF 0
#define H2K_HLX_STATE_ON  1

/* #define QDSP6SS_CP_CLK_CTL 0x000000fc */
/* #define QDSP6SS_CP_CLK_CTL_DISABLE      0x00000000 */
/* #define QDSP6SS_CP_CLK_CTL_ENABLE       0x00000001 */

/* #define QDSP6SS_CP_RESET   0x000000f8 */
/* #define QDSP6SS_CP_RESET_ASSERT         0x00000001 */
/* #define QDSP6SS_CP_RESET_DEASSERT       0x00000000 */

/* #define QDSP6SS_CP_PWR_CTL 0x000000f0 */
/* #define QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60  0x011fffff */
/* #define QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60 0x010fffff */
/* #define QDSP6SS_CP_PWR_CTL_POWER_OFF_V60    0x001fffff */

/* #define QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62  0x00000003 */
/* #define QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62 0x00000002 */
/* #define QDSP6SS_CP_PWR_CTL_POWER_OFF_V62    0x00000001 */

#ifndef ASM
#include <c_std.h>
void H2K_hlx_poweron(void) IN_SECTION(".text.power.hlx");
void H2K_hlx_poweroff(void) IN_SECTION(".text.power.hlx");
#endif

#endif
