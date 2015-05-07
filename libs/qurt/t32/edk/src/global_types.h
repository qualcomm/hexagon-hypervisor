/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

// Not sure if this is the correct place to put these...
#define ARCH_v1		0
#define ARCH_v2		1

typedef unsigned char          size1u_t; 
typedef char                   size1s_t; 
typedef unsigned short int     size2u_t; 
typedef short                  size2s_t; 
typedef unsigned int           size4u_t; 
typedef int                    size4s_t; 
#ifdef VCPP
typedef unsigned _int64        size8u_t; 
typedef _int64                 size8s_t; 
typedef unsigned long          ulong;
typedef unsigned int           uint;
#else
typedef unsigned long long int size8u_t; 
typedef long long int          size8s_t; 
#endif

#endif /* GLOBAL_TYPES_H */
