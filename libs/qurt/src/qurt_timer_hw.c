/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurtos_internal.h>

#define TIME_FOREVER (~0ULL)

#define TIMERHW_NS2TICKS(X)	((X) / resolution)
#define TIMERHW_TICKS2NS(X)	((X) * resolution)

unsigned long long int resolution;
qurt_timetick_word_t timer_match_val = TIME_FOREVER;

void hw_set_timeout(qurt_timetick_word_t match) {

	h2_vmtrap_timerop(H2K_TIMER_TRAP_SET_TIMEOUT, TIMERHW_TICKS2NS(match));
	timer_match_val = match;
}

qurt_timetick_word_t hw_timer_curr_timetick (void) {

	/* Don't need the IST to do this */
	return 	TIMERHW_NS2TICKS(h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_TIME, 0));
}

unsigned long long int qurt_sysclock_get_hw_ticks() {
	return hw_timer_curr_timetick();
}

qurt_timetick_word_t hw_timer_match_val (void) {

	return timer_match_val;
}

qurt_timetick_word_t hw_timer_prg_next_interrupt (qurt_timetick_word_t match, qurt_timetick_word_t count, unsigned int order) {

	hw_set_timeout(match);
	return hw_timer_curr_timetick() - count;
}

void hw_timer_init () {

	resolution = h2_vmtrap_timerop(H2K_TIMER_TRAP_GET_RESOLUTION, 0);
}

unsigned long long qurt_timer_timetick_to_us( unsigned long long n ) {
	return (n*5ULL/96ULL);
}
