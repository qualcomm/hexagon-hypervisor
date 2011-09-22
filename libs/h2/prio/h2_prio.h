/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PRIO_H
#define H2_PRIO_H 1

int h2_get_prio(void);
int h2_set_prio(unsigned int threadid, unsigned int newprio);

#endif

