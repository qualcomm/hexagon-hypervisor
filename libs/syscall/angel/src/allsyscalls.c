/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <angel.h>
#include <string.h>
#include <stdlib.h>

unsigned int angel(unsigned int r0, void *r1, unsigned int r2) {

	return __angel(r0, ANGEL_OFFSET_PTR(r1), r2);
}

static inline void clean(const void *vx,int words)
{
	const int *x = vx;
	int i;
	for (i = 0; i < words; i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
	asm volatile (" %0 = memb(%1) " : "=r"(i) : "r"(x));
	asm volatile (" dccleaninva(%0) " : : "r"(x));
}

/* X must be 32-byte aligned and COUNT must be a multiple of 32. */
static inline void invalidate(const char *x,count_t count)
{
	count_t i;
	for (i = 0; i < count; i += 32) {
		asm volatile ("dcinva(%0)" : :"r"(x+i):"memory");
	};
}

static inline void clean_str(const char *x)
{
	int len = strlen(x);
	int i;
	for (i = 0; i <= len; i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	}
	asm volatile (" %0 = memb(%1) " : "=r"(i) : "r"(x));
	asm volatile (" dccleaninva(%0) " : : "r"(x));
}

errno_t sys_clock() { int x = 0; return ANGEL(SYS_CLOCK,&x,0); }

errno_t sys_close(fd_t fd) { clean(&fd,1); return ANGEL(SYS_CLOSE,&fd,0); }

int sys_closedir(int dir) { return ANGEL(SYS_CLOSEDIR,dir,0); }

errno_t sys_errno() { int x = 0; clean(&x,1); return ANGEL(SYS_ERRNO,&x,0); }

void sys_exit(okay_t status) { ANGEL(SYS_EXIT,&status,status); }

errno_t sys_flen(fd_t fd) { errno_t ret; clean(&fd,1); ret = ANGEL(SYS_FLEN,&fd,0); DEBUG_PRINTF("sys_flen: fd=%d ret=%d\n",fd,ret); return ret; }

int sys_fstat(fd_t fd, void *buffer)  /* Clean buffer? */
{
	struct {
		fd_t fd;
		void *buf;
	} x;
	x.fd = fd;
	x.buf = buffer;
	clean(&x,2);
	return ANGEL(SYS_FSTAT,&x,0);
}

errno_t sys_ftell(fd_t fd) { errno_t ret; clean(&fd,1); ret = ANGEL(SYS_FTELL,&fd,0); DEBUG_PRINTF("sys_ftell: fd=%d ret=%d\n",fd,ret); return ret; }

unsigned char __boot_cmdline__[256] __attribute__((section(".data"))) = { 0 };

errno_t sys_get_cmdline(char *buffer, count_t count)
{
	errno_t ret;
	struct { char *buf; count_t count; } x;
	if (__boot_cmdline__[0] != 0) {
		strncpy(buffer,__boot_cmdline__,count);
		buffer[count-1] = 0;
		return 0;
	}
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.count = count;
	clean(buffer,count/4+3); clean(&x,2);
	ret = ANGEL(SYS_GET_CMDLINE,&x,0);
	clean(buffer,count/4+3);
	return ret;
}

struct heap_info *sys_heapinfo(struct heap_info *buffer)
{
	struct heap_info *ret;
	clean(&buffer,4);
	ret = VANGEL(SYS_HEAPINFO,&buffer,0);
	clean(&buffer,4);
	return ret;
}

okay_t sys_iserror(errno_t errcode) { clean(&errcode,1); return ANGEL(SYS_ISERROR,&errcode,0); }

int sys_mkdir(char *name, int mode) { clean_str(name); return ANGEL(SYS_MKDIR,ANGEL_OFFSET_PTR(name),mode); }

int sys_opendir(const char *name) { clean_str(name); return ANGEL(SYS_OPENDIR,ANGEL_OFFSET_PTR(name),0); }

count_t sys_read(fd_t fd, char *buffer, count_t count)
{
	count_t ret;
	struct { fd_t fd; char *buf; count_t count; } x;
	char *malloc_ret;
	count_t offset, start, middle, leftover;
	x.fd = fd;
	offset = ((unsigned long)buffer)&31;
	start = (offset == 0) ? 0 : (32-offset > count) ? count : 32-offset;
	middle = (count-start)&-32;
	leftover = count-middle;
	if (middle) {
		x.buf = ANGEL_OFFSET_PTR(buffer+start);
		x.count = middle;
		clean(&x,3);
		invalidate(x.buf,middle);
		ret = ANGEL(SYS_READ,&x,0);
		invalidate(x.buf,middle);
		if (start) {
			memmove(buffer,x.buf,middle-ret);
		}
		if (ret) {
			DEBUG_PRINTF("ANGEL read: wanted %d, returned %d\n",count,ret);
			return ret+leftover;
		}
	}
	if (leftover) {
		malloc_ret = malloc(96);
		x.buf = ANGEL_OFFSET_PTR((char *)((((unsigned long)malloc_ret)+31)&-32));
		x.count = leftover;
		clean(&x,3);
		invalidate(x.buf,64);
		ret = ANGEL(SYS_READ,&x,0);
		invalidate(x.buf,64);
		memcpy(buffer+middle,x.buf,leftover-ret);
		free(malloc_ret);
	}
	DEBUG_PRINTF("ANGEL read: wanted %d, returned %d\n",count,ret);
	return ret;
}

unsigned char sys_readc() { return ANGEL(SYS_READC,0,0); }

struct dirent *sys_readdir(int dir, struct dirent *dptr)  /* Clean something? */
{ return VANGEL(SYS_READDIR,dir,ANGEL_OFFSET_PTR(dptr)); }

okay_t sys_remove(const char *name)
{
	struct { const char *name; int len; } x;
	x.name = ANGEL_OFFSET_PTR(name); x.len = strlen(name);
	clean_str(name); clean(&x,2);
	return ANGEL(SYS_REMOVE,&x,0);
}

okay_t sys_rename(const char *oldname, const char *newname)
{
	struct { const char *on; int ol; const char *nn; int nl; } x;
 	x.on = ANGEL_OFFSET_PTR(oldname); x.ol = strlen(oldname);
	x.nn = ANGEL_OFFSET_PTR(newname); x.nl = strlen(newname);
	clean_str(oldname); clean_str(newname); clean(&x,4);
	return ANGEL(SYS_RENAME,&x,0);
}

int sys_rmdir(const char *name) { clean_str(name); return ANGEL(SYS_RENAME,&name,0); }

errno_t sys_seek(fd_t fd, count_t offset)
{
	errno_t ret;
	struct { fd_t fd; count_t c; } x; x.fd = fd; x.c = offset;
	clean(&x,2);
	ret = ANGEL(SYS_SEEK,&x,0);
	DEBUG_PRINTF("sys_seek: fd: %d off: %d ret: %d\n",fd,offset,ret);
	return ret;
}

int sys_statvfs(char *rootdir, void *buffer)  /* Clean buffer? */
{
	struct { char *rd; void *buf; } x;
	x.rd = ANGEL_OFFSET_PTR(rootdir);
	x.buf = ANGEL_OFFSET_PTR(buffer);
	clean_str(rootdir); clean(&x,2);
	return ANGEL(SYS_FSTATVFS,&x,0);
}

okay_t sys_system(const char *command)
{
	struct { const char *cmd; int cl; } x;
	x.cmd = ANGEL_OFFSET_PTR(command);
	x.cl = strlen(x.cmd);
	clean_str(command); clean(&x,2);
	return ANGEL(SYS_SYSTEM,&x,0);
}

count_t sys_time() { return ANGEL(SYS_TIME,0,0); }

errno_t sys_tmpnam(char *buffer, unsigned char target, count_t count)
{
	errno_t ret;
	struct { char *buf; unsigned char target; count_t c; } x;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.target = target; x.c = count;
	clean(buffer,count/4+3); clean(&x,3); ret = ANGEL(SYS_TMPNAM,&x,0);
	clean(buffer,count/4+3);
	return ret;
}

void sys_write0(const char *str) { clean_str(str); clean(&str,1); ANGEL(SYS_WRITE0,&str,0); }

count_t sys_write(fd_t fd, const char *buffer, count_t count)
{
	struct { fd_t fd; const char *buf; count_t c; } x;
	x.fd = fd;
	x.buf = ANGEL_OFFSET_PTR(buffer);
	x.c = count;
	clean(buffer,count/4+3); clean(&x,3); return ANGEL(SYS_WRITE,&x,0);
}

void sys_writec(unsigned char ch) { clean(&ch,1); ANGEL(SYS_WRITEC,&ch,0); }

