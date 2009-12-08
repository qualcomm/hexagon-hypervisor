/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_INTWAIT_H
#define H2_INTWAIT_H 1

/*
 * H2 intwait.h
 *
 * Ask the kernel to wait for an interrupt, out of the interrupts
 * in mask.  The return value is the interrupt taken. 
 */

int h2_intwait(unsigned long long int mask);

#endif

