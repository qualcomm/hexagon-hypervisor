/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_INTWAIT_H
#define BLAST_INTWAIT_H 1

/*
 * BLAST intwait.h
 *
 * Ask the kernel to wait for an interrupt, out of the interrupts
 * in mask.  The return value is the interrupt taken. 
 */

int blast_intwait(unsigned long long int mask);

#endif

