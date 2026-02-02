/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "syscall_wrapper.h"
#include "syscall_defs.h"
#include <angel.h>

// Space holder for error code translation (SYS error code to libc error code)
static int sys_error_translation(errno_t err) {
	return err;
}

sys_call_ret_t sys_open_internal(const char *name, t_mode_t mode);

__attribute__((weak)) int open(const char *pathname, int flags, ...) {
	sys_call_ret_t res = sys_open_internal(pathname, flags);
	int ret = res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_close_internal(fd_t fd);

__attribute__((weak)) int close(int fd) {
	sys_call_ret_t res = sys_close_internal(fd);
	int ret = res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t  sys_read_internal(fd_t fd, char *buffer, count_t count);

__attribute__((weak)) ssize_t read(int fd, void *buf, size_t count) {
	sys_call_ret_t res = sys_read_internal(fd, buf, count);
	int ret = (ssize_t)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return count - ret; // According to documentation there should be, but doesn't work: return ret == 0 ? count : 0;
}

sys_call_ret_t sys_write_internal(fd_t fd, const char *buffer, count_t count);

__attribute__((weak)) ssize_t write(int fd, const void *buf, size_t count) {
	sys_call_ret_t res = sys_write_internal(fd, buf, count);
	int ret = (ssize_t)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return count - ret; // According to documentation there should be, but doesn't work: return ret == 0 ? count : 0;
}

sys_call_ret_t sys_flen_internal(fd_t fd);

__attribute__((weak)) int flen(int fd) {
	sys_call_ret_t res = sys_flen_internal(fd);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_ftell_internal(fd_t fd);

__attribute__((weak)) int ftell(int fd) {
	sys_call_ret_t res = sys_ftell_internal(fd);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_seek_internal(fd_t fd, count_t offset);

__attribute__((weak)) off_t lseek(int fd, off_t offset, int whence) {
	off_t current_offset = -1;

	switch(whence) {
	case SEEK_CUR:
		current_offset = ftell(fd);
		break;
	case SEEK_END:
		current_offset = flen(fd);
		break;
	case SEEK_SET:
		current_offset = 0;
		break;
	default:
		SET_LTS_ERROR(-1, ENXIO);
		break;
	}

	if (current_offset == -1)
		return -1;

	sys_seek_internal(fd, current_offset + offset);

	return sys_ftell(fd);
}

__attribute__((weak)) int fstat(int fd, struct stat *statbuf) {
	return sys_fstat(fd, statbuf);
}

__attribute__((weak)) void _exit(int status) {
	sys_exit(status);
	__builtin_unreachable();
}

__attribute__((weak)) clock_t times(struct tms *buf) {
	count_t time = sys_time();
	// FIXME: add support of tms struct
	(void)buf;
	return time;
}

sys_call_ret_t sys_isatty_internal(void);

__attribute__((weak)) int isatty(int __fildes) {
	sys_call_ret_t res = sys_isatty_internal();
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_unlink_internal(const char *name);

__attribute__((weak)) int unlink (const char *__path) { // Need to be provided to mktemp, mkstemp etc
	sys_call_ret_t res = sys_unlink_internal(__path);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

__attribute__((weak)) int get_cmdline(char *buffer, int count) {
	if (count < 1)
		return -1;
	buffer[0] = '\0';
	errno_t res = sys_get_cmdline(buffer, count);
	buffer[count - 1] = '\0';
	return res;
}

sys_call_ret_t sys_stat_internal(const char *name, void *buffer);

__attribute__((weak)) int stat(const char    *__restrict __path, struct stat    *__restrict __sbuf) {
	sys_call_ret_t res = sys_stat_internal(__path, __sbuf);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

#ifdef __PICOLIBC_ERRNO_FUNCTION
__attribute__((weak)) int errno(void) {
	return sys_error_translation(sys_errno());
}
#endif

// STUBS FIXME: add support of missed system calls

__attribute__((weak)) off64_t lseek64(int fd, off64_t offset, int whence) {
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int getentropy(void *buffer, size_t length) {
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int gettimeofday(struct timeval * __restrict __p, void * __restrict __tz) {
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}
