/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_trace.h
 * system call tracing helpers
 */

#ifndef BLAST_TRACE_H
#define BLAST_TRACE_H 1

#ifndef BLAST_DEBUG
#define BLAST_TRACE(str, ...) __VA_ARGS__
#else
#define BLAST_TRACE(str, ...) \
	do { \
		blast_printf("%s:%d: %s: >>> calling %s\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
		__VA_ARGS__; \
		blast_printf("%s:%d: %s: <<< %s returned\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
	} while (0);
#endif

#endif
