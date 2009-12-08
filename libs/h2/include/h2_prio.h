/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PRIO_H
#define H2_PRIO_H 1

int h2_get_prio();
int h2_set_prio(unsigned int threadid, unsigned int newprio);
unsigned int h2_mask_prios_above(unsigned int worst_prio);

#endif

