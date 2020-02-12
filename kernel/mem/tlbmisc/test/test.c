/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <spinlock.h>
#include <tlbmisc.h>
#include <tlbfmt.h>
#include <tlbinsert.h>
#include <hw.h>
#include <globals.h>

H2K_kg_t H2K_kg;

void FAIL(const char *str)
{
    puts("FAIL");
    puts(str);
    exit(1);
}

/* Helper functions */
u32_t find_tlb_index_last_used(void) {
    u32_t found_tlb_index = H2K_kg.last_tlb_index;  
    for (u32_t index = 0; index < H2K_kg.last_tlb_index; index++) {
        u64_t entry = H2K_mem_tlb_read(index);
        if (entry != 0) {
            found_tlb_index = index;
        }
    }
    return found_tlb_index;
}

void set_entry_at_tlb_index(u32_t tlb_index, u64_t tlb_entry, u32_t va, u32_t asid) {
    H2K_mem_tlbfmt_t entry;
    entry.raw = tlb_entry;
    entry.vpn = (va >> PAGE_BITS);
    entry.asid = asid;
    tlb_entry = entry.raw; // with updated va & asid
    H2K_spinlock_lock(&H2K_kg.tmpmap_lock);  // for tmpmap
    H2K_mutex_lock_tlb();  // for H2K_safemem
    H2K_mem_tlb_write(tlb_index, tlb_entry);
    H2K_isync();
    H2K_mutex_unlock_tlb();
    H2K_spinlock_unlock(&H2K_kg.tmpmap_lock);
}

int main()
{
    __asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
    
    H2K_kg.tlb_size = 128;
    H2K_kg.last_tlb_index = H2K_kg.tlb_size - 1;
    H2K_kg.pinned_tlb_mask = (~0ULL) << ((H2K_kg.last_tlb_index+1) & 0x3F);
    
    H2K_thread_context H2K_tc;
    
    __asm__ __volatile(
        " %0 = ssr \n"
        " %0 = extractu(%0,#7,#8)\n" 
        : "=r"(H2K_tc.ssr_asid));
    
    u32_t tlb_index_last_used = find_tlb_index_last_used();
    if (tlb_index_last_used >= H2K_kg.last_tlb_index) {
        FAIL("TLB free index not found to attempt set and invalidate");
    }
    
    u64_t tlb_entry = H2K_mem_tlb_read(tlb_index_last_used);
    
    u32_t va = H2K_LINK_ADDR; //TODO - find better va
    
    u32_t asid = H2K_tc.ssr_asid;
    
    set_entry_at_tlb_index(1+tlb_index_last_used, tlb_entry, va, asid);
    
    u32_t tlb_index = H2K_mem_tlb_probe(va, asid);
    if (tlb_index >= H2K_kg.last_tlb_index) {
        FAIL("TLB free index not set to attempt invalidate");
    }
    
    H2K_mem_tlb_invalidate_va(va, 1, asid, &H2K_tc);
    
    asid = H2K_tc.ssr_asid;
    tlb_index = H2K_mem_tlb_probe(va, asid);
    if (tlb_index < H2K_kg.last_tlb_index) {
        FAIL("TLB invalidate");
    }
    
    puts("TEST PASSED");
    return 0;
}
