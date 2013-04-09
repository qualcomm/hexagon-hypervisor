/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#if 0
h2os_mm_cow_write(mmap_t *mmap, u32_t pageva, pte_t *ptep)
{
	pte_t pte;
	paddr_t paddr,newpaddr;
	ppage_t *pp;
	h2_mutex_lock(&mmap->lock);
	h2os_mm_walk(mmap,vaddr,&pte,&paddr);
	pp = get_ppage(paddr);
	if (pp->refs == 1) {
		/* The only refence is my address space, just change to NORMAL */
		pp->flags = NORMAL;
	} else {
		newpaddr = dup_ppage(paddr);
		h2os_mm_pte_set_paddr(&pte,newpaddr);
		h2os_pp_deref(pp);
	}
	/* Make writable */
	h2os_mm_pte_add_perms(&pte,WRITE);
	h2os_mm_replace(mmap,vaddr,pte);
	h2os_mm_tlb_invalidate(mmap,vaddr);
	h2_mutex_unlock(&mmap->lock);
}

static int pte_prep(void *opaque, pte_p *ptep, va_t va, pa_t pa, int pgshift)
{
	expected_t *expected = opaque;
	if (is_invalid(*ptep)) {
		newtable(ptep);
	}
	if (wanted_pgshift > pgshift)
		/* recurse */
	} else {
		set_pte(ptep,expected->pte)
	}
	return 0;
}

static int add_pte()
{
	/* Prepare to set the PTE */
	with_va(&pgshift,va,pte_prep)
	
}

static int needs_cow(ppage_t ppage)
{
	if (pa.type == NORMAL) return 1;
	/* COW already, or RWSHARED */
	return 0;
}

static int h2os_cow_clone_mkread_helper(void *opaque, pte_p *ptep, va_t va, pa_t pa, int pgshift)
{
	newmmap = opaque;
	pte_t pte = *ptep;
	int ret = 0;
	ppage_t *ppage = pa2ppage(pa);
	if (!pte_valid(pte)) return 0;
	if (needs_cow(ppage)) {
		/* mark ppage as COW */
		pa.type = COW;
		/* Remove read permission */
		set_pte(ptep,pte_mk_nowrite(pte));
		ret = 1;
	}
	ppage_ref(ppage);
	add_pte(va,pgshift,ppage,pte);
	return ret;
}

h2os_cow_clone(mmap_t *mmap)
{
	new_mmap = mknew_mm();
	h2os_mmap_traverse_range(new_mmap,mmap,0,MAX_USER,h2os_cow_clone_mkread_helper);
}

h2os_OLD_mm_cow_clone(mmap_t *mmap)
{
	u32_t vaddr;
	mmap_t *newmmap;
	pte_t pte;
	paddr_t paddr;
	ppage_t *pp;
	h2_mutex_lock(&mmap->lock);
	pte_t pte;
	int i;
	vaddr = 0;
	/* Disable all permissions */
	while (vaddr < KERN_VA_START) {
		h2os_mm_walk(mmap,vaddr,&pte,&paddr);
		if (pte.valid) {
			pp = get_ppage(paddr);
			/* Need to make some of this atomic, maybe? */
			if (pp->flags == NORMAL) {
				pp->flags = COW;
				h2os_mm_pte_rm_perms(&pte,WRITE);
			}
			h2os_pp_ref(pp);
			h2os_mm_replace(mmap,vaddr,pte);
		}
		vaddr += h2os_mm_pte_size_bytes(pte);
	}
	h2os_mm_tlb_invalidate_all(mmap);
	/* Duplicate pages */
	newmmap = page_clone(mmap);
	for (i = 0; i < KERN_VA_START/L2_AREA_SIZE; i++) {
		pte = decode_pte(newmmap.l2ptr[i]);
		if (pte.l2_table_ptr) pte.addr = page_clone(pte.addr);
		newmap.l2ptr[i] = encode_pte(pte);
	}
	h2_mutex_unlock(mmap->lock);
	return newmmap;
}

#endif

