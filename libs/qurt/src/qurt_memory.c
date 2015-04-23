/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include <h2_common_linear.h>
#include <h2_common_pmap.h>

/*
 * EJP: try and make a qurt memory 
 *
 * EJP: this API is not my fault.
 *
 * OK, the goal here is to manage several kinds of resources:
 *  - Physical pages: for normal memory you don't want to allocate the same thing twice
 *  - Virtual address ranges: find places for mappings to go
 *  - Translations: manage the translation information + CCC + perms + bus attribs + ...
 * 
 * To do this, QuRT defines the following things:
 *  - Memory pools, which are collections of physical addresses
 *  - Memory regions, which are chunks allocated out of pools.  VA, PA, Mapping, etc can be set
 *  - Mappings, which can (I guess?) be done independently of pools/regions? (mostly not used)
 * 
 * QuRT distinguishes between static things (done at build time) and dynamic ones. 
 * I'm not sure why, I think I'll start out consolidated.
 * 
 * We can probably live without mapping API, but it will be useful for implementing memory
 * regions. 
 * 
 * Technically memory pools allow for collections of PAs, but contiguous is probably fine
 * 
 * Separate from all this is the need for an allocator for VA and PA spaces. Let's 
 * bust that out into pgalloc.c
 * 
 */

/*
 * qurt_mem_pool_t and qurt_mem_region_t ... change to pointers to structs...
 *
 * I think we have to support this, they use it for HLOS giving memory to modem?
 *
 *
int qurt_mem_pool_attach(char *name, qurt_mem_pool_t *pool);
int qurt_mem_pool_create(char *name, unsigned base, unsigned size, qurt_mem_pool_t *pool);
NEVER USED? int qurt_mem_pool_add_pages
NEVER USED? int qurt_mem_pool_remove_pages

int qurt_mem_region_create(qurt_mem_region_t *region, qurt_size_t size, qurt_mem_pool_t pool, qurt_mem_region_attr_t *attr);
int qurt_mem_region_delete(qurt_mem_region_t region);

int qurt_mem_region_query_64(qurt_mem_region_t *region_handle, qurt_addr_t vaddr, qurt_paddr_64_t paddr_64);

int qurt_mapping_create_64(qurt_addr_t vaddr, qurt_paddr_64_t paddr_64, qurt_size_t size,
                         qurt_mem_cache_mode_t cache_attribs, qurt_perm_t perm);
int qurt_mapping_remove_64(qurt_addr_t vaddr, qurt_paddr_64_t paddr_64, qurt_size_t size);

qurt_paddr_64_t qurt_lookup_physaddr_64 (qurt_addr_t vaddr);

TBD later: int qurt_mapping_reclaim(qurt_addr_t vaddr, qurt_size_t vsize, qurt_mem_pool_t pool);
 *
 */

struct qurt_mem_pool_struct {
	struct qurt_mem_pool_struct *next;
	struct qurt_freelist_node *freelist;
	qurt_mem_pool_attr_t attr;
};

#define MAX_TRANSLATIONS 512

static H2K_linear_fmt_t linear_pages[MAX_TRANSLATIONS] __attribute__((aligned(4096))) __attribute__((section(".data")));

static qurt_mutex_t mem_mutex;

struct qurt_mem_region_struct {
	struct qurt_mem_region_struct *next;
	struct qurt_mem_pool_struct *ppool;
	qurt_mem_region_attr_t attr;
};

static struct qurt_mem_pool_struct *vpool = NULL;
static struct qurt_mem_pool_struct *all_pools = NULL;
qurt_mem_pool_t qurt_mem_default_pool;

static inline H2K_linear_fmt_t *find_mapping(unsigned int vpn);

static inline unsigned int minu(unsigned int a, unsigned int b) { return (a <= b) ? a : b; }

static inline struct qurt_mem_pool_struct *mem_pool_from_uint(qurt_mem_pool_t pool_int)
{
	union {
		struct qurt_mem_pool_struct *p;
		qurt_mem_pool_t i;
	} x;
	x.i = pool_int;
	return x.p;
}

