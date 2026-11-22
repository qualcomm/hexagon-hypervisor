/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_VMINT_H
#define H2_COMMON_VMINT_H 1

#ifndef ASM
typedef enum {
	H2K_INTOP_NOP,
	H2K_INTOP_GLOBEN,
	H2K_INTOP_GLOBDIS,
	H2K_INTOP_LOCEN,
	H2K_INTOP_LOCDIS,
	H2K_INTOP_AFFINITY,
	H2K_INTOP_GET,
	H2K_INTOP_PEEK,
	H2K_INTOP_STATUS,
	H2K_INTOP_POST,
	H2K_INTOP_CLEAR,
	H2K_INTOP_FIRST_INVALID_OP
} intop_type;

typedef enum {
	H2K_IE_DISABLE,
	H2K_IE_ENABLE,
	H2K_IE_END
} ie_type;
#else
#define H2K_IE_DISABLE 0
#define H2K_IE_ENABLE 1
#define H2K_IE_END 2
#endif

#endif
