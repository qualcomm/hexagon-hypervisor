/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <angel.h>
#include <string.h>

errno_t sys_clock() { int x = 0; return ANGEL(SYS_CLOCK,&x,0); }

errno_t sys_close(fd_t fd) { return ANGEL(SYS_CLOSE,&fd,0); }

int sys_closedir(int dir) { return ANGEL(SYS_CLOSEDIR,dir,0); }

errno_t sys_errno() { int x = 0; return ANGEL(SYS_ERRNO,&x,0); }

void sys_exit(okay_t status) { ANGEL(SYS_EXIT,&status,status); }

errno_t sys_flen(fd_t fd) { return ANGEL(SYS_FLEN,&fd,0); }

int sys_fstat(fd_t fd, void *buffer)
{
	struct {
		fd_t fd;
		void *buf;
	} x;
	x.fd = fd;
	x.buf = buffer;
	return ANGEL(SYS_FSTAT,&x,0);
}

errno_t sys_ftell(fd_t fd) { return ANGEL(SYS_FTELL,&fd,0); }

errno_t sys_get_cmdline(char *buffer, count_t count)
{
	struct { char *buf; count_t count; } x;
	x.buf = buffer;
	x.count = count;
	return ANGEL(SYS_GET_CMDLINE,&x,0);
}

struct heap_info *sys_heapinfo(struct heap_info *buffer)
{
	return VANGEL(SYS_HEAPINFO,&buffer,0);
}

okay_t sys_iserror(errno_t errcode) { return ANGEL(SYS_ISERROR,&errcode,0); }

int sys_mkdir(char *name, int mode) { return ANGEL(SYS_MKDIR,name,mode); }

int sys_opendir(const char *name) { return ANGEL(SYS_OPENDIR,name,0); }

count_t sys_read(fd_t fd, char *buffer, count_t count)
{
	struct { fd_t fd; char *buf; count_t count; } x;
	x.fd = fd; x.buf = buffer; x.count = count;
	return ANGEL(SYS_READ,&x,0);
}

unsigned char sys_readc() { int x = 0; return ANGEL(SYS_READC,&x,0); }

struct dirent *sys_readdir(int dir, struct dirent *dptr)
{ return VANGEL(SYS_READDIR,dir,dptr); }

okay_t sys_remove(const char *name) 
{
	struct { const char *name; int len; } x;
	x.name = name; x.len = strlen(name);
	return ANGEL(SYS_REMOVE,&x,0);
}

okay_t sys_rename(const char *oldname, const char *newname)
{ 
	struct { const char *on; int ol; const char *nn; int nl; } x;
 	x.on = oldname; x.ol = strlen(oldname);
	x.nn = newname; x.nl = strlen(newname);
	return ANGEL(SYS_RENAME,&x,0);
}

int sys_rmdir(const char *name) { return ANGEL(SYS_RENAME,&name,0); }

errno_t sys_seek(fd_t fd, count_t offset)
{
	struct { fd_t fd; count_t c; } x; x.fd = fd; x.c = offset;
	return ANGEL(SYS_SEEK,&x,0);
}

int sys_statvfs(char *rootdir, void *buffer)
{
	struct { char *rd; void *buf; } x; x.rd = rootdir; x.buf = buffer;
	return ANGEL(SYS_FSTATVFS,&x,0);
}

okay_t sys_system(const char *command)
{
	struct { const char *cmd; int cl; } x; 
	x.cmd = command; x.cl = strlen(x.cmd);
	return ANGEL(SYS_SYSTEM,&x,0);
}

count_t sys_time() { int x=0; return ANGEL(SYS_TIME,&x,0); }

errno_t sys_tmpnam(char *buffer, unsigned char target, count_t count)
{
	struct { char *buf; unsigned char target; count_t c; } x;
	x.buf = buffer; x.target = target; x.c = count;
	return ANGEL(SYS_TMPNAM,&x,0);
}

void sys_write0(const char *str) { ANGEL(SYS_WRITE0,&str,0); }

count_t sys_write(fd_t fd, const char *buffer, count_t count)
{
	struct { fd_t fd; const char *buf; count_t c; } x;
	x.fd = fd; x.buf = buffer; x.c = count;
	return ANGEL(SYS_WRITE,&x,0);
}
void sys_writec(unsigned char ch) { ANGEL(SYS_WRITEC,&ch,0); }

