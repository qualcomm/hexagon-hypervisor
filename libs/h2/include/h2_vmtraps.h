/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_VMTRAPS_H
#define H2_VMTRAPS_H 1

typedef enum {
	H2K_ASID_TRANS_TYPE_LINEAR,
	H2K_ASID_TRANS_TYPE_TABLE,
	H2K_ASID_TRANS_TYPE_XXX_LAST
} translation_type;

typedef enum {
	H2K_INTOP_NOP,
	H2K_INTOP_GLOBEN,
	H2K_INTOP_GLOBDIS,
	H2K_INTOP_LOCEN,
	H2K_INTOP_LOCDIS,
	H2K_INTOP_AFFINITY,
	H2K_INTOP_GET,
	H2K_INTOP_PEEK,
	H2K_INTOP_STATUS,
	H2K_INTOP_POST,
	H2K_INTOP_CLEAR
} intop_type;

void h2_vmtrap_return();
int h2_vmtrap_setvec(void *ptr);
int h2_vmtrap_setie(unsigned int val);
int h2_vmtrap_getie();
int h2_vmtrap_intop(intop_type op, unsigned int arg1, unsigned int arg2);
int h2_vmtrap_clrmap(void *ptr);
int h2_vmtrap_newmap(void *newbase, translation_type type);
int h2_vmtrap_cachectl(unsigned int op, void *start, unsigned int count);
unsigned long long int h2_vmtrap_get_pcycles();
void h2_vmtrap_set_pcycles(unsigned long long int);
int h2_vmtrap_wait();
void h2_vmtrap_yield();
int h2_vmtrap_start(void *pc, void *stack);
void h2_vmtrap_stop();
int h2_vmtrap_vmpid();
void h2_vmtrap_setregs(unsigned int g0, unsigned int g1, unsigned int g2, unsigned int g3);
void h2_vmtrap_getregs(unsigned int *regsptr);

#endif

