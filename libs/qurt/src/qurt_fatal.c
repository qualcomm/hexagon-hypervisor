/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <stdlib.h>

void qurt_exception_raise_fatal()
{
	qurt_printf("FATAL: from %p<%p<%p: EJP: crashing later?\n",__builtin_return_address(0),__builtin_return_address(1),__builtin_return_address(2));
	// h2_fatal_crash();
	// while (1) /* SPIN */;
}

void qurt_exception_shutdown_fatal()
{
	qurt_printf("Shutdown fatal.\n");
	h2_fatal_crash();
}

void qurt_assert_error(const char *filename, int lineno)
{
	qurt_printf("assertion fail @ %s:%d\n",filename,lineno);
	qurt_exception_shutdown_fatal();
	while (1) /* SPIN */;
}

unsigned int qurt_exception_register_fatal_notification ( void(*entryfuncpoint)(void *), void *argp)
{
	return h2_config_fatal_hook((unsigned int)entryfuncpoint, (unsigned int)argp);
}

