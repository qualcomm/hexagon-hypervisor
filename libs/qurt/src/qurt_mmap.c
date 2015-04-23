/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <stdlib.h>

void *sys_mmap(void *addr, size_t len, int prot, int flags, int fildes, long off)
{
	UNSUPPORTED;
	return NULL;
}

int sys_munmap(void *addr, size_t len)
{
	R_UNSUPPORTED;
}

int sys_mprotect(void *addr, size_t len, int prot) 
{
	R_UNSUPPORTED;
}

