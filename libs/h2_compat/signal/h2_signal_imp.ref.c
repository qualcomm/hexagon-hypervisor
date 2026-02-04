/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_signal_imp.ref.c
 * @brief Signals wake up the waiter when signals are set - Implementation
 */

#include "h2_signal.h"

void h2_signal_init(h2_signal_t *signal) { 
	signal->raw = 0; 
}

unsigned int h2_signal_wait(h2_signal_t *signal, unsigned int mask, unsigned int type)
{
	if (type == H2_SIGNAL_WAIT_ANY) return h2_signal_wait_any(signal,mask);
	else return h2_signal_wait_all(signal,mask);
}

unsigned int h2_signal_clear(h2_signal_t *signal, unsigned int mask)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	return h2_atomic_and32((unsigned int *)&signal->signals,~mask);
#pragma GCC diagnostic pop
}

unsigned int h2_signal_get(h2_signal_t *signal) { 
	return signal->signals; 
}
