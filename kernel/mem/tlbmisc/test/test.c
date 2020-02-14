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
    u32_t index_last_used = 0;  
    H2K_mem_tlbfmt_t entry;
    entry.raw = 0;
    for (u32_t index = 0; index <= H2K_kg.last_tlb_index; index++) {
        entry.raw = H2K_mem_tlb_read(index);
        if (entry.valid != 0) {
            index_last_used = index;
        }
    }
    return index_last_used;
}

u32_t find_tlb_index_first_free(void) {
    u32_t index_first_free = H2K_kg.last_tlb_index + 1;  
    H2K_mem_tlbfmt_t entry;
    entry.raw = 0;
    for (u32_t index = 0; index <= H2K_kg.last_tlb_index; index++) {
        entry.raw = H2K_mem_tlb_read(index);
        if (entry.valid == 0) {
            index_first_free = index;
            break;
        }
    }
    return index_first_free;
}

u64_t update_tlb_entry_va_asid(u64_t tlb_entry, u32_t va, u32_t asid) {
    H2K_mem_tlbfmt_t entry;
    entry.raw = tlb_entry;  // with original va & asid
    entry.vpn = (va >> PAGE_BITS);
    entry.asid = asid;
    return entry.raw;  // with updated va & asid
}

void set_tlb_entry_at_table_index(u32_t tlb_index, u64_t tlb_entry) {
    H2K_spinlock_lock(&H2K_kg.tmpmap_lock);  // for tmpmap
    H2K_mutex_lock_tlb();  // for H2K_safemem
    H2K_mem_tlb_write(tlb_index, tlb_entry);
    H2K_isync();
    H2K_mutex_unlock_tlb();  // for H2K_safemem
    H2K_spinlock_unlock(&H2K_kg.tmpmap_lock);  // for tmpmap
}

int main()
{
    __asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
    H2K_kg.tlb_size = 0x80;  // tlb size value in init unit test
    H2K_kg.last_tlb_index = H2K_kg.tlb_size - 1;
    
    H2K_thread_context H2K_tc;
    u32_t tlb_index_last_used = find_tlb_index_last_used();  // used tlb index to get tlb entry template
    u32_t tlb_index_first_free = find_tlb_index_first_free();  // free tlb index to overwrite
    if (tlb_index_first_free >= H2K_kg.last_tlb_index) {
        FAIL("TLB free index not found to attempt set and invalidate va");
    }
    
    u64_t tlb_entry = H2K_mem_tlb_read(tlb_index_last_used);  // tlb entry as template
    u32_t va = H2K_LINK_ADDR;  // VA set H2K_LINK_ADDR OK to use if just within a unit test 
    volatile u32_t asid = 0; // ASID set for running thread
    __asm__ __volatile(
        " %0 = ssr \n"
        " %0 = extractu(%0,#7,#8)\n" 
        : "=r"(asid));
    
    tlb_entry = update_tlb_entry_va_asid(tlb_entry, va, asid);  // ensure tlb entry not duplicate va by update  
    set_tlb_entry_at_table_index(tlb_index_first_free, tlb_entry);  // overwrite free tlb index's entry with va update
    u32_t tlb_index = H2K_mem_tlb_probe(va, asid); // check overwrite of free tlb index's entry with va update
    if (tlb_index != tlb_index_first_free) {
        FAIL("TLB free index set attempt");
    }
    
    H2K_mem_tlb_invalidate_va(va, 1, asid, &H2K_tc);  // invalidate tlb entry for va & asid at previously free index
    tlb_index = H2K_mem_tlb_probe(va, asid);  // check invalidate of re-freed tlb index's entry
    if (tlb_index < H2K_kg.last_tlb_index) {
        FAIL("TLB invalidate");
    }
    
    puts("TEST PASSED");
    return 0;
}
