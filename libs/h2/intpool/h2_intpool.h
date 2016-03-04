/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_INTPOOL_H
#define H2_INTPOOL_H 1

#include <h2_trap_constants.h>

void h2_intpool_config(unsigned int interrupt, unsigned int enable);
int h2_intpool_wait(int int_ack_num);

#endif
