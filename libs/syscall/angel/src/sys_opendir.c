/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
int sys_opendir(const char *name) { clean_str(name); return ANGEL(SYS_OPENDIR, name, 0); }

