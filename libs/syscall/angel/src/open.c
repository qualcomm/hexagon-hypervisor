/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdlib.h>
#include <angel.h>
#include <string.h>

/* Dinkumware is all crazy. */

		/* open flags */
#define O_CREAT		(0x1 << 6)	/* create file if none exists */
#define O_EXCL		(0x2 << 6)	/* exclusive use */
#define O_NOCTTY	(0x4 << 6)	/* don't make controlling tty */
#define O_TRUNC		(0x8 << 6)	/* truncate file */

		/* fcntl and open file status fiags */
#define O_APPEND	(0x2 << 9)	/* seek to end before each write */
#define O_NONBLOCK	(0x4 << 9)	/* non-blocking */
#define O_DSYNC		(0x8 << 9)	/* sync writes for data integrity */
#define O_RSYNC		(0x8 << 9)	/* sync reads */
#define O_SYNC		(0x8 << 9)	/* sync writes for file integrity */

#define O_RDONLY	0x0		/* open for read only */
#define O_WRONLY	0x1     	/* open for write only */
#define O_RDWR		0x2		/* open for read/write */

#define O_ACCMODE	0x3		/* mask for file access modes */

#define O_BINARY	0x8000		/* unused in Linux/Unix */

static const int modemap[] = {	/* convert open mode */
	O_RDONLY,
	O_RDONLY | O_BINARY,
	O_RDWR,
	O_RDWR | O_BINARY,

	O_WRONLY | O_CREAT | O_TRUNC,
	O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
	O_RDWR | O_CREAT | O_TRUNC,
	O_RDWR | O_CREAT | O_TRUNC | O_BINARY,

	O_WRONLY | O_CREAT | O_APPEND,
	O_WRONLY | O_CREAT | O_APPEND | O_BINARY,
	O_RDWR | O_CREAT | O_APPEND,
	O_RDWR | O_CREAT | O_APPEND | O_BINARY,

	O_RDWR | O_CREAT,
	O_RDWR | O_CREAT | O_EXCL,
};

static inline void clean_str(const char *x)
{
	do {
		asm volatile ("dccleaninva(%0)" : :"r"(x):"memory");
	} while (*x++);
}

static inline void clean(void *vx,int words)
{
	int *x = vx;
	int i;
	for (i = 0; i < words; i++) {
		asm volatile ("dccleaninva(%0)" : :"r"(x+i):"memory");
	};
}

fd_t sys_open(const char *name, t_mode_t mode)
{
	int i;
	struct { const char *name; int mode; int len; } x;
	x.name = ANGEL_OFFSET_PTR(name);
	x.len = strlen(name); x.mode = 0;

	mode &= (O_RDONLY
		| O_WRONLY
		| O_RDWR
		| O_CREAT
		| O_TRUNC
		| O_APPEND
		| O_BINARY
		| O_EXCL);
	for (i = 0; i < sizeof (modemap) / sizeof (modemap[0]); ++i) {
		if (modemap[i] == mode) {
			x.mode = i;
			break;
		}
	}
	clean_str(x.name); clean(&x,3);
	return ANGEL(SYS_OPEN,&x,0);
}
_STD_END

/*
 * Copyright (c) 2006 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 */

/*
061015 pjp: added new file
061016 pjp: changed arg1 to an array
061017 pjp: added masking for mode
 */