static inline qurt_mem_pool_t uint_from_mem_pool(struct qurt_mem_pool_struct *pool_ptr)
{
	union {
		struct qurt_mem_pool_struct *p;
		qurt_mem_pool_t i;
	} x;
	x.p = pool_ptr;
	return x.i;
}

static inline struct qurt_mem_region_struct *mem_region_from_uint(qurt_mem_region_t region_int)
{
	union {
		struct qurt_mem_region_struct *p;
		qurt_mem_region_t i;
	} x;
	x.i = region_int;
	return x.p;
}

static inline qurt_mem_region_t uint_from_mem_region(struct qurt_mem_region_struct *region_ptr)
{
	union {
		struct qurt_mem_region_struct *p;
		qurt_mem_region_t i;
	} x;
	x.p = region_ptr;
	return x.i;
}

static void qurt_pprint_mappings()
{
	int i;
	H2K_linear_fmt_t entry;
	qurt_printf("Memory Mappings:\n");
	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		entry = linear_pages[i];
		if (entry.xwru == 0) continue;
		qurt_printf("%03d: va=%05x000 pa=%06x000 size=%x cccc=%d xwru=%x\n",
			i,entry.vpn,entry.ppn,entry.size,entry.cccc,entry.xwru);
	}
}

static inline void transtab_add_region(struct qurt_mem_region_struct *region)
{
	unsigned int vpn;
	unsigned int ppn;
	unsigned int size;
	unsigned int pgsize;
	H2K_linear_fmt_t entry;
	entry.raw = 0;
	entry.cccc = region->attr.cccc;
	entry.xwru = region->attr.perms << 1;
	entry.abits = region->attr.abits;
	vpn = region->attr.vpn;
	ppn = region->attr.ppn;
	size = region->attr.size;
	while (size) {
		pgsize = minu(6,(__builtin_ctz(vpn|ppn)>>1));
		pgsize = minu(pgsize,((31-__builtin_clz(size))>>1));
		entry.size = pgsize;
		entry.vpn = vpn;
		entry.ppn = ppn;
		qurt_mapping_create_linear(entry);
		pgsize = 1<<(pgsize * 2);
		vpn += pgsize;
		ppn += pgsize;
		size -= pgsize;
	}
	//qurt_pprint_mappings();
}

static inline void transtab_remove_region(struct qurt_mem_region_struct *region)
{
	unsigned int vpn = region->attr.vpn;
	unsigned int cursize;
	int size = region->attr.size;
	H2K_linear_fmt_t empty;
	H2K_linear_fmt_t *tmp;
	empty.raw = 1;
	while (size > 0) {
		if ((tmp = find_mapping(vpn)) == NULL) continue;
		cursize = 1<<(tmp->size*2);
		*tmp = empty;
		h2_vmtrap_clrmap((void *)(vpn << 12));
		size -= cursize;
		vpn += cursize;
	}
}

int qurt_mem_pool_attach(char *name, qurt_mem_pool_t *pool)
{
	struct qurt_mem_pool_struct *tmp;
	qurt_rmutex_lock(&mem_mutex);
	for (tmp = all_pools; tmp != NULL; tmp = tmp->next) {
		if (strcmp(tmp->attr.name,name) == 0) {
			*pool = uint_from_mem_pool(tmp);
			qurt_rmutex_unlock(&mem_mutex);
			return QURT_EOK;
		}
	}
	*pool = uint_from_mem_pool(NULL);
	qurt_rmutex_unlock(&mem_mutex);
	return QURT_EINVALID;
}

static inline void qurt_mem_pool_init_pool(struct qurt_mem_pool_struct *tmp, const char *name, unsigned int base, unsigned int size)
{
	strcpy(tmp->attr.name,name);
	tmp->freelist = NULL;
	tmp->attr.ranges[0].start = base;
	tmp->attr.ranges[0].size = size;
	qurt_pgfree(&tmp->freelist,base,size);
}

