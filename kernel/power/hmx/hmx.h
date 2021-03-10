/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_POWER_HMX_H
#define H2K_POWER_HMX_H 1

#define H2K_HMX_STATE_OFF 0
#define H2K_HMX_STATE_ON  1

// Lahaina/Makena/Cedros Cores RSC Config @ SiVal Project Generation Folders
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV68  0x59
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV68   0x63
// Waipio/Fillmore Cores RSC Config @ SiVal Project Generation Folders
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV69  0x6a
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV69   0x76
// Kailua Cores RSC Config @ SiVal Project Generation Folders
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_COREV73  0x6a
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_COREV73   0x76
// Other TBD Cores RSC Config @ SiVal Project Generation Folders
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_OFF_START_ADDR_DEFAULT  0x6a
#define Q6SS_RSCC_CDSP_HMX_RSC_SEQ_POWER_ON_START_ADDR_DEFAULT   0x76

#ifndef ASM
#include <c_std.h>
void H2K_hmx_poweron(void) IN_SECTION(".text.power.hmx");
void H2K_hmx_poweroff(void) IN_SECTION(".text.power.hmx");
#endif

#endif
