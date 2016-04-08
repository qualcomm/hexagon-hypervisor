/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

int sys_mkdir(char *name, int mode) { clean_str(name); return ANGEL(SYS_MKDIR, name, mode); }

