/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SWITCH_H
#define H2K_SWITCH_H 1

#include <context.h>

__attribute__((noreturn)) void H2K_switch(H2K_thread_context *from, H2K_thread_context *to) IN_SECTION(".text.core.switch");

#endif

