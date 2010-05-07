/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QMEMORY_H
#define _QMEMORY_H

#include <qerror.h>
#include <qtypes.h>
#include <assert.h>
#include <stdlib.h>

#define qmem_region_attr_setvirtaddr(attr,addr) do { attr->virtaddr = addr; } while (0)

#ifdef __cplusplus
extern "C" {
#endif
extern qmem_pool_t qmem_default_pool;

/*****************************************************************/
/* FILE: qmemory.h                                               */
/*                                                               */
/* SERVICES: qmemory API                                         */
/*                                                               */
/* DESCRIPTION: Prototypes of qmemory API                        */
/*****************************************************************/

/************************ COPYRIGHT NOTICE ***********************/

/* All data and information contained in or disclosed by this    */
/* document is confidential and proprietary information of       */
/* QUALCOMM, Inc and all rights therein are expressly reserved.  */
/* By accepting this material the recipient agrees that this     */
/* material and the information contained therein is held in     */
/* confidence and in trust and will not be used, copied,         */
/* reproduced in whole or in part, nor its contents revealed in  */
/* any manner to others without the express written permission   */
/* of QUALCOMM, Inc.                                             */
/*****************************************************************/

#define KERNEL_CACHE_OPT_THRESHHOLD (128*1024)

/**
 * Attach to a memory pool 
 *
 * The qmem_pool_attach is used to attach to a memory pool. 
 *
 * Qube memory model works with the concept of memory pool and memory region.
 * User can specify different types of PHYSICAL memory as different memory
 * pools.  For example EBI/SMI/TCM memory can be specified as EBI pool, SMI
 * pool and TCM pool respectively.   
 *
 * Memory pools are static system information that can only be specified in xml
 * configuration file.  All pools are by default accessible by all PDs in the
 * system, a PD can get the handle of a pool with qmem_pool_attach function
 * with a name.
 *
 * The default physical pool specified in elfweaver configuration is mapped to
 * qmem_default_pool global handle.  Users only need to call qmem_pool_attach
 * for non-default physical memory pool
 *
 * @param name        Name of the memory pool. This shall be a name specified
 *                    in xml configuration file for a physical pool 
 * @param pool  [OUT] memory pool object
 * @return            EOK:       Successful attach
*/
static inline int qmem_pool_attach(const char *name, qmem_pool_t *pool) { *pool = 0xb0e0e0f0; return EOK; }

/**
 * Create a memory region with specific attributes               
 *
 * The qmem_region_create is used to create a memory region. The sz
 * specifies the size of memory region. The attr is used to specify
 * attributes. The attributes that can be specified are physical
 * address, memory type (defafult, tcm), mapping type (virtual,
 * idempotent or physical contiguous), and cache properties
 * (write-back, write-through or uncached). If attr is NULL, then
 * default values are used
 *
 * Memory regions are always grouped and shared with other PDs if
 * needed.  The grp specifies the memory group to which a memory
 * regions belongs to.
 *
 * @param region  [OUT] Memory region object
 * @param group         Memory group that the memory regions belongs to
 * @param sz            Size of the memory region. When size is not with the
 *                      granularity of 4K, it will be round up to 4K boundary 
 * @param attr          Attributes of memory region
 * @return              EOK if region is created successfuly; 
 * @return              EMEM if out of memory; 
 * @return              EVAL if not valid memory type/mapping type/cache type;
 * @return              EINVALID if group is not a valid handle 
 *
 */
static inline int qmem_region_create(qmem_region_t *region, size_t sz, qmem_pool_t pool,
                       const qmem_region_attr_t *attr)
{
	if ((*region = (unsigned int)memalign(4096,sz)) != 0) return EOK;
	return EMEM;
}

/**
 * Remove a memory region
 *
 * @param region  Memory region object
 * @return        EOK if deletion is successful;
 * @return        EINVALID if region is not a valid handle; 
 */
//static inline int qmem_region_delete(qmem_region_t region) { return EOK; } 
static inline int qmem_region_delete(qmem_region_t region) { free((void *)region); return EOK; }

/**
 * Perform a cache operation (flush or invaldate or both)
 *
 * Limitaion: Currently, Flush & Invalidate operations are
 * "for" loop of user flush instructions.  Further
 * investigation is needed to decide under which
 * circumstances privileged flush instruction should be used,
 * because possible system call or IPC call may have other
 * costs 
 * 
 * @param data    Pointer to data or instruction memory that needs to be flushed
 * @param size    Size of the data
 * @param opcode  Operation type (flush or invalidate or both)
 * @param type    Cache type (instruction or data)
 * @return        EOK if operation is successful 
 * @return        EVAL if type is not valid; 
 */
void qmem_blast_inv_icache(const void *data, size_t size);
void qmem_blast_clean_dcache(const void *data, size_t size);
void qmem_blast_inv_dcache(const void *data, size_t size);
void qmem_blast_cleaninv_dcache(const void *data, size_t size);
int qmem_kernel_cache_opt(const void * data, size_t size, const unsigned int type);

enum cache_opt {
    KERNEL_DCCLEAN=0,
    KERNEL_DCINV,
    KERNEL_DCCLEANINV,
    KERNEL_ICINV
}; 

//
//For detailed arch information, refer to Chapter 6(Cache Memory) in QDSP6 V2
//Architecture System-Level Specification
//

static inline int qmem_cache_clean(const void *data, size_t size, const qmem_cache_op_t opcode, const qmem_cache_type_t type)
{
	if (type == QMEM_ICACHE) {
		switch (opcode) {
			case QMEM_CACHE_FLUSH: return EOK;
			case QMEM_CACHE_INVALIDATE: qmem_blast_inv_icache(data,size); return EOK;
			case QMEM_CACHE_FLUSH_INVALIDATE: qmem_blast_inv_icache(data,size); return EOK;
			default: return EVAL;
		}
	} else if (type == QMEM_DCACHE) {
		switch (opcode) {
			case QMEM_CACHE_FLUSH: 
				qmem_blast_clean_dcache(data,size); return EOK;
			case QMEM_CACHE_INVALIDATE: 
				qmem_blast_inv_dcache(data,size); return EOK;
			case QMEM_CACHE_FLUSH_INVALIDATE: 
				qmem_blast_cleaninv_dcache(data,size); return EOK;
			default: return EVAL;
		}
	}
	return EVAL;
}

static inline int qmem_cache_clean_aligning(const void *data, size_t size, const qmem_cache_op_t opcode, const qmem_cache_type_t type)
{
	//Don't fail, rather do what L4 did, which is align the memory/size and proceed.
	char * byte_addr = (char*)data;
	/* Align down to 2 ^ 5 = 32 */
	char * aligned_addr = (char*)(((unsigned int)byte_addr) & (0xffffffe0));
	size += (((unsigned int)byte_addr) % 32);
	if ((size%32)!=0) size+=(32-(size%32));
	return qmem_cache_clean(aligned_addr, size, opcode, type);
}

/**
 * Initialize attribute object with default values. The default value for
 * physical address and virtual address is INVALID_ADDR (-1),
 * QMEM_MEMORY_DEFAULT for memory type, QMEM_MAPPING_VIRTUAL for mapping type,
 * and QMEM_CACHE_WRITEBACK_SHARED for cacheing property
 *
 * @param attr  Attributes object
 * @return      void 
 */
static inline void qmem_region_attr_init(qmem_region_attr_t *attr){
	attr->mapping_type = QMEM_MAPPING_VIRTUAL;
	attr->cache_mode = QMEM_CACHE_WRITEBACK;
	attr->physaddr =(bl_addr_t) -1;
	attr->virtaddr =(bl_addr_t) -1;
	attr->type = QMEM_REGION_LOCAL;
}

/**
* Get attributes of a memory region
*
* @param region        Memory region object
* @param attr   [OUT]  Memory region attributes
* @return              EOK if successful, error code otherwise
*/
static inline int qmem_region_get_attr(qmem_region_t region, qmem_region_attr_t *attr)
{
	attr->physaddr = region;
	attr->virtaddr = region;
	return EOK;
}

/**
 * Set memory type. The possible values are QMEM_MEMORY_DEFAULT and
 * QMEM_MEMORY_TCM
 *
 * @param attr  Attributes object
 * @param type  Memory type
 * @return      void 
 */
static inline void qmem_region_attr_settype(qmem_region_attr_t *attr, qmem_region_type_t type){
    attr->type = type;
}

/**
 * Get memory type
 *
 * @param attr  Attributes object
 * @param type  Memory type
 * @return      void 
 */
static inline void qmem_region_attr_gettype(qmem_region_attr_t *attr, qmem_region_type_t *type){
    (*type) = attr->type;
}

/**
 * Set physical address attribute
 *
 * @param attr  Attributes object
 * @param addr  Physical address where memory region should be created
 * @return      void
 */
static inline void qmem_region_attr_setphysaddr(qmem_region_attr_t *attr, bl_addr_t addr){
    attr->physaddr = addr;
}

/**
 * Get physical address attribute
 *
 * @param attr  Attributes object
 * @param addr  Physical address where memory region should be created
 * @return      void 
 */
static inline void qmem_region_attr_getphysaddr(qmem_region_attr_t *attr, unsigned int *addr){
    (*addr) = attr->physaddr;
}

/**
 * Get virtual address attribute
 *
 * We don't have a setvirtaddr helper api because users can not specify virtual
 * address for a memory region
 *
 * @param attr  Attributes object
 * @param addr  Virtual address where memory region should be created
 * @return      void 
 */
static inline void qmem_region_attr_getvirtaddr(qmem_region_attr_t *attr, unsigned int *addr){
    (*addr) = attr->virtaddr;
}

/**
 * Set memory mapping type. The possible values are QMEM_MAPPING_VIRTUAL,
 * QMEM_MAPPING_IDEMPOTENT and QMEM_MAPPING_PHYS_CONTIGUOUS
 *
 * @param attr     Attributes object
 * @param mapping  Mapping property
 * @return         void 
 */
static inline void qmem_region_attr_setmapping(qmem_region_attr_t *attr, qmem_mapping_t mapping){
    attr->mapping_type = mapping;
}

/**
 * Get memory mapping type
 *
 * @param attr     Attributes object
 * @param mapping  Mapping property
 * @return         void
 */
static inline void qmem_region_attr_getmapping(qmem_region_attr_t *attr, qmem_mapping_t *mapping){
    (*mapping) = attr->mapping_type;
}

/**
 * Set cache property. The possible values are QMEM_CACHE_WRITEBACK,
 * QMEM_CACHE_WRITETHROUGH and QMEM_CACHE_NONE
 *
 * @param attr  Attributes object
 * @param mode  Cacheability property
 * @return      void
 */
static inline void qmem_region_attr_setcachemode(qmem_region_attr_t *attr, qmem_cache_mode_t mode){
    attr->cache_mode = mode;
}

/**
 * Get cache property
 *
 * @param attr  Attributes object
 * @param mode  Cacheability property
 * @return      void
 */
static inline void qmem_region_attr_getcachemode(qmem_region_attr_t *attr, qmem_cache_mode_t *mode){
    (*mode) = attr->cache_mode;
}

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
