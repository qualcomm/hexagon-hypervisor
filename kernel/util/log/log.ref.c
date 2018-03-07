/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <spinlock.h>
#include <alloc.h>
#include <bzero.h>
#include <max.h>
#include <globals.h>
#include <cache.h>
#include <angel.h>

#ifdef H2K_LOGBUF

static u32_t strcpy(char *dest, const char *src)
{
	u32_t count = 1;

	while ((*dest++ = *src++) != '\0') {
		count++;
	}

	return count;
}

static u32_t strlen(const char *src)
{
	u32_t count = 0;

	while ((*src++) != '\0') {
		count++;
	}

	return count;
}

static u32_t logbuf_space() {

	return LOGBUF_SIZE - H2K_gp->logbuf_pos;
}

static void logbuf_wrap() {

	H2K_gp->logbuf_pos = 0;
}

s32_t H2K_fd_write(u32_t fd, const u8_t *buf, u32_t len) {
	struct req { u32_t fd; u8_t *buf; u32_t c; } x;
	s32_t ret;
	u32_t bytes_left = len;  // left to write
	u32_t bytes_done = 0;

	while (bytes_left > 0) {

		x.fd = fd;
		x.buf = ((u8_t *)buf) - H2K_gp->phys_offset + bytes_done;
		x.c = bytes_left;
		H2K_dccleana_range((void *)buf + bytes_done, bytes_left);
		H2K_dccleana_range((void *)&x, sizeof(struct req));

		ret = H2K_trap_angel(SYS_WRITE, (void *)(((u32_t)&x) - H2K_gp->phys_offset), 0);

		if (ret < 0) {  // error
			return ret;
		}
		bytes_left = ret;  // angel transactor apparently gets this backwards from write()
		bytes_done = len - bytes_left;
	}
	return len;
}

u32_t H2K_log_string(const char *string) {

	u32_t count = 0;
	u32_t len = strlen(string);
	s32_t ret;

	H2K_spinlock_lock(&H2K_gp->log_lock);

	if (len > logbuf_space()) {
		logbuf_wrap();
	}
	count = strcpy(H2K_gp->logbuf + H2K_gp->logbuf_pos, string);
	H2K_gp->logbuf_pos += count;

	ret = H2K_fd_write(1, (const u8_t *)"H2K: ", 5);
	if (ret < 0) goto error;

	ret = H2K_fd_write(1, (const u8_t *)string, len);
	if (ret < 0) goto error;

	H2K_spinlock_unlock(&H2K_gp->log_lock);
	return count;

 error:
	H2K_spinlock_unlock(&H2K_gp->log_lock);
	return ret;
}

char *H2K_log_init() {

	if (NULL == (H2K_gp->logbuf = (char *)H2K_mem_alloc(LOGBUF_SIZE))) {
		return NULL;
	}
	H2K_bzero(H2K_gp->logbuf, LOGBUF_SIZE);
	
	H2K_gp->logbuf_pos = 0;

	H2K_spinlock_init(&H2K_gp->log_lock);

	return H2K_gp->logbuf;
}

#endif
