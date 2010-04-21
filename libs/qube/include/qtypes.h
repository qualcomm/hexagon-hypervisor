/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QTYPE_H
#define _QTYPE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QTHREAD_NAME_LEN       16 
#define QTHREAD_MIN_STACKSIZE  4096
#define QTHREAD_MAX_STACKSIZE  1048576
#define QTHREAD_MIN_PRIORITY   0
#define QTHREAD_MAX_PRIORITY   252
#define QTHREAD_STATUS_DELETED 0xbabe

typedef uint32_t  bl_addr_t;
typedef uint32_t  qobject_t;

typedef unsigned int qmem_region_t; 
typedef unsigned int qmem_pool_t;

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

/**
 * Mapping type
 * 
 * QMEM_MAPPING_VIRTUAL is the default mode, in which system will
 * pick up an availabe range of virtual address, and mapped to
 * contiguous or seperated physical address;    
 *
 * In QMEM_MAPPING_IDEMPOTENT mode, the virtual address will be same
 * as the phyiscal address. 
 *
 * In QMEM_MAPPING_PHYS_CONTIGUOUS mode, the virtual address might
 * not be  same as the phyiscal address. But physical address of the
 * memory region is guaranted to be contiguous. 
 */
typedef enum {
        QMEM_MAPPING_VIRTUAL=0,
        QMEM_MAPPING_PHYS_CONTIGUOUS,
} qmem_mapping_t;

/**
 * Cache mode type
 */
typedef enum {
#if __QDSP6_ARCH__ >= 2
        QMEM_CACHE_WRITEBACK=7,
        QMEM_CACHE_NONE_SHARED=6,
        QMEM_CACHE_WRITETHROUGH=5,
        QMEM_CACHE_WRITEBACK_NONL2CACHEABLE=0,
        QMEM_CACHE_WRITETHROUGH_NONL2CACHEABLE=1,
        QMEM_CACHE_WRITEBACK_L2CACHEABLE=QMEM_CACHE_WRITEBACK,
        QMEM_CACHE_WRITETHROUGH_L2CACHEABLE=QMEM_CACHE_WRITETHROUGH,
        QMEM_CACHE_NONE
#else
        QMEM_CACHE_WRITEBACK=0,
        QMEM_CACHE_WRITETHROUGH=1,
        QMEM_CACHE_NONE
        
#endif
} qmem_cache_mode_t;

/**
 * Cache type 
 *
 * Specifying data cache or instruction cache
 */
typedef enum {
        QMEM_ICACHE,
        QMEM_DCACHE
} qmem_cache_type_t;

/**
 * Cache operation type
 */
typedef enum { 
    QMEM_CACHE_FLUSH,
    QMEM_CACHE_INVALIDATE,
    QMEM_CACHE_FLUSH_INVALIDATE
} qmem_cache_op_t;

/**
 * Memory access permision
 */
typedef enum {
        QMEM_PERM_READ=0x4,
        QMEM_PERM_WRITE=0x2,
        QMEM_PERM_EXECUTE=0x1,
} qmem_perm_t;

typedef enum {
	QMEM_REGION_LOCAL = 0,
	QMEM_REGION_SHARED
} qmem_region_type_t;

/**
 * Memory attribute
 *
 * Users can specify physaddr in QMEM_MAPPING_IDEMPOTENT mode and
 * QMEM_MAPPING_PHYS_CONTIGUOUS mode.  Users can not specify virtaddr for a
 * memory region, it can only be queried by qmem_attr_getvirtaddr function 
 */
typedef struct {
    qmem_mapping_t    mapping_type;
    qmem_cache_mode_t cache_mode;
    bl_addr_t            physaddr;
    bl_addr_t            virtaddr;
    qmem_region_type_t   type;
} qmem_region_attr_t;

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif

