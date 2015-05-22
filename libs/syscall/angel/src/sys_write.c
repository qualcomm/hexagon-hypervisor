/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

#define WRITE_BUFSIZE (1024*256)
static char write_buf[WRITE_BUFSIZE] __attribute__((aligned(64))) = { 0 };
static unsigned int write_buf_idx = 0;

count_t sys_write(fd_t fd, const char *buffer, count_t count)
{
	struct { fd_t fd; const char *buf; count_t c; } x;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.c = count;
	clean(buffer,count/4+3);
	clean(&x,3);
	ANGEL(SYS_WRITE,&x,0);
	if (fd > 2) return -1;
	if (write_buf_idx+count < WRITE_BUFSIZE) {
		memcpy(write_buf+write_buf_idx,buffer,count);
		write_buf_idx += count;
		if (write_buf_idx == WRITE_BUFSIZE) write_buf_idx = 0;
	} else {
		int i;
		for (i = 0; i < count; i++) {
			write_buf[write_buf_idx++] = buffer[i];
			if (write_buf_idx == WRITE_BUFSIZE) write_buf_idx = 0;
		}
	}
	return count;
}

