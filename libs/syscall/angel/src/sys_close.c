/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

errno_t sys_close(fd_t fd) { clean(&fd,1); return ANGEL(SYS_CLOSE,&fd,0); }

