/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

count_t sys_read(fd_t fd, char *buffer, count_t count);
errno_t sys_seek(fd_t fd, count_t offset);

void *sys_mmap(void *addr, size_t len, int prot, int flags, int fildes, long off)
{
	(void)prot;

	int is_fixed = flags & MAP_FIXED;
	int is_anon  = flags & (MAP_ANON | MAP_ANONYMOUS);

	if (is_fixed) {
		if (is_anon) {
			memset(addr, 0, len);
		} else {
			sys_seek(fildes, off);
			sys_read(fildes, addr, len);
		}
		return addr;
	}

	void *ptr = memalign(4096, len);
	if (!ptr)
		return MAP_FAILED;

	if (is_anon) {
		memset(ptr, 0, len);
	} else {
		sys_seek(fildes, off);
		sys_read(fildes, ptr, len);
	}
	return ptr;
}

int sys_munmap(void *addr, size_t len)
{
	/* dlclose not supported; leaking is acceptable for now */
	(void)addr;
	(void)len;
	return 0;
}

int sys_mprotect(void *addr, size_t len, int prot)
{
	(void)addr;
	(void)len;
	(void)prot;
	return 0;
}
