/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_printf.h>
#include <h2_mutex.h>
#include <h2_thread.h>
#include <stdarg.h>
#include <stdio.h>

static h2_mutex_t printmutex = 0;

int h2_printf(const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args,fmt);
	h2_mutex_lock(&printmutex);
	ret = vprintf(fmt,args);
	h2_mutex_unlock(&printmutex);
	va_end(args);
	return ret;
}