int qurt_mem_pool_create(char *name, unsigned int base, unsigned int size, qurt_mem_pool_t *pool)
{
	struct qurt_mem_pool_struct *tmp;
	if ((tmp = qurt_malloc(sizeof(*tmp))) == NULL) return QURT_EMEM;
	qurt_mem_pool_init_pool(tmp,name,base,size);
	qurt_rmutex_lock(&mem_mutex);
	tmp->next = all_pools;
	all_pools = tmp;
	qurt_rmutex_unlock(&mem_mutex);
	*pool = uint_from_mem_pool(tmp);
	return QURT_EOK;
}

/*
 * EJP: the exact semantics of what all the mapping types and values for mem_region_create mean 
 * is not clear to me, at least not from the code & docs.  But I believe it to be the following:
 *
 * Physical addresses are not guaranteed to be contiguous, but are in practice?
 *
 * QURT_MEM_MAPPING_PHYS_CONTIGUOUS requires contiguous phys mem @ fixed address
 * QURT_MEM_MAPPING_IDEMPOTENT additionally requires VA=PA (not used any more?)
 * 
 * QURT_MEM_MAPPING_VIRTUAL allows any physical pages at any VA, but
 * QURT_MEM_MAPPING_VIRTUAL_FIXED_ADDR requires it at a specific VA
 *
 * QURT_MEM_MAPPING_NONE reserves VA, skips phys pool alloc, doesn't put it in the memory map (why? dunno)
 * QURT_MEM_MAPPING_VIRTUAL_FIXED uses reserved VA space and allocs physical memory or uses fixed addr
 *  (basically, skips virt pool alloc)
 *
 * SO... given that, I think we have:
 * VIRTUAL overrides vaddr with don't care, then it's VIRTUAL_FIXED_ADDR 
 * MAPPING_NONE is VIRTUAL but don't allocate pages and don't map
 * MAPPING_VIRTUAL_FIXED is maybe a physical allocate but no virt pool allocate
 * IDEPOTENT sets vaddr with paddr, then it's ...
 * PHYS_CONTIGUOUS which allows setting physical address
 */

struct qurt_mem_region_struct *all_regions = NULL;

int qurt_mem_region_create(qurt_mem_region_t *region, qurt_size_t size, qurt_mem_pool_t pool_int, qurt_mem_region_attr_t *attr)
{
	unsigned long vpn = 0;
	unsigned long ppn = 0;
	struct qurt_mem_region_struct *tmp;
	struct qurt_mem_pool_struct *pool = mem_pool_from_uint(pool_int);
	// Turn size into number of pages, rounding up.
	size = (size + 0xFFF) >> 12;
	if (size == 0) return QURT_EVAL;
	if ((tmp = qurt_malloc(sizeof(*tmp))) == NULL) {
		qurt_printf("OOM\n");
		return QURT_EMEM;
	}
	if (attr == NULL) {
		attr = &tmp->attr;
		qurt_mem_region_attr_init(attr);
	}
	if ((attr->mapping_type == QURT_MEM_MAPPING_PHYS_CONTIGUOUS) ||
		(attr->mapping_type == QURT_MEM_MAPPING_IDEMPOTENT)) ppn = attr->ppn;
	if ((attr->mapping_type == QURT_MEM_MAPPING_VIRTUAL_FIXED) ||
		(attr->mapping_type == QURT_MEM_MAPPING_VIRTUAL_FIXED_ADDR)) vpn = attr->vpn;
	if (attr->mapping_type == QURT_MEM_MAPPING_IDEMPOTENT) vpn = ppn;
	qurt_rmutex_lock(&mem_mutex);
	if (attr->mapping_type != QURT_MEM_MAPPING_NONE) {
		if ((ppn = qurt_pgalloc(&pool->freelist,ppn,size,0)) == 0) {
			qurt_printf("no free pa space. ppn=%x poolstart=%x poolsize=%x\n",attr->ppn,
				pool->attr.ranges[0].start,pool->attr.ranges[0].size);
			qurt_rmutex_unlock(&mem_mutex);
			qurt_free(tmp);
			*region = uint_from_mem_region(NULL);
			return QURT_EMEM;
		};
	}
	if (attr->mapping_type != QURT_MEM_MAPPING_VIRTUAL_FIXED) {
		if ((vpn = qurt_pgalloc(&vpool->freelist,vpn,size,ppn)) == 0) {
			/* Seriously? I don't believe the whole address space is full */
			qurt_printf("no free va space\n");
			qurt_pgfree(&pool->freelist,ppn,size);
			qurt_free(tmp);
			*region = uint_from_mem_region(NULL);
			qurt_rmutex_unlock(&mem_mutex);
			return QURT_EMEM;
		};
	};
	/* Add region to list of regions */
	tmp->ppool = pool;
	tmp->attr = *attr;
	tmp->attr.vpn = vpn;
	tmp->attr.ppn = ppn;
	tmp->attr.size = size;
	tmp->next = all_regions;
	all_regions = tmp;
	if (attr->mapping_type != QURT_MEM_MAPPING_NONE) {
		transtab_add_region(tmp);
	}
	qurt_rmutex_unlock(&mem_mutex);
	*region = uint_from_mem_region(tmp);
	return QURT_EOK;
}

