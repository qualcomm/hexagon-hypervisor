/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <h2_common_timer.h>

unsigned long long int h2_nanosleep(unsigned long long int time)
{
	unsigned long long int now = h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME,0);
	unsigned long long int end = now + time;
	long long int delta;
	ie_type old_ie;
	old_ie = h2_vmtrap_setie(0);
	h2_vmtrap_intop(H2K_INTOP_GLOBEN, H2K_TIME_GUESTINT, 0);
	h2_vmtrap_timerop(H2K_TIMER_TRAP_SET_TIMEOUT, end);
	h2_vmtrap_wait();
	h2_vmtrap_setie(1);
	now = h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME, 0);
	delta = end-now;
	if (delta < 0) delta = 0;
	h2_vmtrap_setie(old_ie);
	return delta;
}

