/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <posix_time.h>
#include <errno.h>

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	unsigned long long int cycles;
	unsigned int nsec;
	unsigned int sec;
	switch (clock_id) {
	case CLOCK_MONOTONIC: cycles = h2_get_core_pcycles(); break;
	case CLOCK_THREAD_CPUTIME_ID: cycles = h2_get_pcycles(); break;
	default: return EINVAL;
	}
	/* Assume approximately 1GHz. */
	nsec = cycles & 0x3FFFFFFF;
	sec = cycles >> 30;
	/* tv_nsec is allowed to have 0...999999999 */
	if (nsec >= 1000000000) {
		nsec -= 1000000000;
		sec += 1;
	}
	tp->tv_sec = sec;
	tp->tv_nsec = nsec;
	return 0;
}

