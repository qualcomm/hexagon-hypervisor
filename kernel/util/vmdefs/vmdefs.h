/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMDEFS_H
#define H2K_VMDEFS_H 1

#if ARCHV <= 3
#else
#endif

/* Version supported */

#define H2K_VM_VERSION 0x00000800

#define RESET_GEVB_OFFSET (0*4)
#define CHECK_GEVB_OFFSET (1*4)
#define ERROR_GEVB_OFFSET (2*4)
#define DEBUG_GEVB_OFFSET (3*4)
#define TRAP0_GEVB_OFFSET (5*4)
#define INTERRUPT_GEVB_OFFSET (7*4)

#define H2K_GSSR_UM_BIT 31
#define H2K_GSSR_IE_BIT 30
#define H2K_GSSR_SS_BIT 29
#define H2K_GSSR_UM (0x1 << H2K_GSSR_UM_BIT)
#define H2K_GSSR_IE (0x1 << H2K_GSSR_IE_BIT)
#define H2K_GSSR_SS (0x1 << H2K_GSSR_SS_BIT)

#define H2K_VMSTATUS_VMWORK_BIT  0
#define H2K_VMSTATUS_VMWORK	(0x01 << (H2K_VMSTATUS_VMWORK_BIT))
#define H2K_VMSTATUS_KILL_BIT   1
#define H2K_VMSTATUS_KILL	(0x01 << (H2K_VMSTATUS_KILL_BIT))

#ifdef HAVE_EXTENSIONS
/* extended registers are live */
#define H2K_VMSTATUS_SAVEXT_BIT     6
#define H2K_VMSTATUS_SAVEXT		(0x01 << (H2K_VMSTATUS_SAVEXT_BIT))
#endif

#define H2K_VMSTATUS_IE_BIT     7
#define H2K_VMSTATUS_IE		(0x01 << (H2K_VMSTATUS_IE_BIT))

#endif
