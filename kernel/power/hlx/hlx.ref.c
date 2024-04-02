/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

//TODO:REDO for HLX
#include <c_std.h>
#include <max.h>
#include <globals.h>
#include <hlx.h>
#include <atomic.h>
#include <hw.h>

void H2K_hlx_poweron(void) {
#ifndef NO_DEVICES
	if (!H2K_gp->info_boot_flags.boot_have_hlx) {
		return;
	}

	BKL_LOCK();
	if (H2K_gp->hlx_state == H2K_HLX_STATE_ON) {  // already on
		BKL_UNLOCK();
		return;
	}

	H2K_gp->hlx_state = H2K_HLX_STATE_ON;
	BKL_UNLOCK();
#endif
}

void H2K_hlx_poweroff(void) {
#ifndef NO_DEVICES

	if (!H2K_gp->info_boot_flags.boot_have_hlx) {
		return;
	}

	BKL_LOCK();

	if (H2K_gp->hlx_state == H2K_HLX_STATE_OFF) {  // already off
		BKL_UNLOCK();
		return;
	}

	H2K_gp->hlx_state = H2K_HLX_STATE_OFF;

	BKL_UNLOCK();
#endif
}
