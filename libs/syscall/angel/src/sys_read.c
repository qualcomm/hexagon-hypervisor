/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

#ifdef SYS_READ_DEBUG
#include "stdio.h"
#endif

static sys_call_ret_t do_sys_read(fd_t fd, char *buffer, count_t count)
{
	sys_call_ret_t  ret;
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
	ret = angel_with_err(SYS_READ,&x,0);
#ifdef SYS_READ_DEBUG
	asm volatile ( " %0 = c31:30 // READ TIMER \n" : "=r"(end_time));
	printf("SYS_READ_DEBUG: angel read %d bytes in %llu nsecs\n", count, (end_time - start_time) * 52);
#endif
	return ret;
}

/* Trampoline to align sp down to next cache line. This prevents stack
	 allocations further down the call tree from falling in the same
	 cache line as the buffer that needs to be invalidated prior to
	 angel SYS_READ, which can foil the invalidation. */
sys_call_ret_t  sys_read_internal(fd_t fd, char *buffer, count_t count) {
	unsigned long sp_save;
	/*FIXME: Maybe we should get the actual cache line size from cfg_table and save it in a global */
	asm volatile ( " %0 = r29  // save sp \n"
								 " r29 = and(r29, #-256)  // align sp down \n"
								 : "=r"(sp_save));
	return do_sys_read(fd, buffer, count);
}

count_t sys_read(fd_t fd, char *buffer, count_t count) {
	return sys_read_internal(fd, buffer, count).ret_value;
}
