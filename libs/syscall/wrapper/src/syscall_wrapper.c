/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "syscall_wrapper.h"
#include "syscall_defs.h"
#include <angel.h>

__attribute__((weak)) int open(const char *pathname, int flags, ...) {
	return sys_open(pathname, flags);
}

__attribute__((weak)) int close(int fd) {
	return sys_close(fd);
}

__attribute__((weak)) ssize_t read(int fd, void *buf, size_t count) {
	return sys_read(fd, buf, count);
}

__attribute__((weak)) ssize_t write(int fd, const void *buf, size_t count) {
    return sys_write(fd, buf, count);
}

__attribute__((weak)) off_t lseek(int fd, off_t offset, int whence) {
	switch(whence) {
	case SEEK_SET:
		sys_seek(fd, offset);
		break;
	case SEEK_CUR: {
		errno_t cur = sys_ftell(fd);
		sys_seek(fd, cur + offset);
	}
		break;
	case SEEK_END: {
		errno_t len = sys_flen(fd);
		sys_seek(fd, len + offset);
	}
		break;
	default:
		break;
	}
	return sys_ftell(fd);
}

__attribute__((weak)) int fstat(int fd, struct stat *statbuf) {
	return sys_fstat(fd, statbuf);
}

__attribute__((weak)) void _exit(int status) {
	sys_exit(status);
	__builtin_unreachable();
}

__attribute__((weak)) int flen(int fd) {
	return sys_flen(fd);
}

__attribute__((weak)) int ftell(int fd) {
	return sys_ftell(fd);
}

__attribute__((weak)) clock_t times(struct tms *buf) {
	count_t time = sys_time();
	// TODO: add support of tms struct
	return time;
}

#if 0 //TODO: Using of thread local variable brakes Picolibc: printf shows nothing, fprintf shows on the screen instead of to be written into the file
static _Thread_local char __cmd_line__ [SIZE__boot_cmdline__];
__attribute__((weak)) char *sys_semihost_get_cmdline(void) {
	__cmd_line__[0] = '\0';
	sys_get_cmdline(__cmd_line__, sizeof(__cmd_line__)/sizeof(__cmd_line__[0]));
	__cmd_line__[sizeof(__cmd_line__)/sizeof(__cmd_line__[0]) - 1] = '\0';
    return __cmd_line__;
}
#else
// STUB
__attribute__((weak)) char *sys_semihost_get_cmdline(void) {
	return "";
}
#endif

#if 0 //TODO: fix flags of picolibc compilation
__attribute__((weak)) int errno(void) {
	return sys_errno();
}
#endif

// STUBS

__attribute__((weak)) int unlink (const char *__path) { // Need to be provided to mktemp, mkstemp etc
	return -1;
}

__attribute__((weak)) off64_t lseek64(int fd, off64_t offset, int whence) {
	return -1;
}

__attribute__((weak)) int isatty (int __fildes) {
	return 0;
}
