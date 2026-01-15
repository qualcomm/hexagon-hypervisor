/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_anysignal_imp.ref.c
 * @brief Any-signals wake up the waiter when any bits in the mask are set - Implementation
 */

#include "h2_anysignal.h"

void h2_anysignal_init(h2_anysignal_t *signal) { 
	signal->raw = 0; 
}

unsigned int h2_anysignal_get(h2_anysignal_t *signal) { 
	return signal->signals; 
}
