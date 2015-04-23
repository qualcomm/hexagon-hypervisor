/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

errno_t sys_ftell(fd_t fd) { errno_t ret; clean(&fd,1); ret = ANGEL(SYS_FTELL,&fd,0); DEBUG_PRINTF("sys_ftell: fd=%d ret=%d\n",fd,ret); return ret; }

