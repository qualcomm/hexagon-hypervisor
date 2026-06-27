/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int sys_flen(int);

int fstat(int fd, struct stat *buf)
{
	static int ino_counter = 1;
	fprintf(stderr,"WARNING: using fake fstat!\n");
	buf->st_ino = ino_counter++;
	buf->st_dev = 1;
	buf->st_size = sys_flen(fd);
	return 0;
}

