/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_VMTRAPS_H
#define H2_VMTRAPS_H 1

#include <h2_common_asid.h>
#include <h2_common_vmint.h>
#include <h2_common_timer.h>
#include <h2_common_pmu.h>
#include <h2_common_info.h>

void h2_vmtrap_return();
int h2_vmtrap_setvec(void *ptr);
int h2_vmtrap_setie(ie_type val);
int h2_vmtrap_getie();
int h2_vmtrap_intop(intop_type op, unsigned int arg1, unsigned int arg2);
int h2_vmtrap_clrmap(void *ptr);
int h2_vmtrap_newmap(void *newbase, translation_type type, tlb_invalidate_flag flag);
int h2_vmtrap_cachectl(unsigned int op, void *start, unsigned int count);
unsigned long long int h2_vmtrap_get_pcycles();
void h2_vmtrap_set_pcycles(unsigned long long int);
int h2_vmtrap_wait();
void h2_vmtrap_yield();
int h2_vmtrap_start(void *pc, void *stack);
void h2_vmtrap_stop(int);
int h2_vmtrap_vmpid();
void h2_vmtrap_setregs(unsigned int g0, unsigned int g1, unsigned int g2, unsigned int g3);
void h2_vmtrap_getregs(unsigned int *regsptr);
unsigned long long int h2_vmtrap_timerop(timerop_type op, unsigned long long int timeout);
int h2_vmtrap_pmuctrl(pmuop_type op, unsigned int id, unsigned int arg1, unsigned int arg2);
int h2_vmtrap_info(info_type type);

#endif

