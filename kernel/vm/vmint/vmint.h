/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMINT_H
#define H2K_VMINT_H 1

#include <vm.h>

void  H2K_vm_interrupt_post(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno);
void  H2K_vm_interrupt_enable(H2K_vmblock_t *vmblock, u32_t intno);
void  H2K_vm_interrupt_disable(H2K_vmblock_t *vmblock, u32_t intno);
void  H2K_vm_interrupt_localmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno);
void  H2K_vm_interrupt_localunmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno);
void  H2K_vm_interrupt_setaffinity(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno);
s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, u8_t cpu);

#endif
