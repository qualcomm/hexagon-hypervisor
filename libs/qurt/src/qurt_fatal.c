/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <stdlib.h>

void qurt_exception_raise_fatal()
{
	qurt_printf("FATAL: EJP: just going to call exit here.\n");
	exit(1);
	while (1) /* SPIN */;
}

void qurt_assert_error(const char *filename, int lineno)
{
	qurt_printf("assertion fail @ %s:%d\n",filename,lineno);
	qurt_exception_raise_fatal();
}

