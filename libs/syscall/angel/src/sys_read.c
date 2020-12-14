/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

#ifdef SYS_READ_DEBUG
#include <h2_vmtraps.h>
#include "stdio.h"
#endif

count_t sys_read(fd_t fd, char *buffer, count_t count)
{
	count_t ret;
	struct { fd_t fd; char *buf; count_t count; } x;
	unsigned long start,end;
	start = ALIGN32_DOWN((unsigned long)buffer);
	end = ALIGN32_UP((unsigned long)buffer + count);
	invalidate((void *)start, end-start);
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.fd = fd;
	x.count = count;
	clean(&x,3);
#ifdef SYS_READ_DEBUG
	unsigned long long int start_time, end_time;
	asm volatile ( " %0 = c31:30 // READ TIMER \n" : "=r"(start_time));
#endif
	ret = ANGEL(SYS_READ,&x,0);
#ifdef SYS_READ_DEBUG
	asm volatile ( " %0 = c31:30 // READ TIMER \n" : "=r"(end_time));
	printf("SYS_READ_DEBUG: angel read %d bytes in %llu nsecs\n", count, (end_time - start_time) * 52);
#endif
	return ret;
}