int qurt_mem_region_delete(qurt_mem_region_t region_int)
{
	struct qurt_mem_region_struct **ptr;
	struct qurt_mem_region_struct *region = mem_region_from_uint(region_int);
	struct qurt_mem_pool_struct *ppool = region->ppool;
	qurt_rmutex_lock(&mem_mutex);
	if (region->attr.mapping_type != QURT_MEM_MAPPING_NONE) {
		transtab_remove_region(region);
	}
	if (region->attr.mapping_type != QURT_MEM_MAPPING_VIRTUAL_FIXED) {
		if (vpool == NULL) qurt_printf("AAAK... null vpool\n");
		qurt_pgfree(&vpool->freelist,region->attr.vpn,region->attr.size);
	}
	if (region->attr.mapping_type != QURT_MEM_MAPPING_NONE) {
		if (ppool == NULL) qurt_printf("AAAK... null ppool\n");
		qurt_pgfree(&ppool->freelist,region->attr.ppn,region->attr.size);
	}
	/* remove region from list */
	for (ptr = &all_regions; *ptr != NULL; ptr = &(*ptr)->next) {
		if (*ptr == region) {
			*ptr = region->next;
			break;
		}
	}
	qurt_rmutex_unlock(&mem_mutex);
	free(region);
	return QURT_EOK;
}

int qurt_mem_region_query_64_vpn(qurt_mem_region_t *region_handle, unsigned long vpn)
{
	struct qurt_mem_region_struct **ptr;
	struct qurt_mem_region_struct *tmp;
	qurt_rmutex_lock(&mem_mutex);
	for (ptr = &all_regions; *ptr != NULL; ptr = &(*ptr)->next) {
		tmp = *ptr;
		if ((tmp->attr.vpn <= vpn) && ((tmp->attr.vpn + tmp->attr.size) > vpn)) {
			*region_handle = uint_from_mem_region(tmp);
			qurt_rmutex_unlock(&mem_mutex);
			return QURT_EOK;
		}
	}
	qurt_rmutex_unlock(&mem_mutex);
	return QURT_EVAL;
}

int qurt_mem_region_query_64_ppn(qurt_mem_region_t *region_handle, unsigned long ppn)
{
	struct qurt_mem_region_struct **ptr;
	struct qurt_mem_region_struct *tmp;
	qurt_rmutex_lock(&mem_mutex);
	for (ptr = &all_regions; *ptr != NULL; ptr = &(*ptr)->next) {
		tmp = *ptr;
		if ((tmp->attr.ppn <= ppn) && ((tmp->attr.ppn + tmp->attr.size) > ppn)) {
			*region_handle = uint_from_mem_region(tmp);
			qurt_rmutex_unlock(&mem_mutex);
			return QURT_EOK;
		}
	}
	qurt_rmutex_unlock(&mem_mutex);
	return QURT_EVAL;
}

int qurt_mem_pool_attr_get (qurt_mem_pool_t pool_int, qurt_mem_pool_attr_t *attr)
{
	struct qurt_mem_pool_struct *pool = mem_pool_from_uint(pool_int);
	*attr = pool->attr;
	return QURT_EOK;
}

