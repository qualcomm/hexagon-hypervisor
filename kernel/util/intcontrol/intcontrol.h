/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_INTCONTROL_H
#define H2K_INTCONTROL_H 1

#include <globals.h>
#include <max.h>
#include <hw.h>

/* EJP: FIXME: want to add (for example) raise? */

static inline void H2K_intcontrol_enable(u32_t intno)
{
	u32_t index = (intno - 32) >> 5;
	u32_t mask = 1U<<(intno & 31);
	if (intno < 32) {
		ciad(mask);
	} else {
		((volatile u32_t *)(H2K_gp->l2_ack_base))[index] = mask;
	}
}

static inline void H2K_intcontrol_disable(u32_t intno)
{
	u32_t index = (intno - 32) >> 5;
	u32_t mask = 1U<<(intno & 31);
	if (intno < 32) {
		siad(mask);
	} else {
		((volatile u32_t *)(H2K_gp->l2_int_base+(0x180/sizeof(u32_t))))[index] = mask;
	}
}

static inline void H2K_intcontrol_raise(u32_t intno)
{
	u32_t index = (intno - 32) >> 5;
	u32_t mask = 1U<<(intno & 31);
	if (intno < 32) {
		swi(mask);
	} else {
		((volatile u32_t *)(H2K_gp->l2_int_base+(0x480/sizeof(u32_t))))[index] = mask;
	}
}

#endif
