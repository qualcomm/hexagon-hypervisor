/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdlib.h>  //  size_t
  
#define MAX_TRANSLATIONS 512

typedef union {
	unsigned long long  raw;
	struct {
		union {
			unsigned long low;
			struct {
				unsigned long ppn:24;
				unsigned long cccc:4;
				unsigned long xwru:4;
			};
		};
		union {
			unsigned long high;
			struct {
				unsigned long vpn:20;
				unsigned long size:4;
				unsigned long unused:7;
				unsigned long chain:1;
			};
		};
	};
} H2K_linear_fmt_t;

typedef H2K_linear_fmt_t lin_map_fmt_t;

//  Think these should go into H2 internal header files somewhere
#if __QDSP6_ARCH__ <= 3
#define U 0
#define R 1
#define W 2
#define X 4
#else
#define U 1
#define R 2
#define W 4
#define X 8
#endif

//  And we want to map them directly to these:

#define PROT_READ	R
#define PROT_WRITE	W
#define PROT_EXEC	X

#define MAP_FIXED	0x10  /*  force the VA requested (or fail)  */

#define CACHE_SHIFT	28    /*  location of cache bits in flags  */
#define CACHE_MASK	0xf

#define get_cccc(flags)	((flags >> CACHE_SHIFT) & CACHE_MASK)

void *h2_map(void *va,size_t length, int prot, int flags, void *pa);
void h2_unmap(void *va);
