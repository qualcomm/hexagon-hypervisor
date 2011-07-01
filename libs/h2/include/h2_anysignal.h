/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ANYSIGNAL_H
#define H2_ANYSIGNAL_H 1

typedef union {
	unsigned long long int volatile raw;
	struct {
		unsigned int volatile signals;
		unsigned int volatile waiting;
	};
} h2_anysignal_t;

#define H2_ANYSIGNAL_T_INIT { 0 }

static inline void h2_anysignal_init(h2_anysignal_t *signal) { h2_anysignal_t temp = H2_ANYSIGNAL_T_INIT; *signal = temp; };
unsigned int h2_anysignal_wait(h2_anysignal_t *signal, unsigned int mask);
unsigned int h2_anysignal_set(h2_anysignal_t *signal, unsigned int mask);
static inline unsigned int h2_anysignal_get(h2_anysignal_t *signal) { return signal->signals; };
unsigned int h2_anysignal_clear(h2_anysignal_t *signal, unsigned int mask);

#endif

