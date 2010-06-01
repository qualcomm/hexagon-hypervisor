/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STOP_H
#define H2K_STOP_H 1

#include <context.h>

void H2K_thread_stop(H2K_thread_context *me) __attribute((noreturn)) IN_SECTION(".text.misc.stop");

#endif

