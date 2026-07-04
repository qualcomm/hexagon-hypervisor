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

#if ARCHV >= 4
static inline void H2K_intcontrol_enable(u32_t intno)
{
	u32_t l2off = intno - L2_INTERRUPT_START;
	// u32_t vic   = l2off / MAX_L2_INTERRUPTS;
	u32_t src   = l2off % MAX_L2_INTERRUPTS;
	u32_t index = src >> 5;
	u32_t mask  = 1U << (src & 31);
	if (intno < L2_INTERRUPT_START) {
		ciad(1U << intno);
	} else {
		((volatile u32_t *)(H2K_gp->l2_ack_base))[index] = mask;
	}
}

static inline void H2K_intcontrol_disable(u32_t intno)
{
	u32_t l2off = intno - L2_INTERRUPT_START;
	// u32_t vic   = l2off / MAX_L2_INTERRUPTS;
	u32_t src   = l2off % MAX_L2_INTERRUPTS;
	u32_t index = src >> 5;
	u32_t mask  = 1U << (src & 31);
	if (intno < L2_INTERRUPT_START) {
		siad(1U << intno);
	} else {
		((volatile u32_t *)(H2K_gp->l2_int_base + (0x180/sizeof(u32_t))))[index] = mask;
	}
}

static inline void H2K_intcontrol_raise(u32_t intno)
{
	u32_t l2off = intno - L2_INTERRUPT_START;
	// u32_t vic   = l2off / MAX_L2_INTERRUPTS;
	u32_t src   = l2off % MAX_L2_INTERRUPTS;
	u32_t index = src >> 5;
	u32_t mask  = 1U << (src & 31);
	if (intno < L2_INTERRUPT_START) {
		swi(1U << intno);
	} else {
		((volatile u32_t *)(H2K_gp->l2_int_base + (0x480/sizeof(u32_t))))[index] = mask;
	}
}

#else

static inline void H2K_intcontrol_enable(u32_t intno)
{
	ciad(0x80000000UL>>(intno & 31));
}

static inline void H2K_intcontrol_disable(u32_t intno)
{
	/* Actually, we can't do this right... no SIAD on V3 */
}

static inline void H2K_intcontrol_raise(u32_t intno)
{
	intno &= 31;
	swi(0x80000000UL>>intno);
}

#endif

#endif
