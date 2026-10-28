/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "allsyscalls.h"
#include "syscall_defs.h"

extern char H2_ANGEL_write_buf[];
extern unsigned int H2_ANGEL_write_buf_idx;
extern const unsigned int H2_ANGEL_write_buf_size;


int __sys_write_mode__;

#ifdef H2_STDOUT_STDERR_DEBUG_RING_BUFFER

#include <pthread.h>
/*
 * Serializes the H2_ANGEL_write_buf mirror ring below.  Any multithreaded
 * guest has concurrent fd<=2 writers here; each writer only serializes with
 * the others across the semihosting trap itself, not across this
 * mirror-ring update that runs after it returns.  Unserialized, the
 * H2_ANGEL_write_buf_idx bookkeeping ("+= count", the byte-loop's "++" +
 * wrap check) is racy: one thread can observe idx == H2_ANGEL_write_buf_size
 * an instant before the wrap-reset runs, write one byte past the buffer,
 * and leave idx stuck past size forever after -- every later write then
 * walks further out of bounds without limit.
 */
static pthread_plainmutex_t angel_mirror_lock = PTHREAD_PLAINMUTEX_INITIALIZER_NP;

#ifndef DCACHE_LINE_SIZE
#define DCACHE_LINE_SIZE 64U  //FIXME: get it from a header
#endif

static inline void dccleana(const void *addr)
{
	asm volatile (" dccleana(%0)" : : "r"(addr));
}

static inline void dccleana_range(const void *addr, count_t count)
{
	if (count == 0)
		return;

	uintptr_t first =
		((uintptr_t)addr) & ~(DCACHE_LINE_SIZE - 1);

	uintptr_t last =
		((uintptr_t)(addr + count - 1)) &
		~(DCACHE_LINE_SIZE - 1);

	do {
		dccleana((const void*)first);
		first += DCACHE_LINE_SIZE;
	} while (first <= last);
}
#endif

sys_call_ret_t sys_write_internal(fd_t fd, const char *buffer, count_t count)
{
	if (H2_SYS_WRITE_MODE_SILENT == __sys_write_mode__) {
		sys_call_ret_t angel_ret = {0};
		return angel_ret;
	}
	if (1 == fd && H2_SYS_WRITE_MODE_NO_FD1 == __sys_write_mode__) {  // stdout
		sys_call_ret_t angel_ret = {0};
		return angel_ret;
	}
	if (1 != fd && H2_SYS_WRITE_MODE_ONLY_FD1 == __sys_write_mode__) {  // !stdout
		sys_call_ret_t angel_ret = {0};
		return angel_ret;
	}

	struct { fd_t fd; const char *buf; count_t c; } x;
	sys_call_ret_t angel_ret;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.c = count;
	clean(buffer,count/4+3);
	clean(&x,3);
	angel_ret = angel_with_err(SYS_WRITE,&x,0);

#ifdef H2_STDOUT_STDERR_DEBUG_RING_BUFFER
	if (fd > 2) return angel_ret;
	/* count_t is signed: nothing to mirror, and the copies below want count > 0 */
	if (count <= 0) return angel_ret;
	pthread_plainmutex_lock_np(&angel_mirror_lock);
	if (count >= H2_ANGEL_write_buf_size) {
		/*
		 * A write at least as large as the ring can only leave its final
		 * H2_ANGEL_write_buf_size bytes behind, whatever the old index was:
		 * copy just that tail, in one go, laid out linearly from the start
		 * of the ring so a reader needs no unwrapping.
		 */
		memcpy(H2_ANGEL_write_buf, buffer + (count - H2_ANGEL_write_buf_size),
				H2_ANGEL_write_buf_size);
		dccleana_range(H2_ANGEL_write_buf, H2_ANGEL_write_buf_size);
		H2_ANGEL_write_buf_idx = 0;
	} else if (H2_ANGEL_write_buf_idx+count <= H2_ANGEL_write_buf_size) {
		memcpy(H2_ANGEL_write_buf + H2_ANGEL_write_buf_idx, buffer, count);
		dccleana_range(H2_ANGEL_write_buf + H2_ANGEL_write_buf_idx, count);
		H2_ANGEL_write_buf_idx += count;
		if (H2_ANGEL_write_buf_idx == H2_ANGEL_write_buf_size)
			H2_ANGEL_write_buf_idx = 0;
	} else {
		/*
		 * Wrapping copy, split in two: the run that fills the ring up to
		 * its end, then the remainder back at the front.  count < size
		 * here, and reaching this branch means idx+count >= size, so
		 * first <= count and both lengths stay inside the buffer.
		 */
		unsigned int first = H2_ANGEL_write_buf_size - H2_ANGEL_write_buf_idx;

		memcpy(H2_ANGEL_write_buf+H2_ANGEL_write_buf_idx,buffer,first);
		dccleana_range(H2_ANGEL_write_buf+H2_ANGEL_write_buf_idx, first);
		memcpy(H2_ANGEL_write_buf,buffer+first,count-first);
		dccleana_range(H2_ANGEL_write_buf, count-first);
		H2_ANGEL_write_buf_idx = count-first;
	}
	dccleana(&H2_ANGEL_write_buf_idx);
	pthread_plainmutex_unlock_np(&angel_mirror_lock);
#endif
	return angel_ret;
}

count_t sys_write(fd_t fd, const char *buffer, count_t count) {
	return sys_write_internal(fd, buffer, count).ret_value;
}
