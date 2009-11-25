/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_CYCLES_H
#define BLAST_CYCLES_H 1

unsigned long long int blast_get_pcycles();

static inline unsigned long long int blast_get_tcycles() { return blast_get_pcycles()/6; }

unsigned long long int blast_get_core_pcycles();

#endif

