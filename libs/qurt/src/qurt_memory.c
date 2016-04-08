/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qurt.h"
#include <h2_common_pmap.h>
#include <hexagon_protos.h>
#include "qurt_s5alloc.h"

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
 * 
 * FIXME: FIXME: FIXME: need to factor out page table management from qurt memory api
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

/*
 * EJP: What are the things we allocate?
 * qurt_mem_pool structures (64 bytes)  --> 64 byte chip
 * qurt_mem_region structure (28 bytes)	--> 32 byte chip
 * in pgalloc: freelist_node (12 bytes) --> 16 byte chip
 * tables: let's say radix-4 (16 bytes)	--> 16 byte chip
 * 
 * We need to start with some mem pools, but after initial slab we're probably OK
 * If pgalloc needs malloc, we certainly will need to call pgalloc to get new space for a new slab.
 * We probably will need to be able to create a table to get new space for a new slab.
 * We might need to create a mem_region for the new space for the new slab (needs to be mapped in VA space)
 *
 * This means that we will need several spare 16 byte chips and probably at least one spare 32 byte chip always 
 * available so we can allocate the things we need to allocate more allocatable allocations.
 *
 * So either we need an allocator that can manage "emergency rations" or we need an allocator that maintains
 * the count of free items and allocates more slabs when it gets below a low water mark.
 * 
 * Well... what about this? instead of having emergency ration chips, have emergency ration pages.
 * We can always carry around a free page, and if we have allocation issues, we can allocate slabs from the 
 * ration page, and immediately allocate a new ration page before returning to our regularly scheduled allocations.
 * Ration pages can be split between 16/32/64-byte slabs.
 *
 * The ration page can probably bootstrap the whole memory allocator.
 *
 * Since every ration page (except the first one...) was allocated from qurt_mem_region_create, if it frees we 
 * shouldn't need to do anything special, just find the region and return the page.
 *
 * Maybe even better than (or in addition to) emergency ration pages is emergency ration slabs 
 */

qurt_s5id_t mem_pool_s5;
qurt_s5id_t mem_region_s5;
qurt_s5id_t pagetable_s5;

/* 64 bytes */
struct qurt_mem_pool_struct {
	struct qurt_mem_pool_struct *next;
	struct qurt_freelist_node *freelist;
	qurt_mem_pool_attr_t attr;
};

static struct qurt_mem_pool_struct pool_storage[16];

static qurt_mutex_t mem_mutex;

/* 32 bytes */
struct qurt_mem_region_struct {
	struct qurt_mem_region_struct *next;
	struct qurt_mem_pool_struct *ppool;
	qurt_mem_region_attr_t attr;
};

static struct qurt_mem_region_struct region_storage[256];

static struct qurt_mem_pool_struct *vpool = NULL;
static struct qurt_mem_pool_struct *all_pools = NULL;
qurt_mem_pool_t qurt_mem_default_pool;

/* 2048 + bytes in TCM for slab storage */
/* Actually, we'll need lots of 16-byte table entries.  Allocate 16KB total, 15 64-byte and the rest 16-byte */
/* 64 bytes in TCM For root table */
/* Or we can have a 256-entry root table and 15KB for the rest. */
/*
 * It would be simpler and more efficient to just have a gazillion 16 byte blocks in a list than create a lot 
 * of 16-byte slabs if we never need to free the page for something else.
 */
#define ROOT_ENTRY_BITS 8
#define ROOT_ENTRY_SIZE (1<<ROOT_ENTRY_BITS)
#define TABLE_STORAGE_SIZE (15*1024)
#define IN_TCM __attribute__((section(".data.qurt.translations")))
static unsigned int table_root[ROOT_ENTRY_SIZE] IN_TCM __attribute__((aligned(4*ROOT_ENTRY_SIZE)));
static unsigned char table_storage[TABLE_STORAGE_SIZE] IN_TCM __attribute__((aligned(4*ROOT_ENTRY_SIZE)));

static inline unsigned int *find_mapping(unsigned int vpn);

static inline unsigned int minu(unsigned int a, unsigned int b) { return (a <= b) ? a : b; }

static inline void mem_fatal(const char *msg)
{
	qurt_printf("%s\n",msg);
	exit(1);
}

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
	/* FIXME for tables... should implement for debug */
	qurt_printf("Memory Mappings:\n");
	for (i = 0; i < MAX_TRANSLATIONS; i++) {
		if (entry.xwru == 0) continue;
		qurt_printf("%03d: va=%05x000 pa=%06x000 size=%x cccc=%d xwru=%x\n",
			i,entry.vpn,entry.ppn,entry.size,entry.cccc,entry.xwru);
	}
}
#endif

extern long long __tcm_static_vapa_offset__ __attribute__((weak));

static inline void *tcm_p2v(unsigned long pa)
{
	unsigned long offset = (unsigned long)(&__tcm_static_vapa_offset__);
	return (void *)(pa - offset);
}

static inline unsigned long tcm_v2p(void *ptr)
{
	unsigned long addr = (unsigned long)ptr;
	unsigned long offset = (unsigned long)(&__tcm_static_vapa_offset__);
	return addr + offset;
}

static inline unsigned int table_size(unsigned int entry)
{
	return Q6_R_ct0_R(entry)+1;
}

static inline unsigned int terminal_size(unsigned int entry)
{
	return Q6_R_ct0_R(entry);
}

static inline unsigned int *table_addr(unsigned int entry)
{
	return tcm_p2v((entry & (entry-1)) << 2);
}

static inline int is_tableptr(unsigned int entry, unsigned int startbit)
{
	return Q6_R_ct0_R(entry) < startbit;
}

static inline int is_terminal(unsigned int entry, unsigned int startbit)
{
	return Q6_R_ct0_R(entry) == startbit;
}

static inline int is_invalid(unsigned int entry, unsigned int startbit)
{
	return Q6_R_ct0_R(entry) > startbit;
}

static void qurt_pprint_table_walk(unsigned int vpn,
	unsigned int bitsleft,
	unsigned int *tablebase,
	unsigned int tablebits,
	unsigned int indent_level)
{
	int i;
	char *type;
	unsigned int tvpn;
	unsigned int entry;
	unsigned int startbit = bitsleft - tablebits;
	unsigned int *tableaddr;
	unsigned int tablesize;
	for (i = 0; i < (1<<tablebits); i++) {
		tvpn = vpn | (i << startbit);
		entry = tablebase[i];
		                                 type = "INVAL";
		if (is_tableptr(entry,startbit)) type = "TABLE";
		if (is_terminal(entry,startbit)) type = "ENTRY";
		qurt_printf("%*sidx=%03d startbit=%02d vpn=%08x type=%s size=0x%08x entry=0x%08x tableaddr=0x%08x/ppn=0x%08x\n",
			indent_level,"",
			i,
			startbit,
			tvpn,
			type,
			1<<startbit,
			entry,
			(entry & (entry-1)) << 2,
			((entry & (entry-1)) >> 1) & 0x003fffff);
		if (is_tableptr(entry,startbit)) {
			tableaddr = table_addr(entry);
			tablesize = table_size(entry);
			qurt_pprint_table_walk(tvpn,startbit,tableaddr,tablesize,indent_level+4);
		}
	}
}

static void qurt_pprint_table()
{
	qurt_pprint_table_walk(0,20,table_root,ROOT_ENTRY_BITS,0);
}

static inline void transtab_add_region(struct qurt_mem_region_struct *region)
{
	unsigned int vpn;
	unsigned int ppn;
	unsigned int size;
	vpn = region->attr.vpn;
	ppn = region->attr.ppn;
	size = region->attr.size;
	/* qurt_mapping_create_vpn now handles non-power-of-4 page sizes */
	qurt_mapping_create_vpn(vpn,ppn,size,region->attr.cccc, region->attr.perms, region->attr.abits);
}

