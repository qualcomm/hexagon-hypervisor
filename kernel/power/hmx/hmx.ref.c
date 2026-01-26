/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hmx.h>
#include <atomic.h>
#include <hw.h>

#if ARCHV >= 68
#ifndef NO_DEVICES
#ifdef HMX_HLX_SUPPORT
static inline void H2K_hmx_rsc_seq_power_change(u32_t hmx_rsc_seq_power_change_start_addr) {
	/* From HPG 4.8.X */
	volatile u32_t delay;

	delay = 1000;
	while ((delay--) &&
		((*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_busy_drv0))) & (0x1 << QDSP6SS_RSCC_RSC_SEQ_BUSY_DRV0_STATUS_BIT))) asm volatile ("nop");

	*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_override_trigger_start_addr_drv0)) = (hmx_rsc_seq_power_change_start_addr & QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_START_ADDR_DRV0_ADDRESS_BITMASK);

	*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_override_trigger_drv0)) &= ~(((u32_t)(0x1)) << QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0_TRIGGER_BIT);
	H2K_syncht();

	*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_override_trigger_drv0)) |= (((u32_t)(0x1)) << QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0_TRIGGER_BIT);
	H2K_syncht();

	delay = 1000;
	while ((delay--) &&
		!((*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_override_trigger_drv0))) & (0x1 << QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0_TRIGGER_BIT))) asm volatile ("nop");

	delay = 1000;
	while ((delay--) &&
		((*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_busy_drv0))) & (0x1 << QDSP6SS_RSCC_RSC_SEQ_BUSY_DRV0_STATUS_BIT))) asm volatile ("nop");

	*((u32_t volatile *)(H2K_gp->hmx_rsc_seq_override_trigger_drv0)) &= ~(((u32_t)(0x1)) << QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0_TRIGGER_BIT);
	H2K_syncht();
}
#endif
#endif
#endif

void H2K_hmx_poweron(void) {
#ifndef NO_DEVICES
#ifdef HAVE_EXTENSIONS
#ifdef HMX_HLX_SUPPORT
	if (!H2K_gp->info_boot_flags.boot_have_hmx) {
		return;
	}

#if ARCHV >= 68
	BKL_LOCK();
	if (H2K_gp->hmx_state == H2K_HMX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

	H2K_hmx_rsc_seq_power_change(H2K_gp->hmx_rsc_seq_power_on_start_addr);

	H2K_gp->hmx_state = H2K_HMX_STATE_ON;
	BKL_UNLOCK();
#endif
#endif
#endif
#endif
}

void H2K_hmx_poweroff(void) {
#ifndef NO_DEVICES
#ifdef HAVE_EXTENSIONS
#ifdef HMX_HLX_SUPPORT
	if (!H2K_gp->info_boot_flags.boot_have_hmx) {
		return;
	}

#if ARCHV >= 68
	BKL_LOCK();
	if (H2K_gp->hmx_state == H2K_HMX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

	H2K_hmx_rsc_seq_power_change(H2K_gp->hmx_rsc_seq_power_off_start_addr);

	H2K_gp->hmx_state = H2K_HMX_STATE_OFF;
	BKL_UNLOCK();
#endif
#endif
#endif
#endif
}
