/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_printf.h>
#include <h2_plainmutex.h>
#include <h2_thread.h>
#include <stdarg.h>
#include <stdio.h>

static h2_plainmutex_t printmutex = H2_PLAINMUTEX_T_INIT;

int h2_printf(const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args,fmt);
	h2_plainmutex_lock(&printmutex);
	ret = vprintf(fmt,args);
	h2_plainmutex_unlock(&printmutex);
	va_end(args);
	return ret;
}

