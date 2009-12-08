/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_trace.h
 * system call tracing helpers
 */

#ifndef H2_TRACE_H
#define H2_TRACE_H 1

#ifndef H2_DEBUG
#define H2_TRACE(str, ...) __VA_ARGS__
#else
#define H2_TRACE(str, ...) \
	do { \
		h2_printf("%s:%d: %s: >>> calling %s\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
		__VA_ARGS__; \
		h2_printf("%s:%d: %s: <<< %s returned\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
	} while (0);
#endif

#endif
