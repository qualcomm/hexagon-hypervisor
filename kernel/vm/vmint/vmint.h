/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMINT_H
#define H2K_VMINT_H 1

#ifndef ASM

#include <c_std.h>
#include <vm.h>
#include <context.h>
#include <h2_common_vmint.h>

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
s32_t H2K_vm_int_deliver(H2K_vmblock_t *block, H2K_thread_context *dst, u32_t intno) IN_SECTION(".text.vm.int");
s32_t H2K_vm_int_deliver_locked(H2K_vmblock_t *block, H2K_thread_context *dst, u32_t intno) IN_SECTION(".text.vm.int");

/* Trap multi-call */
void H2K_vmtrap_intop(H2K_thread_context *me) IN_SECTION(".text.vm.int");

#else
	// ASM
#define INTOP_INDEX_nop         0
#define INTOP_INDEX_enable      1
#define INTOP_INDEX_disable     2
#define INTOP_INDEX_localen     3
#define INTOP_INDEX_localdis    4
#define INTOP_INDEX_setaffinity 5
#define INTOP_INDEX_get         6
#define INTOP_INDEX_peek        7
#define INTOP_INDEX_status      8
#define INTOP_INDEX_post        9
#define INTOP_INDEX_clear       10

#define INTINFO_SIZE 8
#define INTINFO_num_ints 0
#define INTINFO_handlers 4

#endif

#endif
