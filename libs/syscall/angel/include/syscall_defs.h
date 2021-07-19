/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SYSCALL_DEFS_H
#define SYSCALL_DEFS_H 1

#define SIZE__boot_cmdline__ 4096
#define SIZE__dir_prefix__ 1024
#define SIZE__file_suffix__ 16

typedef enum {
	H2_SYS_WRITE_MODE_NORMAL,
	H2_SYS_WRITE_MODE_NO_FD1,
	H2_SYS_WRITE_MODE_ONLY_FD1,
	H2_SYS_WRITE_MODE_SILENT
} sys_write_mode;

#endif
