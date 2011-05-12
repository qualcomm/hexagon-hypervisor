/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONFIG_H
#define H2K_CONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <vm.h>

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_vmblock_size(u32_t unused, u8_t num_cpus, u16_t num_ints, H2K_thread_context *me) IN_SECTION(".text.config.config");

H2K_vmblock_t *H2K_trap_config_vmblock_init(u32_t unused, void *ptr, vmblock_init_op_t op, u32_t arg1, u32_t u32_t arg2, H2K_thread_context *me) IN_SECTION(".text.config.config");

#endif

