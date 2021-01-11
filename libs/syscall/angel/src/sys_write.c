/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"

#ifdef SYS_WRITE_BUF
extern char H2_ANGEL_write_buf[];
extern unsigned int H2_ANGEL_write_buf_idx;
extern const unsigned int H2_ANGEL_write_buf_size;

static inline void dccleana(const char *addr)
{
	asm volatile (" dccleana(%0)" : : "r"(addr));
}
#endif

count_t sys_write(fd_t fd, const char *buffer, count_t count)
{
	struct { fd_t fd; const char *buf; count_t c; } x;
	count_t angel_ret;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.c = count;
	clean(buffer,count/4+3);
	clean(&x,3);
	angel_ret = ANGEL(SYS_WRITE,&x,0);
#ifdef SYS_WRITE_BUF
	if (fd > 2) return angel_ret;
	if (H2_ANGEL_write_buf_idx+count < H2_ANGEL_write_buf_size) {
		memcpy(H2_ANGEL_write_buf+H2_ANGEL_write_buf_idx,buffer,count);
		dccleana(H2_ANGEL_write_buf+H2_ANGEL_write_buf_idx);
		H2_ANGEL_write_buf_idx += count;
		dccleana(H2_ANGEL_write_buf+H2_ANGEL_write_buf_idx);
		if (H2_ANGEL_write_buf_idx == H2_ANGEL_write_buf_size) H2_ANGEL_write_buf_idx = 0;
	} else {
		int i;
		for (i = 0; i < count; i++) {
			H2_ANGEL_write_buf[H2_ANGEL_write_buf_idx++] = buffer[i];
			if (H2_ANGEL_write_buf_idx == H2_ANGEL_write_buf_size) H2_ANGEL_write_buf_idx = 0;
		}
	}
#endif
	return angel_ret;
}

