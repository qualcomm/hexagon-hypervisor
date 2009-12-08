/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ALLSIGNAL_H
#define H2_ALLSIGNAL_H

typedef union {
	unsigned long long int raw;
	struct {
		unsigned int waiting;
		unsigned int signals_in;
	};
} h2_allsignal_t;

static inline void h2_allsignal_init(h2_allsignal_t *signal) { signal->raw = 0; };
void h2_allsignal_wait(h2_allsignal_t *signal, unsigned int mask);
void h2_allsignal_signal(h2_allsignal_t *signal, unsigned int mask);

#endif

