/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMINT_H
#define H2K_VMINT_H 1

#include <c_std.h>
#include <vm.h>
#include <context.h>

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

void  H2K_vm_interrupt_post(H2K_vmblock_t *vmblock, u8_t first_cpu, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_clear(H2K_vmblock_t *vmblock, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_enable(H2K_vmblock_t *vmblock, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_disable(H2K_vmblock_t *vmblock, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_localmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_localunmask(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno) IN_SECTION(".text.vm.int");
void  H2K_vm_interrupt_setaffinity(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_interrupt_get(H2K_vmblock_t *vmblock, u8_t cpu) IN_SECTION(".text.vm.int");
s32_t H2K_vm_interrupt_peek(H2K_vmblock_t *vmblock, u8_t cpu) IN_SECTION(".text.vm.int");

/* EOI?  Mainly an enable... */

u32_t H2K_vm_interrupt_status(H2K_vmblock_t *vmblock, u8_t cpu, u32_t intno) IN_SECTION(".text.vm.int");

/* Trap multi-call */
void H2K_vmtrap_intop(H2K_thread_context *me) IN_SECTION(".text.vm.int");

u32_t H2K_enable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int");
u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int");

#endif
