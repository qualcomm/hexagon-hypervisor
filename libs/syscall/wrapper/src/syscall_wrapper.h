/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SYSCALL_WRAPPER
#define H2_SYSCALL_WRAPPER

// Picolibc headers
#define __machine_mbstate_t_defined

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/times.h>
#include <sys/errno.h>

#ifdef __PICOLIBC_ERRNO_FUNCTION
#define SET_LTS_ERROR(err)
#else
#define SET_LTS_ERROR(err) do {if (err < 0) errno = sys_error_translation((err));} while(0)
#endif

#endif /*H2_SYSCALL_WRAPPER*/
