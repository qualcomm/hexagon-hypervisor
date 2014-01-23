/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STATE_H
#define H2K_STATE_H 1

#include <context.h>
#include <idtype.h>

u64_t H2K_thread_state(H2K_id_t id, u32_t offset, H2K_thread_context *me);

#endif
