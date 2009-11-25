/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_ANYSIGNAL_H
#define BLAST_ANYSIGNAL_H 1

typedef union {
	unsigned long long int raw;
	struct {
		unsigned int signals;
		unsigned int waiting;
	};
} blast_anysignal_t;

static inline void blast_anysignal_init(blast_anysignal_t *signal) { signal->raw = 0; };
unsigned int blast_anysignal_wait(blast_anysignal_t *signal, unsigned int mask);
unsigned int blast_anysignal_set(blast_anysignal_t *signal, unsigned int mask);
static inline unsigned int blast_anysignal_get(blast_anysignal_t *signal) { return signal->signals; };
unsigned int blast_anysignal_clear(blast_anysignal_t *signal, unsigned int mask);

#endif

