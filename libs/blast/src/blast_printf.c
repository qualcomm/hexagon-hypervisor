/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast_printf.h>
#include <blast_mutex.h>
#include <stdarg.h>
#include <stdio.h>

static blast_mutex_t printmutex = 0;

int blast_printf(const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args,fmt);
	blast_mutex_lock(&printmutex);
	ret = vprintf(fmt,args);
	blast_mutex_unlock(&printmutex);
	va_end(args);
	return ret;
}

