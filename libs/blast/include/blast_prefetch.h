/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_PREFETCH_H
#define BLAST_PREFETCH_H

#define BLAST_PREFETCH_I 1
#define BLAST_PREFETCH_D 2
#define BLAST_PREFETCH_SW 4

void blast_set_prefetch(unsigned int settings);

#endif

