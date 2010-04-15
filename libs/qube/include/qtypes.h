/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QTYPE_H
#define _QTYPE_H

#include <stdint.h>
#include <stddef.h>

#define QTHREAD_NAME_LEN       16 
#define QTHREAD_MIN_STACKSIZE  4096
#define QTHREAD_MAX_STACKSIZE  1048576
#define QTHREAD_MIN_PRIORITY   0
//#define QTHREAD_MAX_PRIORITY   252
#define QTHREAD_STATUS_DELETED 0xbabe

typedef uint32_t  bl_addr_t;
typedef uint32_t  qobject_t;

/**
 * Mutex type
 *
 * Local mutex is only accessable within a PD and shared mutex
 * can be used across PD 
 */ 
typedef enum {
    QMUTEX_LOCAL=0,
    QMUTEX_SHARED
} qmutex_type_t;

typedef struct {
 qmutex_type_t type;
} qmutex_attr_t;

/**
 * Permission to access an object, currrently it only applies to the qube
 * memory region object, specifying the permission for the dest PD to access
 * the memory region 
 */
typedef enum {
        QPERM_READ=0x4,
        QPERM_WRITE=0x2,
        QPERM_EXECUTE=0x1,
        QPERM_FULL=QPERM_READ|QPERM_WRITE|QPERM_EXECUTE,
} qperm_t;

#endif