static inline void transtab_remove_region(struct qurt_mem_region_struct *region)
{
	unsigned int vpn = region->attr.vpn;
	unsigned int cursize;
	unsigned int *entryptr;
	int size = region->attr.size;
	while (size > 0) {
		if ((entryptr = find_mapping(vpn)) == NULL) {
			mem_fatal("translated region not in page tables");
		}
		cursize = terminal_size(*entryptr);
		*entryptr = 0;
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

static inline struct qurt_mem_pool_struct *qurt_mem_alloc_pool()
{
	return qurt_s5_alloc(mem_pool_s5);
}

int qurt_mem_pool_create(char *name, unsigned int base, unsigned int size, qurt_mem_pool_t *pool)
{
	struct qurt_mem_pool_struct *tmp;
	if ((tmp = qurt_mem_alloc_pool()) == NULL) return QURT_EMEM;
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

static inline struct qurt_mem_region_struct *qurt_mem_alloc_region()
{
	return qurt_s5_alloc(mem_region_s5);
}

int qurt_mem_region_create(qurt_mem_region_t *region, qurt_size_t size, qurt_mem_pool_t pool_int, qurt_mem_region_attr_t *attr)
{
	unsigned long vpn = 0;
	unsigned long ppn = 0;
	struct qurt_mem_region_struct *tmp;
	struct qurt_mem_pool_struct *pool = mem_pool_from_uint(pool_int);
	// Turn size into number of pages, rounding up.
	size = (size + 0xFFF) >> 12;
	if (size == 0) return QURT_EVAL;
	if ((tmp = qurt_mem_alloc_region()) == NULL) {
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
			qurt_printf("region_create: call stack=%x %x %x, id=%x\n",
				__builtin_return_address(0),
				__builtin_return_address(1),
				__builtin_return_address(2),
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

static void qurt_memory_make_static_region(unsigned int vpn, unsigned int entry, unsigned int size)
{
	struct qurt_mem_region_struct *tmp;
	if ((tmp = qurt_mem_alloc_region()) == NULL) {
		mem_fatal("OOM");
	}
	qurt_mem_region_attr_init(&tmp->attr);
	tmp->attr.cccc = (entry >> 24) & 0xF;
	tmp->attr.perms = ((entry >> 28) & 0xF) >> 1;
	tmp->attr.abits = 0;
	tmp->attr.vpn = vpn;
	tmp->attr.ppn = ((entry >> 1) & 0x3fffff);
	tmp->attr.ppn &= (tmp->attr.ppn-1);
	tmp->attr.size = size;
	tmp->ppool = 0;
	tmp->attr.mapping_type = QURT_MEM_MAPPING_VIRTUAL_FIXED; // no vpool free
	qurt_rmutex_lock(&mem_mutex);
	tmp->next = all_regions;
	all_regions = tmp;
	qurt_rmutex_unlock(&mem_mutex);
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

/*
 * How do we want to arrange our tables?
 * By policy, 4 translated bits for the first two levels, then 2 bits per level
 * Note that adding an entry for a small page may require allocating up to 6 16-byte tables
 * and 1 64-byte table.
 * 20 16 12 10  8  6  4  2
 *  4  4  2  2  2  2  2  2
 * Or, this works pretty well too (at the expense of ~300 bytes)
 * 20 12 10  8  6  4  2
 *  8  2  2  2  2  2  2
 */
static inline unsigned int policy_tablebits(unsigned int startbit)
{
	return 2;
}

static inline unsigned int *qurt_mem_alloc_new_table()
{
	unsigned long long int *llptr;
	llptr = qurt_s5_alloc(pagetable_s5);
	llptr[0] = 0;
	llptr[1] = 0;
	return (unsigned int *)llptr;
}

static inline unsigned int alloc_table(unsigned int tablebits)
{
	unsigned int *table = qurt_mem_alloc_new_table();
	unsigned long tableaddr = tcm_v2p(table);
	tableaddr >>= 2;
	tableaddr |= 1<<(tablebits-1);
	return tableaddr;
}

static inline int qurt_mapping_create_varadix_walk(
	unsigned int vpn,
	unsigned int pte,
	unsigned int size,
	unsigned int bitsleft,
	unsigned int *tablebase,
	unsigned int tablebits)
{
	unsigned int startbit = bitsleft-tablebits;
	unsigned int this_idx = (vpn >> startbit) & ((1<<tablebits)-1);
	unsigned int entry = tablebase[this_idx];
	unsigned int *tableaddr;
	unsigned int tablesize;
	if (is_terminal(entry,startbit)) return QURT_EMEM;
	if (startbit == size) {
		tablebase[this_idx] = pte;
		return QURT_EOK;
	} else {
		if (is_invalid(entry,startbit)) {
			tablebase[this_idx] = entry = alloc_table(policy_tablebits(startbit));
		}
		tableaddr = table_addr(entry);
		tablesize = table_size(entry);	// or could get from policy...
		return qurt_mapping_create_varadix_walk(vpn,pte,size,startbit,tableaddr,tablesize);
	}
}

int qurt_mapping_create_varadix(unsigned int vpn, unsigned int pte, unsigned int size)
{
	int ret;
	qurt_rmutex_lock(&mem_mutex);
	ret = qurt_mapping_create_varadix_walk(vpn,pte,size,20,table_root,ROOT_ENTRY_BITS);
	qurt_rmutex_unlock(&mem_mutex);
	return ret;
}

/* XXX: FIXME: create new table entry and insert into table */
int qurt_mapping_create_vpn(unsigned int vpn,unsigned int ppn, 
	unsigned int size, unsigned int cache_attribs, unsigned int perm, unsigned int abits)
{
	unsigned int pgsize;
	int ret;
	unsigned int tmp;
	tmp = cache_attribs << 24;
	tmp |= (perm<<1) << 28;
	while (size) {
		pgsize = minu(12,(__builtin_ctz(vpn|ppn)));
		pgsize = minu(pgsize,((31-__builtin_clz(size))));
		pgsize &= ~1;	// even page sizes only
		tmp = (((ppn & 0x3fffff) >> pgsize) << (pgsize+1)) | (1<<pgsize);

		if ((ret=qurt_mapping_create_varadix(vpn,tmp,pgsize)) != QURT_EOK) return ret;

		pgsize = 1<<pgsize;
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

static inline unsigned int *find_mapping_walk(unsigned int vpn, unsigned int bitsleft, 
	unsigned int *tablebase, unsigned int tablebits)
{
	unsigned int startbit = bitsleft-tablebits;
	unsigned int this_idx = (vpn >> startbit) & ((1<<tablebits)-1);
	unsigned int entry = tablebase[this_idx];
	if (is_terminal(entry,startbit)) return &tablebase[this_idx];
	if (is_invalid(entry,startbit)) return NULL;
	return find_mapping_walk(vpn,startbit,table_addr(entry),table_size(entry));
}

static inline unsigned int *find_mapping(unsigned int vpn)
{
	return find_mapping_walk(vpn,20,table_root,ROOT_ENTRY_BITS);
}

void qurt_mapping_remove_vpn(unsigned int vpn)
{
	unsigned int *tmp = find_mapping(vpn);
	if (tmp == NULL) return;
	*tmp = 0;
	h2_vmtrap_clrmap((void *)(vpn << 12));
}

qurt_paddr_64_t qurt_lookup_physaddr_64 (qurt_addr_t vaddr)
{
	unsigned int *tmp = find_mapping(vaddr>>12);
	unsigned int ppn;
	unsigned long long int paddr;
	unsigned int size;
	if (tmp == NULL) return 0;
	size = Q6_R_ct0_R(*tmp);
	ppn = (*tmp >> 1) & 0x3fffff;
	ppn &= ppn - 1;
	paddr = ppn >> size;
	paddr <<= size;
	paddr |= vaddr & ((1<<(12+size))-1);
	//qurt_printf("pa lookup: vaddr=%x paddr=%llx\n",vaddr,paddr);
	return paddr;
}

struct phys_mem_pool_config {
	char name[MAX_POOL_NAME_LEN];
	struct range {
		unsigned int start;
		unsigned int size;
	} ranges[MAX_POOL_RANGES];
};

struct phys_mem_pool_config pool_configs[] __attribute__((weak)) = { { "DEFAULT_PHYSPOOL", { {0x10000,0xF0000000}, {0}}}, { "", { { 0x0, 0x0}, {0x0}}}};
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
		if (0==strcmp(pool_configs[i].name,"DEFAULT_PHYSPOOL")) {
			/* KLUDGE */
			pool_configs[i].ranges[0].start = 0x8d000;
			pool_configs[i].ranges[0].size =  0x00400 << 12;
		}
#endif
		qurt_mem_pool_create(pool_configs[i].name,
			pool_configs[i].ranges[0].start,
			pool_configs[i].ranges[0].size>>12,
			&blah);
	}
	if (qurt_mem_pool_attach("DEFAULT_PHYSPOOL",&qurt_mem_default_pool) != QURT_EOK) {
		mem_fatal("no physpool");
	}
}

static void qurt_memory_pool_init()
{
	unsigned int blah;
	qurt_mem_pool_create("vpool",0x40000,0x10000,&blah);
	vpool = mem_pool_from_uint(blah);
	qurt_memory_builtin_pools();
}

extern long long int __tcm_static_pa_load__ __attribute__((weak));
extern long long int __tcm_static_pa_run__ __attribute__((weak));
extern long long int __tcm_static_section_start__ __attribute__((weak));
extern long long int __tcm_static_section_end__ __attribute__((weak));

#define SYM_PGNO_RND(X) ((((unsigned long)(&X))+0x0fff) >> 12)
#define SYM_PGNO(X) ((((unsigned long)(&X))) >> 12)

/* EJP: FIXME: can we just copy the whole __tcm_static__ area instead of the parts that are mapped? */
static inline void qurt_mapping_static_tcm_load(unsigned int offset)
{
	unsigned long tcm_src_data_addr = (long)(&__tcm_static_pa_load__);
	unsigned long tcm_dst_data_addr = (long)(&__tcm_static_pa_run__) + offset;
	void *tcm_src_data = (void *)tcm_src_data_addr;
	void *tcm_dst_data = (void *)tcm_dst_data_addr;
	unsigned long tcm_static_vaddr_start = (long)(&__tcm_static_section_start__);
	unsigned long tcm_static_vaddr_end = (long)(&__tcm_static_section_end__);
	unsigned long tcm_static_size = tcm_static_vaddr_end - tcm_static_vaddr_start;
	if (tcm_static_size == 0) return;
	memcpy(tcm_dst_data,tcm_src_data,tcm_static_size);
}

#if 0
int qurt_mem_kludge_reject(unsigned long long int inval)
{
	int size = Q6_R_ct0_P(inval);
	unsigned int vpn = (inval >> 32) & 0x000fffff;
	if (((vpn & 0xFF000) == 0x8B000) && (size < 6)) return 1;
	return 0;
}
#endif

/*
 * Here's how I think we'll need to boot up
 * VA=PA+offset to get started (probably before we even get here)
 * Initialize slabs
 * Create page tables based on where things should be eventually
 * Copy TCM to where it belongs
 * Enable new mappings
 * I think we'll want to have all page table allocations as static tcm area... 
 * that way we can compute the pa from the va.  It's kind of ugly but oh well, welcome to qurt land.
 */

static inline void qurt_memory_init_early_varadix_from_tlb(unsigned long long int inval)
{
	unsigned int vpn = (inval >> 32) & 0x000FFFFF;
	unsigned int ppn = (inval >> 1) & 0x003FFFFF;
	unsigned int size = 2*Q6_R_ct0_P(inval);
	unsigned int xwru = (inval >> 28) & 0xf;
	unsigned int cccc = (inval >> 24) & 0xf;
	unsigned int entry = (((ppn >> (size-0)))<<(size+1)) | (1<<size);
	entry |= xwru << 28;
	entry |= cccc << 24;
	qurt_mapping_create_varadix(vpn,entry,size);
	qurt_memory_make_static_region(vpn,entry,size);
}

static void more_mem_fatal(void *opaque, qurt_s5id_t s5id, unsigned int elementsize)
{
	mem_fatal("out of memory on pagetable or mempool s5");
}

static void more_region_storage(void *opaque, qurt_s5id_t s5id, unsigned int elementsize)
{
	void *tmp;
	if ((tmp = malloc(elementsize*64)) == NULL) return;
	qurt_s5_feed(s5id,tmp,elementsize*64);
}

static inline void qurt_memory_init_early_setup_s5mem()
{
	/* Create stream in TCM for page tables.  feed table storage. Panic for moremem. */
	if ((pagetable_s5 = qurt_s5_create(16,more_mem_fatal,NULL)) < 0) {
		mem_fatal("s5_create");
	}
	qurt_s5_feed(pagetable_s5,table_storage,sizeof(table_storage));
	/* Create stream for qurt_mem_pool_structs.  feed pool storage. panic for moremem. */
	if ((mem_pool_s5 = qurt_s5_create(sizeof(struct qurt_mem_pool_struct),more_mem_fatal,NULL)) < 0) {
		mem_fatal("s5_create");
	}
	qurt_s5_feed(mem_pool_s5,pool_storage,sizeof(pool_storage));
	/* Create stream for qurt_mem_region_structs  feed some initial storage. malloc for moremem. */
	if ((mem_region_s5 = qurt_s5_create(sizeof(struct qurt_mem_pool_struct),more_region_storage,NULL)) < 0) {
		mem_fatal("s5_create");
	}
	qurt_s5_feed(mem_region_s5,region_storage,sizeof(region_storage));
}

void qurt_memory_init_early(unsigned long long int *tlbfmt_a, unsigned long long int *tlbfmt_b, unsigned int offset)
{
	int i;
	unsigned int root_entry;
	//qurt_mem_check_sizes();
	qurt_pgalloc_init();
	qurt_memory_init_early_setup_s5mem();
	qurt_rmutex_init(&mem_mutex);
	/* Root table already alloc'd, 256 entries. */
	for (i = 0; tlbfmt_a[i] != 0; i++) {
		qurt_memory_init_early_varadix_from_tlb(tlbfmt_a[i]);
	}
	for (i = 0; tlbfmt_b[i] != 0; i++) {
		qurt_memory_init_early_varadix_from_tlb(tlbfmt_b[i]);
	}
	qurt_memory_pool_init();
	/* Now copy things into TCM that need to be located there 
	 * (including the page tables we just set up) 
	 */
	qurt_mapping_static_tcm_load(offset);
	/* OK, everything should be where it needs to go... now actually turn on real translation */
	root_entry = tcm_v2p(table_root) >> 2;
	root_entry |= (1<<(ROOT_ENTRY_BITS-1));
	qurt_pprint_table();
	h2_vmtrap_newmap_extra((void *)root_entry,H2K_ASID_TRANS_TYPE_VARADIX,H2K_ASID_TLB_INVALIDATE_FALSE,20);
	// qurt_memory_tlb_pin();	// maybe?
	//qurt_pprint_mappings();
	//qurt_pprint_regions();
}

#ifdef QURTMEM_DEBUG
static inline void try_pool_alloc()
{
	qurt_mem_pool_t ppool;
	qurt_mem_region_t region;
	qurt_mem_region_attr_t attr;
	unsigned int addr;
	qurt_mem_pool_attach("DEFAULT_PHYSPOOL",&ppool);
	qurt_mem_region_attr_init(&attr);
	qurt_mem_region_create(&region,4096*64,ppool,&attr);
	qurt_mem_region_attr_get(region,&attr);
	qurt_mem_region_attr_get_virtaddr(&attr,&addr);
	qurt_printf("va=%x\n",addr);
	qurt_pprint_table();
}

void qurt_mem_tester(unsigned long long int *tlbfmt_a)
{
	int i;
	unsigned int vpn;
	unsigned int *entryptr;
	qurt_pgalloc_init();
	qurt_memory_init_early_setup_s5mem();
	qurt_rmutex_init(&mem_mutex);
	for (i = 0; tlbfmt_a[i] != 0; i++) {
		qurt_memory_init_early_varadix_from_tlb(tlbfmt_a[i]);
	}
	qurt_pprint_table();
	for (i = 0; tlbfmt_a[i] != 0; i++) {
		vpn = (tlbfmt_a[i]>>32) & 0x000FFFFF;
		entryptr = find_mapping(vpn);
		qurt_printf("find_mapping: vpn=%x entry @ %p = %x\n",vpn,entryptr,
			(entryptr==NULL) ? 0xdeadbeefU : *entryptr);
	}
	qurt_memory_pool_init();
	qurt_mapping_create_64(0x60000000,0x50000000ULL,0x01000000,0x7,0xf);
	qurt_pprint_table();
	try_pool_alloc();
}
#endif

void qurt_memory_init()
{
	if (vpool == NULL) qurt_printf("Not really setting up memory here...\n");
	//qurt_printf("qurt memory init: sizeof(mempool node): %d\n",sizeof(struct qurt_mem_pool_struct));
	//qurt_printf("qurt memory init: sizeof(region node): %d\n",sizeof(struct qurt_mem_region_struct));
}

