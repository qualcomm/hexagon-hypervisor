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
