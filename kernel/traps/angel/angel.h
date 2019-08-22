/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ANGEL_H
#define H2K_ANGEL_H 1

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

u32_t H2K_trap_angel(u32_t op, void *buf, u32_t arg);
static inline void H2K_angel_init(void) {	H2K_spinlock_init(&H2K_gp->logbuf_lock);}

#endif
