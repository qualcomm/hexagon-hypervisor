/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CYCLES_H
#define H2_CYCLES_H 1

unsigned long long int h2_get_pcycles();

static inline unsigned long long int h2_get_tcycles() { return h2_get_pcycles()/6; }

unsigned long long int h2_get_core_pcycles();

#endif

