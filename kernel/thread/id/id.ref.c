/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>

IN_SECTION(".text.core.id") u32_t H2K_thread_id(H2K_thread_context *me)
{       
        return (u32_t)me;
}

