/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_PRIO_H
#define BLAST_PRIO_H 1

int blast_get_prio();
int blast_set_prio(unsigned int threadid, unsigned int newprio);
unsigned int blast_mask_prios_above(unsigned int worst_prio);

#endif

