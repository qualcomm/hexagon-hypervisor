/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

okay_t sys_iserror(errno_t errcode) { clean(&errcode,1); return ANGEL(SYS_ISERROR,&errcode,0); }

