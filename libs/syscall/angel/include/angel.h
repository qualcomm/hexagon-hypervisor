/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SYSCALL_ANGEL_ANGEL_H
#define SYSCALL_ANGEL_ANGEL_H 1

//extern void *H2_V2P_OFFSET __attribute__ ((weak));

extern long __boot_net_phys_offset__;

unsigned int __angel(unsigned int r0, void *r1, unsigned int r2);

unsigned int angel(unsigned int r0, void *r1, unsigned int r2);

#define ANGEL_OFFSET_PTR(P) ((void *)((unsigned long)(P) + __boot_net_phys_offset__))

#define ANGEL(A,B,C) (angel((A),((void *)(B)),((unsigned int)(C))))
#define VANGEL(A,B,C) ((void *)angel((A),((void *)(B)),((unsigned int)(C))))

	/* SYSTEM CALL CODES */
#define SYS_OPEN		1
#define SYS_CLOSE		2
#define SYS_WRITEC		3
#define SYS_WRITE0		4
#define SYS_WRITE		5
#define SYS_READ		6
#define SYS_READC		7
#define SYS_ISERROR		8
#define SYS_ISTTY		9
#define SYS_SEEK		10
#define SYS_FLEN		12
#define SYS_TMPNAM		13
#define SYS_REMOVE		14
#define SYS_RENAME		15
#define SYS_CLOCK		16
#define SYS_TIME		17
#define SYS_SYSTEM		18
#define SYS_ERRNO		19
#define SYS_GET_CMDLINE	21
#define SYS_HEAPINFO	22
#define SYS_EXIT		24
#define SYS_WRITECREG           0x43
#define SYSCALL			0x53

#define SYS_FTELL		0x100
#define SYS_FSTAT		0x101
#define SYS_FSTATVFS		0x102
#define SYS_STAT		0x103
/* #define SYS_GETCWD		0x104 */
/* #define SYS_ACCESS		0x105 */
/* #define SYS_FCNTL		0x106 */
/* #define SYS_GETTIMEOFDAY	0x107 */

#define SYS_OPENDIR		0x180
#define SYS_CLOSEDIR		0x181
#define SYS_READDIR		0x182
#define SYS_MKDIR		0x183
#define SYS_RMDIR		0x184

/* #define SYS_MMAP        0x200 */
/* #define SYS_MUNMAP      0x201 */
/* #define SYS_MPROTECT    0x202 */
/* #define SYS_FORK        0x203 */
/* #define SYS_EXECVE      0x204 */

	/* TYPES */
typedef int count_t;
typedef int errno_t;
typedef int fd_t;
typedef unsigned int t_mode_t;
typedef int okay_t;

struct dirent
{
    long  d_ino;
    char  d_name[255]; /* Should be variable size */
};

struct heap_info {	/* heap information */
	int heap_base;
	int heap_limit;
	int stack_base;
	int stack_limit;
	};

struct __sys_stat
{
    unsigned long long int dev;
    unsigned long long int ino;
    unsigned long mode;
    unsigned long nlink;
    unsigned long long int rdev;
    unsigned long size;
    unsigned long __pad1;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
    unsigned long __pad2;
}__attribute__ ((aligned (8)));

	/* SYSTEM CALL FUNCTIONS */
errno_t sys_clock(void);
errno_t sys_close(fd_t);
errno_t sys_errno(void);
void sys_exit(okay_t);
errno_t sys_flen(fd_t);
errno_t sys_fstat(fd_t fd, void *buffer);
errno_t sys_ftell(fd_t);
errno_t sys_get_cmdline(char *, count_t);
struct heap_info *sys_heapinfo(struct heap_info *);
okay_t sys_iserror(errno_t);
fd_t sys_open(const char *, t_mode_t);
count_t sys_read(fd_t, char *, count_t);
unsigned char sys_readc(void);
okay_t sys_remove(const char *);
okay_t sys_rename(const char *, const char *);
errno_t sys_seek(fd_t, count_t);
okay_t sys_system(const char *);
count_t sys_time(void);
errno_t sys_tmpnam(char *, unsigned char, count_t);
count_t sys_write(fd_t, const char *, count_t);
void sys_writec(unsigned char);
void sys_write0(const char *);

#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF(...) /* nothing */
#endif

#endif

