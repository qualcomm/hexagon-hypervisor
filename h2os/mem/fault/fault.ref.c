/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#if 0
h2os_mm_fault_nopage(mm,vaddr)
{
	/* If we had lazy allocation, add here */
	/* If we had demand paging of files from disk, add here */
	error();
}

h2os_mm_fault_execute(mm,vaddr)
{
	/* Assuming this isn't used for fancy cache coherence... */
	error();
}

h2os_mm_fault_read(mm,vaddr)
{
	/* Assuming this isn't used for fancy cache coherence... */
	error();
}

h2os_mm_fault_write(mm,vaddr)
{
	ppage_t *me;
	me = find_ppage(mm,vaddr);
	if (me.type == COW) {
		h2os_mm_cow_write();
	} else {
		error();
	}
}

#endif

