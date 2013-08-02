/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef THREAD_H
#define THREAD_H 1

#include <context.h>

void H2K_thread_init() IN_SECTION(".text.init.thread");
void H2K_thread_context_clear(H2K_thread_context *thread) IN_SECTION(".text.misc.thread");

#ifdef DO_EXT_SWITCH
void H2K_ext_context_clear(H2K_ext_context *ext) IN_SECTION(".text.misc.thread");
#endif

#endif

