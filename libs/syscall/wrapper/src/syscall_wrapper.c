/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "syscall_wrapper.h"
#include "syscall_defs.h"
#include <angel.h>
#include <stdlib.h>

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

sys_call_ret_t sys_rename_internal(const char *oldname, const char *newname);

__attribute__((weak)) int rename(const char *old_name, const char *new_name) {
	sys_call_ret_t res = sys_rename_internal(old_name, new_name);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_mkdir_internal(const char *name, int mode);

__attribute__((weak)) int mkdir(const char *path, mode_t mode) {
	sys_call_ret_t res = sys_mkdir_internal(path, mode);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

sys_call_ret_t sys_ftell_internal(fd_t fd);

static int tell_internal(int fd) {
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
		current_offset = tell_internal(fd);
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
        /* file offset is out of range, only 32bit offsets are supported right
         * now */
        if (offset > INT32_MAX || offset < INT32_MIN) {
          errno = EINVAL;
          return -1;
        }

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

sys_call_ret_t sys_opendir_internal(const char *name);
sys_call_ret_t sys_closedir_internal(int dir);
sys_call_ret_t sys_readdir_internal(int dir, dirent_internal *dptr);

__attribute__((weak)) DIR *opendir(const char *name) {
	sys_call_ret_t res = sys_opendir_internal(name);
	int handle = (int)res.ret_value;
	if (handle == -1) {
		SET_LTS_ERROR(-1, (errno_t)res.err_value);
		return NULL;
	}
	DIR *dirp = malloc(sizeof(*dirp));
	if (!dirp) {
		sys_closedir_internal(handle);
		SET_LTS_ERROR(-1, ENOMEM);
		return NULL;
	}
	dirp->fd = handle;
	return dirp;
}

__attribute__((weak)) struct dirent *readdir(DIR *dirp) {
	if (!dirp) {
		SET_LTS_ERROR(-1, EBADF);
		return NULL;
	}
	dirent_internal *ad = (dirent_internal *)dirp->buf;
	sys_call_ret_t res = sys_readdir_internal(dirp->fd, ad);
	if (res.ret_value == 0) {
		if (res.err_value != 0)
			SET_LTS_ERROR(-1, (errno_t)res.err_value);
		return NULL;
	}
	dirp->dirent.d_ino = ad->d_ino;
	dirp->dirent.d_type = DT_UNKNOWN;
	size_t name_max = sizeof(dirp->dirent.d_name) - 1;
	if (name_max > sizeof(ad->d_name) - 1)
		name_max = sizeof(ad->d_name) - 1;
	strncpy(dirp->dirent.d_name, ad->d_name, name_max);
	dirp->dirent.d_name[name_max] = '\0';
	return &dirp->dirent;
}

__attribute__((weak)) int closedir(DIR *dirp) {
	if (!dirp) {
		SET_LTS_ERROR(-1, EBADF);
		return -1;
	}
	sys_call_ret_t res = sys_closedir_internal(dirp->fd);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	free(dirp);
	return ret;
}

// STUBS FIXME: add support of missed system calls

__attribute__((weak)) int getentropy(void *buffer, size_t length) {
	(void)buffer;
	(void)length;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	(void)how;
	(void)set;
	(void)oldset;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int gettimeofday(struct timeval * __restrict __p, void * __restrict __tz) {
	(void) __p;
	(void)__tz;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int access(const char *pathname, int mode) {
	(void)pathname;
	(void)mode;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) pid_t fork(void) {
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int execve(const char *path, char *const argv[], char *const envp[]) {
	(void)path;
	(void)argv;
	(void)envp;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) pid_t waitpid(pid_t pid, int *wstatus, int options) {
	(void)pid;
	(void)wstatus;
	(void)options;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int pipe(int pipefd[2]) {
	(void)pipefd;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int dup2(int oldfd, int newfd) {
	(void)oldfd;
	(void)newfd;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

static char *__h2_empty_environ[] = { NULL };
__attribute__((weak)) char **environ = __h2_empty_environ;

__attribute__((weak)) uid_t getuid(void) {
	return 0;
}

__attribute__((weak)) int clock_getres(clockid_t clk_id, struct timespec *res) {
	(void)clk_id;
	(void)res;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) char *getcwd(char *buf, size_t size) {
	(void)buf;
	(void)size;
	SET_LTS_ERROR(-1, ENOSYS);
	return NULL;
}

__attribute__((weak)) int ftruncate(int fd, off_t length) {
	(void)fd;
	(void)length;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int getrlimit(int resource, struct rlimit *rlim) {
	(void)resource;
	(void)rlim;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int setrlimit(int resource, const struct rlimit *rlim) {
	(void)resource;
	(void)rlim;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int statvfs(const char *path, struct statvfs *buf) {
	(void)path;
	(void)buf;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int fstatvfs(int fd, struct statvfs *buf) {
	(void)fd;
	(void)buf;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

/* libh2.a is still compiled with older libc which defines errno as a macro that
 * expands to _Geterrno we can scrap this symbol once we start building H2 with
 * Picolibc
 */
int _Geterrno(void) { return errno; }
