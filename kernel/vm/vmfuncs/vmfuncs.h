/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMFUNCS_H
#define H2K_VMFUNCS_H 1

#include <c_std.h>
#include <context.h>

void H2K_vmtrap_version(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_return(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_setvec(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_setie(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_getie(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_get_pcycles(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_set_pcycles(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_wait(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_yield(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_start(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_stop(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_vmpid(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_setregs(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_getregs(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");
void H2K_vmtrap_pmuconfig(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");

#endif
