/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <max.h>
#include <hw.h>

s32_t H2K_stmode_begin()
{
	s32_t ret;
	u32_t is_stmode;
	u32_t tmp;
	/* Acquire lock */
	BKL_LOCK();

	/* Clear syscfg.g */
	H2K_clear_gie();

	/* check modectl */
	tmp = H2K_get_modectl();
	/* Compute threads that are ON and not WAITing */
	tmp = (tmp & ((~tmp)>>16)) & 0x0000ffff;
	/* Remove Least Significant Set Bit */
	tmp = tmp & (tmp-1);
	/* Are we in ST mode? */
	is_stmode = (tmp == 0);
	if (is_stmode) {
		ret = 0;
	} else {
		ret = -1;
		/* Set syscfg.g */
		H2K_set_gie();
	}
	BKL_UNLOCK();
	return ret;
}

void H2K_stmode_end()
{
	/* set syscfg.g */
	H2K_set_gie();
}

