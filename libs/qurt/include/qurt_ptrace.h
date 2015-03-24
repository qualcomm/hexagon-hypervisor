/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __SYS_PTRACE_H__
#define __SYS_PTRACE_H__

enum __ptrace_request
{
    /**
        @brief Indicate that the process making this request is requesting to be traced.
    */
	PTRACE_TRACEME = 0,
	PTRACE_EXT_IS_DEBUG_PERMITTED = 500
};

static inline long ptrace(enum __ptrace_request request, unsigned int pid, void*addr, void *data) { UNSUPPORTED; }

#endif //__SYS_PTRACE_H__
