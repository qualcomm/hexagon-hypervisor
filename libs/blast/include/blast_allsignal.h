/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_ALLSIGNAL_H
#define BLAST_ALLSIGNAL_H

typedef union {
	unsigned long long int raw;
	struct {
		unsigned int waiting;
		unsigned int signals_in;
	};
} blast_allsignal_t;

static inline void blast_allsignal_init(blast_allsignal_t *signal) { signal->raw = 0; };
void blast_allsignal_wait(blast_allsignal_t *signal, unsigned int mask);
void blast_allsignal_signal(blast_allsignal_t *signal, unsigned int mask);

#endif

