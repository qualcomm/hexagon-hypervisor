/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMEVENT_H
#define H2K_VMEVENT_H 1

#include <c_std.h>
#include <context.h>

void H2K_vm_event(u32_t gbadva, u32_t cause, u32_t vec_offset, H2K_thread_context *me) IN_SECTION(".text.vm.event");

#endif