int qurt_mem_region_attr_get(qurt_mem_region_t region_int, qurt_mem_region_attr_t *attr)
{
	struct qurt_mem_region_struct *region = mem_region_from_uint(region_int);
	*attr = region->attr;
	return QURT_EOK;
}

static inline H2K_linear_fmt_t *find_empty_entry()
{
	int i;
	H2K_linear_fmt_t *tmp;
	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		tmp = &linear_pages[i];
		if (tmp->xwru == 0) return tmp;
	}
	return NULL;
}

int qurt_mapping_create_linear(H2K_linear_fmt_t entry)
{
	H2K_linear_fmt_t *tmp;
	qurt_rmutex_lock(&mem_mutex);
	tmp = find_empty_entry();
	if (tmp == NULL) {
		qurt_rmutex_unlock(&mem_mutex);
		return QURT_EMEM;
	}
	*tmp = entry;
	qurt_rmutex_unlock(&mem_mutex);
	//qurt_pprint_mappings();
	return QURT_EOK;
}

static inline H2K_linear_fmt_t *find_mapping(unsigned int vpn)
{
	int i;
	H2K_linear_fmt_t *tmp;
	unsigned int mask;
	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		tmp = &linear_pages[i];
		mask = 0xFFFFFFFFU << (tmp->size*2);
		if (tmp->raw == 0) return NULL;
		if ((tmp->xwru != 0) && ((vpn & mask) == (tmp->vpn & mask))) return tmp;
	}
	return NULL;
}

void qurt_mapping_remove_vpn(unsigned int vpn)
{
	H2K_linear_fmt_t *tmp = find_mapping(vpn);
	H2K_linear_fmt_t empty;
	empty.raw = 1;
	if (tmp == NULL) return;
	*tmp = empty;
	h2_vmtrap_clrmap((void *)(vpn << 12));
}

qurt_paddr_64_t qurt_lookup_physaddr_64 (qurt_addr_t vaddr)
{
	H2K_linear_fmt_t *tmp = find_mapping(vaddr>>12);
	unsigned long long int paddr;
	if (tmp == NULL) return 0;
	paddr = ((unsigned long long int)(tmp->ppn)) << 12;
	paddr &= ((-1LL) << (12 + tmp->size*2));
	paddr |= vaddr & ((1 << (12 + tmp->size*2))-1);
	qurt_printf("pa lookup: vaddr=%x paddr=%llx\n",paddr);
	return paddr;
}

void qurt_memory_init()
{
	H2K_linear_fmt_t entry;
	int i;
	unsigned int blah;
	qurt_rmutex_init(&mem_mutex);
	entry.raw = 0;
	entry.ppn = 0x01000;
	entry.vpn = 0x01000;
	entry.size = SIZE_16M;
	entry.xwru = URWX;
	entry.cccc = L1WB_L2C;
	qurt_mem_pool_create("vpool",0x80000,0xF8000,&blah);
	vpool = mem_pool_from_uint(blah);
	/* FOR NOW... initialize some default stuff */
	qurt_mem_pool_create("DEFAULT_PHYSPOOL",0x10000,0x40000,&qurt_mem_default_pool);
	for (i = 0; i < 8; i++) {
		qurt_mapping_create_linear(entry);
		entry.ppn += 0x01000;
		entry.vpn += 0x01000;
	}

	/* map device space for timer */
	entry.ppn = 0xfe000;
	entry.vpn = 0xfe000;
	entry.size = SIZE_16M;
	entry.xwru = URWX;
	entry.cccc = DEVICE_TYPE;
	qurt_mapping_create_linear(entry);

	qurt_mem_pool_attach("DEFAULT_PHYSPOOL",&qurt_mem_default_pool);
	qurt_pprint_mappings();
	h2_vmtrap_newmap(linear_pages,H2K_ASID_TRANS_TYPE_LINEAR,H2K_ASID_TLB_INVALIDATE_FALSE);
	qurt_printf("newmap done\n");
	qurt_pprint_mappings();
}

