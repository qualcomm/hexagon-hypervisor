/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <angel.h>
#include <fcntl.h>

#define ERRSTR_LEN 1024
char errstr[ERRSTR_LEN];

void error(char *str1, char *str2) {

	int err = sys_errno();
	strncat(errstr, ": ", ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str1, ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str2, ERRSTR_LEN - strlen(errstr) - 1);
	errno = err;
	perror(errstr);

	exit(1);
}

int main (int argc, char **argv) {
	int infd = 0;
	int outfd = 0;
	int bytes = -1;
	int ret;
	unsigned int bufsize = 1024 * 16;
	char *buf;

	strncpy(errstr, argv[0], ERRSTR_LEN - 1);

	argc--;
	argv++;
	if (0 == argc) {
		printf("--infile <file>  --outfile <file>  --bytes <int>  --bufsize <int>\nAll optional; bufsize default 16Kbytes\n");
		exit(0);
	}

	while (argc) {
		if (0 == strcmp(argv[0], "--infile")) {
			if (-1 == (infd = open(argv[1], O_RDONLY))) {
				error("Can't open infile ", argv[1]);
			}
			argc -= 2; argv += 2;
			continue;
		}
		if (0 == strcmp(argv[0], "--outfile")) {
			if (-1 == (outfd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC))) {
				error("Can't open outfile ", argv[1]);
			}
			argc -= 2; argv += 2;
			continue;
		}
		if (0 == strcmp(argv[0], "--bytes")) {
			bytes = strtol(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
		}
		if (0 == strcmp(argv[0], "--bufsize")) {
			bufsize = strtoul(argv[1],NULL,0);
			argc -= 2; argv += 2;
			continue;
		}
	}

	if (NULL == (buf = (char *)malloc(bufsize))) {
		error("malloc buf", NULL);
	}

	while (bytes > 0 || infd) {
		if (infd) {
			if (-1 == (ret = read(infd, buf, bufsize))) {
				error("read infile", NULL);
			}
			bytes -= ret;
			if (0 == ret) {
				close(infd);
				bytes = 0;
				infd = 0;
			}

			if (outfd) {
				if (-1 == (ret = write(outfd, buf, ret))) {
					error("write outfile", NULL);
				}
			}
		} else if (outfd) {
			if (-1 == (ret = write(outfd, buf, bufsize))) {
				error("write outfile", NULL);
			}
			bytes -= ret;
		} else {
			exit(0);
		}
	}
}
