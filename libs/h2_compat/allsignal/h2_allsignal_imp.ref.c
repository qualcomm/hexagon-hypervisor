/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_allsignal_imp.ref.c
 * @brief All-signals wake up the waiter when all bits in the mask are set - Implementation
 */

#include "h2_allsignal.h"

void h2_allsignal_init(h2_allsignal_t *signal) { 
	signal->raw = 0; 
}
