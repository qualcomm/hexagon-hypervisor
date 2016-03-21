/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2if.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
/* EJP: for now, lie about nanosleep.
 * We can actually nanosleep, see h2_nanosleep, but it might not be conformant and we have to simulate with timer.
 */
	unsigned long long int now = h2_get_elapsed_nanos();
	unsigned long long int delta;
	unsigned long long int later;
	delta = req->tv_sec;
	delta <<= 30;
	delta |= req->tv_nsec;
	later = now + delta;
	while (h2_get_elapsed_nanos() < later) /* SPIN */;
	return 0;
}
/* BMC: lie about sleep for now as well */
unsigned sleep(unsigned seconds)
{
	struct timespec sleeptime;
	sleeptime.tv_nsec = 0;
	sleeptime.tv_sec = seconds;
	return nanosleep(&sleeptime,NULL);
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	unsigned long long int cycles;
	unsigned int nsec;
	unsigned int sec;
	switch (clock_id) {
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC:
		cycles = h2_get_elapsed_nanos(); break;
	case CLOCK_THREAD_CPUTIME_ID:
	case CLOCK_PROCESS_CPUTIME_ID:
		cycles = h2_get_pcycles(); break;
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

