/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include <h2_common_linear.h>
#include <h2_common_pmap.h>
#include <hexagon_protos.h>

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
int qurt_mem_pool_ereate(char *name, unsigned base, unsigned size, qurt_mem_pool_t *pool);
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

#define MAX_TRANSLATIONS 256

static H2K_linear_fmt_t linear_pages[MAX_TRANSLATIONS] __attribute__((aligned(2048))) __attribute__((section(".data.qurt.translations")));

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

#if 0
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
#endif

static inline void transtab_add_region(struct qurt_mem_region_struct *region)
{
	unsigned int vpn;
	unsigned int ppn;
	unsigned int size;
#if 0
	unsigned int pgsize;
	H2K_linear_fmt_t entry;
	entry.raw = 0;
	entry.cccc = region->attr.cccc;
	entry.xwru = region->attr.perms << 1;
	entry.abits = region->attr.abits;
#endif
	vpn = region->attr.vpn;
	ppn = region->attr.ppn;
	size = region->attr.size;
	/* qurt_mapping_create_vpn now handles non-power-of-4 page sizes */
	qurt_mapping_create_vpn(vpn,ppn,size,region->attr.cccc, region->attr.perms, region->attr.abits);
#if 0
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
#endif
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

static struct qurt_mem_region_struct *all_regions = NULL;

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
		(attr->mapping_type == QURT_MEM_MAPPING_IDEMPOTENT) ||
		((attr->mapping_type == QURT_MEM_MAPPING_VIRTUAL_FIXED) && (attr->ppn != ~0))) { /* FIXME: INVALID_ADDR? */
		ppn = attr->ppn;
	}
	if ((attr->mapping_type == QURT_MEM_MAPPING_VIRTUAL_FIXED) ||
		(attr->mapping_type == QURT_MEM_MAPPING_VIRTUAL_FIXED_ADDR)) vpn = attr->vpn;
	if (attr->mapping_type == QURT_MEM_MAPPING_IDEMPOTENT) vpn = ppn;
	qurt_rmutex_lock(&mem_mutex);
	if (attr->mapping_type != QURT_MEM_MAPPING_NONE) {
		if ((ppn = qurt_pgalloc(&pool->freelist,ppn,size,0)) == 0) {
			qurt_printf("region_create: no free pa space. ppn=%x (%x) size=%x poolstart=%x poolsize=%x type=%x\n",attr->ppn,ppn,
				size,pool->attr.ranges[0].start,pool->attr.ranges[0].size,
				attr->mapping_type);
			// qurt_pgalloc_print_freelist(pool->freelist);
			qurt_printf("region_create: call stack=%x %x %x %x %x %x, id=%x\n",
				__builtin_return_address(0),
				__builtin_return_address(1),
				__builtin_return_address(2),
				__builtin_return_address(3),
				__builtin_return_address(4),
				__builtin_return_address(5),
				h2_thread_myid());
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

static void qurt_memory_make_static_region(H2K_linear_fmt_t entry)
{
	struct qurt_mem_region_struct *tmp;
	if ((tmp = qurt_malloc(sizeof(*tmp))) == NULL) {
		qurt_printf("OOM\n");
		exit(1);
	}
	qurt_mem_region_attr_init(&tmp->attr);
	tmp->attr.cccc = entry.cccc;
	tmp->attr.perms = entry.xwru >> 1;
	tmp->attr.abits = entry.abits;
	tmp->attr.vpn = entry.vpn;
	tmp->attr.ppn = entry.ppn;
	tmp->attr.size = 1<<(entry.size*2);
	tmp->ppool = 0;
	tmp->attr.mapping_type = QURT_MEM_MAPPING_VIRTUAL_FIXED; // no vpool free
	qurt_rmutex_lock(&mem_mutex);
	tmp->next = all_regions;
	all_regions = tmp;
	qurt_rmutex_unlock(&mem_mutex);
}

static void qurt_memory_make_static_regions()
{
	int i;
	for (i = 0; linear_pages[i].raw != 0; i++) {
		qurt_memory_make_static_region(linear_pages[i]);
	}
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
		if (vpool != NULL) qurt_pgfree(&vpool->freelist,region->attr.vpn,region->attr.size);
	}
	if (region->attr.mapping_type != QURT_MEM_MAPPING_NONE) {
		if (ppool != NULL) qurt_pgfree(&ppool->freelist,region->attr.ppn,region->attr.size);
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

void qurt_pprint_regions()
{
	struct qurt_mem_region_struct *tmp;
	for (tmp = all_regions; tmp != NULL; tmp = tmp->next) {
		qurt_printf("MemRegion: [%x,%x) ppn=%x xwru=%x cccc=%x\n",
			tmp->attr.vpn,
			tmp->attr.vpn+tmp->attr.size,
			tmp->attr.ppn,
			tmp->attr.perms << 1,
			tmp->attr.cccc);
	}
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
			//qurt_printf("EJPDBG: memory @ vpn %x found. ppn=%x",vpn,tmp->attr.ppn);
			return QURT_EOK;
		}
	}
	qurt_rmutex_unlock(&mem_mutex);
	//qurt_printf("EJPDBG: memory @ vpn %x not found",vpn);
	return QURT_EVAL;
}

int qurt_mem_region_query_64_ppn(qurt_mem_region_t *region_handle, unsigned long ppn)
{
	struct qurt_mem_region_struct **ptr;
	struct qurt_mem_region_struct *tmp;
	qurt_rmutex_lock(&mem_mutex);
	for (ptr = &all_regions; *ptr != NULL; ptr = &(*ptr)->next) {
		tmp = *ptr;
		/* If the mapping type is QURT_MEM_MAPPING_NONE, we didn't allocate PA space so its not valid */
		if (tmp->attr.mapping_type == QURT_MEM_MAPPING_NONE) continue;
		if ((tmp->attr.ppn <= ppn) && ((tmp->attr.ppn + tmp->attr.size) > ppn)) {
			*region_handle = uint_from_mem_region(tmp);
			qurt_rmutex_unlock(&mem_mutex);
			//qurt_printf("EJPDBG: memory @ ppn %x found. vpn=%x\n",ppn,tmp->attr.vpn);
			return QURT_EOK;
		}
	}
	qurt_rmutex_unlock(&mem_mutex);
	//qurt_printf("EJPDBG: memory @ ppn %x not found\n",ppn);
	return QURT_EVAL;
}

int qurt_mem_region_query_64(qurt_mem_region_t *region_handle, 
	qurt_addr_t vaddr, qurt_paddr_64_t paddr)
{
	if ((qurt_paddr_t)paddr != QURT_MEM_INVALID) 
		return qurt_mem_region_query_64_ppn(region_handle,paddr>>12);
	if (vaddr != QURT_MEM_INVALID)
		return qurt_mem_region_query_64_vpn(region_handle,((unsigned long)vaddr) >> 12);
	return QURT_EFATAL;
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

int qurt_mem_map_static_query_64(qurt_addr_t *vaddr, qurt_paddr_64_t paddr_64, unsigned int page_size, qurt_mem_cache_mode_t cache_attribs, qurt_perm_t perm)
{
	qurt_mem_region_t region;
	qurt_mem_region_attr_t attrs;
	qurt_paddr_64_t basepa;
	qurt_addr_t va;
	if ((qurt_mem_region_query_64(&region,QURT_MEM_INVALID,paddr_64)) != QURT_EOK) return QURT_EVAL;
	qurt_mem_region_attr_get(region,&attrs);
	qurt_mem_region_attr_get_virtaddr(&attrs,&va);
	qurt_mem_region_attr_get_physaddr_64(&attrs,&basepa);
	va += (paddr_64-basepa);
	*vaddr = va;
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

/* EJP: TBD? make this static inline because we get a lot of constant parameters and hopefully
 * the compiler can optimize building the linear format 
 */
int qurt_mapping_create_vpn(unsigned int vpn,unsigned int ppn, 
	unsigned int size, unsigned int cache_attribs, unsigned int perm, unsigned int abits)
{
	H2K_linear_fmt_t tmp;
	unsigned int pgsize;
	int ret;
	tmp.raw = 0;
	tmp.cccc = cache_attribs;
	tmp.xwru = perm<<1;
	tmp.abits = abits;
	while (size) {
		pgsize = minu(6,(__builtin_ctz(vpn|ppn)>>1));
		pgsize = minu(pgsize,((31-__builtin_clz(size))>>1));
		tmp.size = pgsize;
		tmp.ppn = ppn;
		tmp.vpn = vpn;

		if ((ret=qurt_mapping_create_linear(tmp)) != QURT_EOK) return ret;

		pgsize = 1<<(pgsize*2);
		size -= pgsize;
		vpn += pgsize;
		ppn += pgsize;
	}
	return QURT_EOK;
}

int qurt_mapping_create_64(qurt_addr_t vaddr, qurt_paddr_64_t paddr_64, qurt_size_t size,
			 qurt_mem_cache_mode_t cache_attribs, qurt_perm_t perm)
{
	/* EJP: shouldn't it be caller's responsibility here? */
	if (size & 0xFFF) return QURT_EMEM;
	if (size == 0) return QURT_EMEM;
	if (perm == 0) return QURT_EMEM;
	return qurt_mapping_create_vpn(vaddr>>12,paddr_64>>12,size>>12,cache_attribs,perm,0);
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
	//qurt_printf("pa lookup: vaddr=%x paddr=%llx\n",vaddr,paddr);
	return paddr;
}

struct phys_mem_pool_config {
	char name[32];
	struct range {
		unsigned int start;
		unsigned int size;
	} ranges[16];
};

struct phys_mem_pool_config pool_configs[] __attribute__((weak)) = { { "DEFAULT_PHYSPOOL", { {0x10000,0x40000}, {0}}}, };
//extern struct phys_mem_pool_config pool_configs[];

// extern struct phys_mem_pool_config pool_configs[] __attribute__((weak));
// #pragma weak pool_configs = default_config

/* Put weak empty pool config here */

static void qurt_memory_builtin_pools()
{
	int i;
	unsigned int blah;
	for (i = 0; pool_configs[i].name[0] != '\0'; i++) {
#if 0
		qurt_printf("pool <%s> (%x-%x)\n",
			pool_configs[i].name,
			pool_configs[i].ranges[0].start,
			pool_configs[i].ranges[0].size);
#endif
		if (0==strcmp(pool_configs[i].name,"DEFAULT_PHYSPOOL")) {
			/* KLUDGE */
			pool_configs[i].ranges[0].start = 0x8d000;
			pool_configs[i].ranges[0].size =  0x00400 << 12;
		}
		qurt_mem_pool_create(pool_configs[i].name,
			pool_configs[i].ranges[0].start,
			pool_configs[i].ranges[0].size>>12,
			&blah);
	}
	if (qurt_mem_pool_attach("DEFAULT_PHYSPOOL",&qurt_mem_default_pool) != QURT_EOK) {
		//qurt_printf("Can't find default physpool. Bad config?\n");
		exit(1);
	}
}

static void qurt_memory_pool_init()
{
	unsigned int blah;
	qurt_mem_pool_create("vpool",0x40000,0x10000,&blah);
	vpool = mem_pool_from_uint(blah);
	qurt_memory_builtin_pools();
}

#if 0
static void qurt_physpool_shrink()
{
	struct qurt_mem_pool_struct *physpool = mem_pool_from_uint(qurt_mem_default_pool);
	unsigned int base = 0x8C800;
	unsigned int size = 0x00400;
	// easy button: eat the memory leak
	physpool->freelist = NULL;
	// kludge, just know where we should make the pool
	physpool->attr.ranges[0].start = base;
	physpool->attr.ranges[0].size = size;
	qurt_pgfree(&tmp->freelist,
}
#else /* handled above */
static inline void qurt_physpool_shrink() {}
#endif

void qurt_memory_init()
{
	// H2K_linear_fmt_t entry;
	// int i;
	if (linear_pages[0].raw != 0) { 
		/* Already did early init! */
		qurt_physpool_shrink();
		qurt_memory_make_static_regions();
		qurt_pprint_regions();
		return;
	}
#if 0
	qurt_rmutex_init(&mem_mutex);
	qurt_memory_pool_init();
	entry.raw = 0;
	entry.ppn = 0x01000;
	entry.vpn = 0x01000;
	entry.size = SIZE_16M;
	entry.xwru = URWX;
	entry.cccc = L1WB_L2C;
	/* FOR NOW... initialize some default stuff */
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
	qurt_memory_make_static_regions();
#endif
}

#if 1

extern long long int __tcm_static_pa_load__ __attribute__((weak));
extern long long int __tcm_static_pa_run__ __attribute__((weak));
extern long long int __tcm_static_section_start__ __attribute__((weak));
extern long long int __tcm_static_section_end__ __attribute__((weak));

#define SYM_PGNO_RND(X) ((((unsigned long)(&X))+0x0fff) >> 12)
#define SYM_PGNO(X) ((((unsigned long)(&X))) >> 12)

static inline int vpn_in_tcm_range(unsigned long vpn)
{
	unsigned long tcm_start_vpn = SYM_PGNO(__tcm_static_section_start__);
	unsigned long tcm_end_vpn = SYM_PGNO_RND(__tcm_static_section_end__);
	return ((vpn >= tcm_start_vpn) && (vpn < tcm_end_vpn));
}

static inline H2K_linear_fmt_t qurt_mapping_static_tcm_load(H2K_linear_fmt_t entry)
{
	unsigned long tcm_start_vpn = SYM_PGNO(__tcm_static_section_start__);
	//unsigned long tcm_end_vpn = SYM_PGNO_RND(__tcm_static_section_end__);
	unsigned long tcm_ddr_ppn = SYM_PGNO(__tcm_static_pa_load__);
	unsigned long tcm_tcm_ppn = SYM_PGNO(__tcm_static_pa_run__);
	if (&__tcm_static_pa_load__ == NULL) return entry; // no symbols
	if (!vpn_in_tcm_range(entry.vpn)) return entry; // not in range
	/* Before adding translation, copy into TCM */
	unsigned long src_pa = (entry.vpn - tcm_start_vpn + tcm_ddr_ppn) << 12;
	unsigned long dst_pa = (entry.vpn - tcm_start_vpn + tcm_tcm_ppn) << 12;
	memcpy((void *)dst_pa,(void *)src_pa,1ULL << (12+(entry.size*2)));
	entry.ppn = entry.vpn - tcm_start_vpn + tcm_tcm_ppn;
	/* EJP: FIXME: pin TLB entry? */
	/* EJP: need to pin TLB entries after translation enabled so we get the right ASID */
	return entry;
}
#endif

int qurt_mem_kludge_reject(unsigned long long int inval)
{
	int size = Q6_R_ct0_P(inval);
	unsigned int vpn = (inval >> 32) & 0x000fffff;
	if (((vpn & 0xFF000) == 0x8B000) && (size < 6)) return 1;
	return 0;
}

static inline void qurt_memory_early_add_tlbfmt(unsigned long long int inval)
{
	int size;
	H2K_linear_fmt_t entry;
	size = Q6_R_ct0_P(inval);
	inval = inval & (inval - 1); // clear least significant set bit
	entry.raw = 0;
	entry.vpn = inval >> 32;
	entry.ppn = (inval & 0x00FFFFFF) >> 1;
	entry.size = size;
	entry.xwru = inval >> 28;
	entry.cccc = inval >> 24;
	entry = qurt_mapping_static_tcm_load(entry);
	/* if (entry.size >= 4) FIXME: pin TLB entry */
	qurt_mapping_create_linear(entry);
}

#if 0
static int qurt_mem_compare_va(const void *va, const void *vb)
{
	const H2K_linear_fmt_t *a = va;
	const H2K_linear_fmt_t *b = vb;
	int avpn = a->vpn;
	int bvpn = b->vpn;
	/*
	 * VA comparison
	 * Invalid entries should always compare > valid entries.
	 * Note that invalid entries are zero
	 */
	if (a->raw == 0) return (b->raw != 0);
	if (b->raw == 0) return -1;
	return avpn - bvpn;
}

static int qurt_mem_compare_size(const void *va, const void *vb)
{
	const H2K_linear_fmt_t *a = va;
	const H2K_linear_fmt_t *b = vb;
	int sizediff;
	/*
	 * VA comparison
	 * Invalid entries should always compare > valid entries.
	 * Note that invalid entries are zero
	 */
	if (a->raw == 0) return (b->raw != 0);
	if (b->raw == 0) return -1;
	sizediff = -(a->size - b->size);
	if (sizediff != 0) return sizediff;
	return a->vpn - b->vpn;
}

static inline int qurt_mem_coalesce_ok(H2K_linear_fmt_t base, H2K_linear_fmt_t test, unsigned int shift)
{
	unsigned int mask_hi = (~0) << shift;
	unsigned int mask_lo = ((1 << shift)-1);
	if ((base.vpn & mask_hi) != (test.vpn & mask_hi)) return 1;
	if ((test.ppn & mask_lo) != (test.vpn & mask_lo)) return 0;
	if (base.size != test.size) return 0;
	if (base.cccc != test.cccc) return 0;
	return 1;
}

static inline H2K_linear_fmt_t qurt_mem_do_coalesce(H2K_linear_fmt_t base, unsigned int idx, unsigned int shift)
{
	unsigned int mask_hi = (~0) << shift;
	H2K_linear_fmt_t dest = linear_pages[idx];
	if ((base.vpn & mask_hi) == (dest.vpn & mask_hi)) {
		base.xwru |= dest.xwru;
		linear_pages[idx].raw = 0;
	}
	return base;
}

static inline int qurt_mem_coalesce_count(
	H2K_linear_fmt_t a,
	H2K_linear_fmt_t b,
	H2K_linear_fmt_t c,
	H2K_linear_fmt_t d,
	unsigned int shift)
{
	int count = 0;
	unsigned int mask_hi = (~0) << shift;
	count += ((a.vpn & mask_hi) == (b.vpn & mask_hi));
	count += ((a.vpn & mask_hi) == (c.vpn & mask_hi));
	count += ((a.vpn & mask_hi) == (d.vpn & mask_hi));
	return count;
}

static inline int qurt_mem_coalesce_good_idea(
	H2K_linear_fmt_t a,
	H2K_linear_fmt_t b,
	H2K_linear_fmt_t c,
	H2K_linear_fmt_t d,
	unsigned int size,
	unsigned int shift)
{
	if ((qurt_mem_coalesce_count(a,b,c,d,shift) == 0) && (size > 3)) return 0;
	if (size >= 6) return 0;
	return 1;
}

int qurt_memory_translation_optimize_pass()
{
	int i;
	H2K_linear_fmt_t a,b,c,d;
	unsigned int shift;
	unsigned int size;
	// unsigned int mask_hi;
	unsigned int mask_lo;
	for (i = 0; (i < (MAX_TRANSLATIONS-3)) && (linear_pages[i].raw != 0); i++) {
		a = linear_pages[i+0];
		b = linear_pages[i+1];
		c = linear_pages[i+2];
		d = linear_pages[i+3];
		size = a.size;				/* Current translation size */
		shift = (size+1)*2;			/* Shift for new mask: one bigger */
		// mask_hi = (~0) << shift;
		mask_lo = (1<<shift) - 1;
		if ((a.vpn & mask_lo) != 0) continue;	/* first one not aligned */
		if ((a.ppn & mask_lo) != 0) continue;	/* first one not aligned */
		if (!qurt_mem_coalesce_ok(a,b,shift)) continue;
		if (!qurt_mem_coalesce_ok(a,c,shift)) continue;
		if (!qurt_mem_coalesce_ok(a,d,shift)) continue;
		if (qurt_mem_coalesce_good_idea(a,b,c,d,size,shift) == 0) continue;
		/* Everything OK. Coalesce to bigger size. */
		a = qurt_mem_do_coalesce(a,i+1,shift);
		a = qurt_mem_do_coalesce(a,i+2,shift);
		a = qurt_mem_do_coalesce(a,i+3,shift);
		//qurt_printf("[[optimized entry %d]]\n",i);
		a.size++;
		linear_pages[i] = a;
		return 1;
	}
	return 0;
}

static inline void qurt_memory_translation_optimize()
{
	int changed;
	/* Find mappings that we can grow: lowest mapping aligned, next
	 * mappings either missing or to the appropriately-larger PA range, same CCCC.
	 * If different WRXU, or together. 
	 * If we change something, start all over
	 */
	//qurt_printf(">> BEFORE\n");
	//qurt_pprint_mappings();
	do {
		/* sort translations by VA */
		qsort(	linear_pages,
			MAX_TRANSLATIONS,
			sizeof(H2K_linear_fmt_t),
			qurt_mem_compare_va);
		changed = qurt_memory_translation_optimize_pass();
		//qurt_printf(">> OPTIMIZE PASS (changed=%d)\n",changed);
		//qurt_pprint_mappings();
	} while (changed != 0);
	/* Now sort translations by size */
	qsort(	linear_pages,
		MAX_TRANSLATIONS,
		sizeof(H2K_linear_fmt_t),
		qurt_mem_compare_size);
	//qurt_printf(">> FINAL\n");
	//qurt_pprint_mappings();
}
#endif

void qurt_memory_translation_check()
{
	int i,j;
	H2K_linear_fmt_t a;
	H2K_linear_fmt_t b;
	unsigned int mask;
	for (i = 0; linear_pages[i].raw != 0; i++) {
		a = linear_pages[i];
		for (j = 0; linear_pages[j].raw != 0; j++) {
			if (i == j) continue;
			b = linear_pages[j];
			mask = ((~0) << (a.size*2)) & ((~0) << (b.size*2));
			if ((a.vpn & mask) == (b.vpn & mask)) {
				qurt_printf("OOPS::: %d overlaps %d? 0x%016llx 0x%016llx\n",i,j,a.raw,b.raw);
			}
		}
	}
}

void qurt_memory_tlb_pin()
{
	int i;
	qurt_addr_t vaddr;
	qurt_paddr_64_t paddr;
	qurt_size_t size;
	qurt_mem_cache_mode_t cache_attribs;
	qurt_perm_t perms;
	unsigned int id;
	for (i = 0; linear_pages[i].raw != 0; i++) {
		if (!vpn_in_tcm_range(linear_pages[i].vpn) 
			&& (linear_pages[i].size < 4)) continue;
		vaddr = linear_pages[i].vpn;
		vaddr <<= 12;
		paddr = linear_pages[i].ppn;
		paddr <<= 12;
		size = 0x1000 << (linear_pages[i].size * 2);
		cache_attribs = linear_pages[i].cccc;
		perms = linear_pages[i].xwru >> 1;
		qurt_tlb_entry_create_64(&id,vaddr,paddr,size,cache_attribs,perms,0);
	}
}

void qurt_memory_init_early(unsigned long long int *tlbfmt_a, unsigned long long int *tlbfmt_b)
{
	int i;
	qurt_rmutex_init(&mem_mutex);
	for (i = 0; tlbfmt_a[i] != 0; i++) {
		if (qurt_mem_kludge_reject(tlbfmt_a[i])) continue;
		qurt_memory_early_add_tlbfmt(tlbfmt_a[i]);
	}
	for (i = 0; tlbfmt_b[i] != 0; i++) {
		if (qurt_mem_kludge_reject(tlbfmt_b[i])) continue;
		qurt_memory_early_add_tlbfmt(tlbfmt_b[i]);
	}
	//qurt_memory_translation_optimize();
	qurt_memory_translation_check();
	h2_vmtrap_newmap(linear_pages,H2K_ASID_TRANS_TYPE_LINEAR,H2K_ASID_TLB_INVALIDATE_FALSE);
	qurt_memory_tlb_pin();
	qurt_memory_pool_init(); // for now... 
	//qurt_pprint_mappings();
	//qurt_pprint_regions();
}

