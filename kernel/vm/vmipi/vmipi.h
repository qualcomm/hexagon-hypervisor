/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMIPI_H
#define H2K_VMIPI_H 1

#include <context.h>

void  H2K_vm_ipi_do(u32_t ipi_intno, H2K_thread_context *me, u32_t hwtnum) IN_SECTION(".text.vm.ipi");
void  H2K_vm_ipi_send(H2K_thread_context *dest) IN_SECTION(".text.vm.ipi");

#endif

