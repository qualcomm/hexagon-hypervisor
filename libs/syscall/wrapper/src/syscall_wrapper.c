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

sys_call_ret_t sys_rmdir_internal(const char *name);

__attribute__((weak)) int rmdir(const char *path) {
	sys_call_ret_t res = sys_rmdir_internal(path);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

/* lstat: the Angel/semihosting filesystem has no symbolic links, so lstat is
 * equivalent to stat (it never has to avoid following a link). */
sys_call_ret_t sys_stat_internal(const char *name, void *buffer);

__attribute__((weak)) int lstat(const char *__restrict __path, struct stat *__restrict __sbuf) {
	sys_call_ret_t res = sys_stat_internal(__path, __sbuf);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	return ret;
}

errno_t sys_tmpnam(char *buffer, unsigned char target, count_t count);

__attribute__((weak)) char *tmpnam(char *s) {
	static char buf[L_tmpnam];
	char *out = s ? s : buf;
	if (sys_tmpnam(out, 0, L_tmpnam) != 0) {
		SET_LTS_ERROR(-1, ENOENT);
		return NULL;
	}
	return out;
}

okay_t sys_system(const char *command);

__attribute__((weak)) int system(const char *command) {
	/* A NULL command is the POSIX query for a command processor; report none. */
	if (!command)
		return 0;
	return sys_system(command);
}

void *sys_mmap(void *addr, size_t len, int prot, int flags, int fildes, long off);
int sys_munmap(void *addr, size_t len);
int sys_mprotect(void *addr, size_t len, int prot);

__attribute__((weak)) void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
	return sys_mmap(addr, len, prot, flags, fildes, (long)off);
}

__attribute__((weak)) int munmap(void *addr, size_t len) {
	return sys_munmap(addr, len);
}

__attribute__((weak)) int mprotect(void *addr, size_t len, int prot) {
	return sys_mprotect(addr, len, prot);
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

sys_call_ret_t sys_fstat_internal(int fd, void *buffer);

__attribute__((weak)) int fstat(int fd, struct stat *__sbuf) {

	struct __sys_stat hexstat;
	sys_call_ret_t res = sys_fstat_internal(fd, &hexstat);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	if(ret >= 0)
        {
          memset(__sbuf, 0, sizeof(*__sbuf));
          __sbuf->st_dev = hexstat.dev;
          __sbuf->st_ino = hexstat.ino;
          __sbuf->st_mode = hexstat.mode;
          __sbuf->st_nlink = hexstat.nlink;
          __sbuf->st_rdev = hexstat.rdev;
          __sbuf->st_size = hexstat.size;
          __sbuf->st_atime = hexstat.atime;
          __sbuf->st_mtime = hexstat.mtime;
          __sbuf->st_ctime = hexstat.ctime;
        }
	return ret;
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
	struct __sys_stat hexstat;
	sys_call_ret_t res = sys_stat_internal(__path, &hexstat);
	int ret = (int)res.ret_value;
	SET_LTS_ERROR(ret, (errno_t)res.err_value);
	if(ret >= 0)
        {
          memset(__sbuf, 0, sizeof(*__sbuf));
          __sbuf->st_dev = hexstat.dev;
          __sbuf->st_ino = hexstat.ino;
          __sbuf->st_mode = hexstat.mode;
          __sbuf->st_nlink = hexstat.nlink;
          __sbuf->st_rdev = hexstat.rdev;
          __sbuf->st_size = hexstat.size;
          __sbuf->st_atime = hexstat.atime;
          __sbuf->st_mtime = hexstat.mtime;
          __sbuf->st_ctime = hexstat.ctime;
        }
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

__attribute__((weak)) long sysconf(int name) {
	(void)name;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int truncate(const char *path, off_t length) {
	(void)path;
	(void)length;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) char *realpath(const char *__restrict path, char *__restrict resolved_path) {
	(void)path;
	(void)resolved_path;
	SET_LTS_ERROR(-1, ENOSYS);
	return NULL;
}

__attribute__((weak)) int openat(int dirfd, const char *pathname, int flags, ...) {
	(void)dirfd;
	(void)pathname;
	(void)flags;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int unlinkat(int dirfd, const char *pathname, int flags) {
	(void)dirfd;
	(void)pathname;
	(void)flags;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int fchmod(int fd, mode_t mode) {
	(void)fd;
	(void)mode;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags) {
	(void)dirfd;
	(void)pathname;
	(void)mode;
	(void)flags;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int utimes(const char *path, const struct timeval times[2]) {
	(void)path;
	(void)times;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int link(const char *path1, const char *path2) {
	(void)path1;
	(void)path2;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) int symlink(const char *name1, const char *name2) {
	(void)name1;
	(void)name2;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) ssize_t readlink(const char *__restrict path, char *__restrict buf, size_t bufsize) {
	(void)path;
	(void)buf;
	(void)bufsize;
	SET_LTS_ERROR(-1, ENOSYS);
	return -1;
}

__attribute__((weak)) DIR *fdopendir(int fd) {
	(void)fd;
	SET_LTS_ERROR(-1, ENOSYS);
	return NULL;
}

/* libh2.a is still compiled with older libc which defines errno as a macro that
 * expands to _Geterrno we can scrap this symbol once we start building H2 with
 * Picolibc
 */
int _Geterrno(void) { return errno; }
