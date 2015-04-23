/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

errno_t sys_errno() { int x = 0; clean(&x,1); return ANGEL(SYS_ERRNO,&x,0); }

