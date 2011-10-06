/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMDEFS_H
#define H2K_VMDEFS_H 1

#if __QDSP6_ARCH__ <= 3
#else
#endif

#define RESET_GEVB_OFFSET (0*4)
#define CHECK_GEVB_OFFSET (1*4)
#define ERROR_GEVB_OFFSET (2*4)
#define TRAP0_GEVB_OFFSET (5*4)
#define INTERRUPT_GEVB_OFFSET (7*4)

/* FIXME:  also defined in context.h */
#define H2K_GSSR_UM_BIT 31
#define H2K_GSSR_IE_BIT 30
#define H2K_GSSR_UM (0x1 << H2K_GSSR_UM_BIT)
#define H2K_GSSR_IE (0x1 << H2K_GSSR_IE_BIT)
#define H2K_VMSTATUS_IE_BIT     7

#endif
