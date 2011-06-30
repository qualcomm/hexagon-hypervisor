/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qmemory.h>
#include <stdio.h>

int qmem_pool_attach(const char *name, qmem_pool_t *pool)
{ 
	printf("qmem_pool_attach: %x=<%s>\n",name,name);
	*pool = (qmem_pool_t)name; 
	return EOK; 
}

int qmem_region_create(qmem_region_t *region, size_t sz, qmem_pool_t pool,
                       const qmem_region_attr_t *attr)
{
	printf("qmem_region_create: Creating region from pool %x\n",pool);
	if (attr->physaddr == -1) {
		if ((*region = (unsigned int)memalign(4096,sz)) != 0) return EOK;
	} else {
		printf("qmem_region_create: Physaddr given; using %x\n", attr->physaddr);
		*region = (unsigned int)attr->physaddr;
		return EOK;
	}
	return EMEM;
}

