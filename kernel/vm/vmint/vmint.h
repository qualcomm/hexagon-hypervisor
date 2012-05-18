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
	H2K_INTOP_CLEAR,
	H2K_INTOP_FIRST_INVALID_OP
} intop_type;

typedef s32_t (*H2K_vm_int_op_fn_t)(H2K_vmblock_t *vmblock, 
		H2K_thread_context *me, u32_t intno, struct H2K_vm_int_opinfo_struct *info);

#if 0
typedef struct H2K_vm_int_ops_struct {
	s32_t (*nop)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t unused);
	s32_t (*post)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*clear)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*enable)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*disable)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*localmask)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*localunmask)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*setaffinity)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
	s32_t (*get)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset);
	s32_t (*peek)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t offset);
	u32_t (*status)(H2K_vmblock_t *vmblock, H2K_thread_context *me, u32_t intno);
} H2K_vm_int_ops_t;
#else
typedef struct H2K_vm_int_ops_struct {
	H2K_vm_int_op_fn_t nop;
	H2K_vm_int_op_fn_t enable;
	H2K_vm_int_op_fn_t disable;
	H2K_vm_int_op_fn_t localen;
	H2K_vm_int_op_fn_t localdis;
	H2K_vm_int_op_fn_t setaffinity;
	H2K_vm_int_op_fn_t get;
	H2K_vm_int_op_fn_t peek;
	H2K_vm_int_op_fn_t status;
	H2K_vm_int_op_fn_t post;
	H2K_vm_int_op_fn_t clear;
} H2K_vm_int_ops_t;
#endif

typedef struct H2K_vm_int_opinfo_struct {
	u32_t num_ints;
	union {
		const H2K_vm_int_ops_t *handlers;
		const H2K_vm_int_op_fn_t *optab;
	};
} H2K_vm_int_opinfo_t;

void H2K_vm_int_intinfo_init(H2K_vmblock_t *vmblock, u32_t num_ints) IN_SECTION(".text.config.config");

u32_t H2K_enable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int");
u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int");
s32_t H2K_vm_check_interrupts(H2K_thread_context *me) IN_SECTION(".text.vm.int");
void H2K_vm_int_deliver(H2K_vmblock_t *block, H2K_thread_context *dst, u32_t intno) IN_SECTION(".text.vm.int");
void H2K_vm_int_deliver_locked(H2K_vmblock_t *block, H2K_thread_context *dst, u32_t intno) IN_SECTION(".text.vm.int");

/* Trap multi-call */
void H2K_vmtrap_intop(H2K_thread_context *me) IN_SECTION(".text.vm.int");

#endif
