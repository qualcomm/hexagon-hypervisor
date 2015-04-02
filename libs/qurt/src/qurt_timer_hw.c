/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurtos_internal.h>

#define HW_COUNT_LO (0x1000/4)
#define HW_COUNT_HI (0x1004/4)
#define HW_MATCH_LO (0x1020/4)
#define HW_MATCH_HI (0x1024/4)
#define HW_ENABLE   (0x102C/4)

#define HW_CNTFRQ	(0x0000/4)
#define HW_CNTSR	(0x0004/4)
#define HW_CNTACR	(0x0040/4)

#define QURT_TIMER_BASE 0xfe2a0000

#define TIME_FOREVER (~0ULL)

unsigned long volatile *devptr;

void hw_set_timeout(qurt_timetick_word_t match) {

	devptr[HW_MATCH_HI] = ~0;
	devptr[HW_MATCH_LO] = match;
	devptr[HW_MATCH_HI] = match >> 32;
}

qurt_timetick_word_t hw_timer_curr_timetick (void) {

	unsigned long tmphi;
	union {
		unsigned long long pair;
		struct {
			unsigned long lo;
			unsigned long hi;
		};
	} ticktmp;

	do {
		tmphi = devptr[HW_COUNT_HI];
		ticktmp.lo = devptr[HW_COUNT_LO];
		ticktmp.hi = devptr[HW_COUNT_HI];
	} while (tmphi != ticktmp.hi);
	return ticktmp.pair;
}

qurt_timetick_word_t hw_timer_match_val (void) {

	unsigned long tmphi;
	union {
		unsigned long long pair;
		struct {
			unsigned long lo;
			unsigned long hi;
		};
	} ticktmp;

	do {
		tmphi = devptr[HW_MATCH_HI];
		ticktmp.lo = devptr[HW_MATCH_LO];
		ticktmp.hi = devptr[HW_MATCH_HI];
	} while (tmphi != ticktmp.hi);
	return ticktmp.pair;
}

qurt_timetick_word_t hw_timer_prg_next_interrupt (qurt_timetick_word_t match, qurt_timetick_word_t count, unsigned int order) {

	hw_set_timeout(match);
	return hw_timer_curr_timetick() - count;
}

void hw_timer_init (qurt_timetick_word_t match) {

	 devptr = (unsigned long *)QURT_TIMER_BASE;

	 hw_set_timeout(TIME_FOREVER);

	 devptr[HW_CNTSR] = ~0;
	 devptr[HW_CNTACR] = ~0;
	 devptr[HW_CNTFRQ] = 19200000;
	 devptr[HW_ENABLE] = 1;

}

unsigned long long qurt_timer_timetick_to_us( unsigned long long n ) {
	return (n*5ULL/96ULL);
}
