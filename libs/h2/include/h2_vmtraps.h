/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_VMTRAPS_H
#define H2_VMTRAPS_H 1

int h2_vmtrap_setvec(void *ptr);
int h2_vmtrap_clrmap(void *ptr);
int h2_vmtrap_newptb(void *newbase);
void h2_vmtrap_setregs(unsigned int g0, unsigned int g1, unsigned int g2, unsigned int g3);
void h2_vmtrap_getregs(unsigned int *regsptr);

#endif

