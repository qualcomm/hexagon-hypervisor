/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RIL_DEFINES_V4_H
#define RIL_DEFINES_V4_H

#define addr_t          qc_addr_t
#define uint32_t        qc_uint32_t
#define uintptr_t       qc_uintptr_t

typedef unsigned long    u64_t;
typedef unsigned int     u32_t;
typedef unsigned short   u16_t;
typedef unsigned char    u8_t;
typedef signed long      s64_t;
typedef signed int       s32_t;
typedef signed short     s16_t;
typedef signed char      s8_t;
typedef u8_t             hwthread_t;
typedef u8_t             uint8_t;
typedef u8_t             coreid_t;
typedef u8_t             sched_dom_t;
typedef u8_t             cpuid_t;
typedef u32_t            word_t;
typedef u64_t            syscall_t;
typedef s32_t            hw_asid_t;
typedef int              prio_t;
typedef char*            cstring;
typedef u32_t            addr_t;
typedef u32_t            BTKey;
typedef u32_t            BTKeyCount;
typedef u32_t            uint32_t;
typedef u32_t            uintptr_t;
//typedef bool             _Bool;
typedef u32_t            L4_Word_t;
typedef word_t          atomic_word_t;
typedef void (*continuation_t)(void);
typedef word_t          bfl_t;
typedef word_t          qch_t;
typedef word_t          qtimer_cb_type;
typedef u64_t           qtimetick_type;
typedef word_t          qtimer_cb_data_type;
typedef word_t          qtimer_client_ptr;
typedef word_t          qmsgq_msg_ptr;
typedef word_t          qmutex_receiver_ptr;
typedef word_t          qpd_t;
typedef word_t          qmsgq_node_ptr;
typedef word_t          qmsgq_client_ptr;
typedef word_t          qhash_entry_ptr;
typedef word_t          qmutex_t;
typedef word_t          qch_receiver_ptr;
typedef word_t          L4_ThreadId_t;
typedef word_t          qtimetick_word_t;
#ifdef __WINDOWS__
  typedef word_t          size_t;
#endif

#define OSAM_QURT_VERSION   "0.1.0" //don't really have a convention here, starting with q6zip 0.1.0
//defines for v2p
#define KERNEL_OFFSET 0xF0000000
#define FOURK_MASK    0xFFFFF000
//Updated for 0.8 Specs
#define MASK_OFFSET   0x700000
#define PAGE_OFFSET   0xFFFFF
#define FOURK_OFFSET  0xFFF
#define MASK_SHIFT    20
#define FOURK_SHIFT   12
#define INVALID_PAGE  -1
#define MEM_INTERNAL  20
#define MAX_HTHREADS 6

//defines for QURTK kernel
#define MAX_INTERRUPTS      32
#define TCB_LIST            "QURTK_thread_contexts"
#define TCB_LIST_TCM        "QURTK_thread_contexts_tcm"
#define SPACE_LIST          "QURTK_pagetables"
#define RUN_LIST            "QURTK_runlist" // to deprecate, not used by MT version of OSAM
#define READY_LIST          "QURTK_ready" // to deprecate
#define INT_LIST            "QURTK_intqueue" // to deprecate
#define QURT_INIT           "qurt_has_initted"
#define IDLE_THREAD         "QURTK_idle_context"
#define THREAD_SWITCH       ".QURTK_thread_switch_ME_TO_NEW"
#define THREAD_SWITCH1      ".QURTK_thread_switch_ME_TO_NULL"
#define THREAD_SWITCH2      ".QURTK_thread_switch_NULL_TO_NEW"
#define THREAD_SWITCH3      ".QURTK_thread_switch_ME_TO_ISR"
#define MAX_THREADS         "QURTK_MAX_THREADS"
#define CONTEXT_SIZE        "QURTK_CONTEXT_SIZE"
#define MAX_THREADS_TCM     "QURTK_MAX_THREADS_IN_TCM"
#define QURT_MAX_L2VIC      "QURTK_int_max"
#define ISLAND_MODE         "QURTK_island_mode"

#define IDLE_COUNTER        "QURTK_idle_pcycles"
#define INT_TABLE           "QURTK_l2_interrupt_table"

//#define FUTEX_TABLE         "QURTK_futexhash"
#define FUTEX_TABLE         "QURTK_futex_objs" //kernel table of futexes that are blocked on
#define FUTEX_TABLE_END     "QURTK_futex_objs_end"
#define FUTEX_NUM           "QURTK_futex_num"
#define QURT_DEBUG_BUF      "QURTK_debug_buf"
#define DEBUG_BUF           "debug_buf"
#define DEBUG_BUF_INDEX     "debug_buf_index"
#define DEBUG_BUFFER_SIZE   8192
#define POOLS               "qurtos_phys_pool_list_head"
#define STATIC_MEMS         "qurtos_mmap_table"
#define SHMEM_LIST          "qurtos_shmem_allocate_list"
#define MAX_POOLS           32
#define MEM_MAX_IDX         20
#define TRACE_BUFFERS       "QURTK_trace_buffers"
#define TRACE_BUFFERS_DDR   "QURTK_trace_buffers_ddr"
#define TRACE_BUFFERS_TCM   "QURTK_trace_buffers_tcm"
#define TRACE_BUFFER_SIZE   "QURTK_trace_buffer_size"
#define TRACE_BUFFER_DDR_SIZE  "QURTK_trace_ddr_size"
#define TRACE_BUFFER_TCM_SIZE  "QURTK_trace_tcm_size"
#define VALID_HWT_MASK      "QURTK_hthread_startup"
#define ALLOC_MEMS          "qurtos_region_allocate_list"
#define CONFIG_BITMASK      "qurt_config_bitmask"
#define KERNEL_HEAP_USED    "max_heap_used"
#define APP_HEAP            "_Aldata"
#define APP_HEAP_SIZE       "QURTK_app_heap_size"
#define IMAGE_VEND          "image_vend"
#define IMAGE_VSTART        "image_vstart"
#define HEAP_BASE           "heapBase"
#define KHEAP_SIZE          "QURTK_KHEAP_SIZE"
#define KHEAP_BASE          "QURTK_heap"
//defines for POSIX
#define PTHREAD_LIST        "ltl_head"
#define PTHREAD_TABLE       "pthread_id_table"
#define PMSGQ_LIST          "mqlist"
#define MAX_MSGQS_VAR       "max_mq_in_system"
#define MAX_MSGQS_DEFAULT   64
#define POSIX_THREAD_MAGIC  0xABCD0001

//defines for q6zip if any of these is missing, functionality is not defined
#define START_VA_UNCOMPRESSED_RW    "start_va_uncompressed_rw"
#define END_VA_UNCOMRESSED_RW       "end_va_uncompressed_rw"
#define START_VA_UNCOMPRESSED_TEXT  "start_va_uncompressed_text"
#define RW_INFO                     "dlpager_rw_info"
#define RW_STATS                    "rw_stats"
#define START_VA_UNCOMPRESSED_TEXT  "start_va_uncompressed_text"
#define END_VA_UNCOMPRESSED_TEXT    "end_va_uncompressed_text"
#define RX_METADATA                 "dlpager_rx_metadata"
#define DICT1_BITS                  10 //needed for dll, not used
#define DICT2_BITS                  12 //needed for dll, not used
#define Q6ZIP_DICT_SIZE ((1<<DICT1_BITS)+(1<<DICT2_BITS)) //needed for dll, not used
#define Q6ZIP_VM_RW_OFFSET           0x80000000 //kind of arbitrary for now
#define DLPAGER_MAX_OUTSTANDING_WRITES  32 //DLPAGER_NUM_MAX_OUTSTANDING_WRITES_RW 32, per page

//stack ovf
#define STACK_MAGIC         0xf8f8f8f8
#define STACK_MAGIC_COUNT   128
#define STACK_FILL_CNT      "qurt_osam_stack_fill_count"

//defines for l4
#define RUNNABLE_STATE(id)  ((id << 1) | 0)
#define BLOCKED_STATE(id)   ((id << 1) | 1)

//defines for mp
#define MAX_PROC            32 //"MAX_ASIDS"
#define PROCESS_TABLE       "g_spaces"

#define REGION_LOCAL        0

/* for libc heap */
#define _MEMBND 4U /* 16-byte boundaries (2^^4) */
#define M_MASK  ((1 << _MEMBND) - 1)    /* rounds all sizes */

//L4 enums for thread state, page size and queue state
typedef enum
{
    running                 = RUNNABLE_STATE(1),
    waiting_forever         = BLOCKED_STATE(0xFFFFFFFF),
    waiting_timeout         = BLOCKED_STATE(2),
    locked_waiting          = BLOCKED_STATE(3),
    locked_running          = RUNNABLE_STATE(4),
    locked_running_ipc_done = RUNNABLE_STATE(9),
    polling                 = BLOCKED_STATE(5),
    halted                  = BLOCKED_STATE(6),
    aborted                 = BLOCKED_STATE(7),
    xcpu_waiting_deltcb     = BLOCKED_STATE(8),
    xcpu_waiting_exregs     = BLOCKED_STATE(12),
}thread_state_e;

typedef enum
{
    state_ready                 = 1,
    state_wakeup                    = 2,
    state_late_wakeup               = 4,
    state_wait                  = 8,
    state_send                  = 16,
    state_xcpu                  = 32,
}queue_state_e ;

//#ifndef T32
#if 0
union phv_t {
    struct qurt_context_prio
    {
        unsigned char prio;
        unsigned char hthread;
        unsigned char valid;
        unsigned char resv;
    }info;
    u32_t raw;
};
//#else
#endif
union phv_t {
    struct qurt_context_prio
    {
        unsigned char resv;
        unsigned char valid;
        unsigned char hthread;
        unsigned char prio;
    }info;
    u32_t raw;
};

#define EMPTY_ENTRY (0xffffffff)

//typedef struct pgent pgent_t;

#define SIZE_4K         (0x00001000)
#define SIZE_16K        (0x00004000)
#define SIZE_64K        (0x00010000)
#define SIZE_256K       (0x00040000)
#define SIZE_1M         (0x00100000)
#define SIZE_4M         (0x00400000)
#define SIZE_16M        (0x01000000)

u32_t QURT_pgsize_encode_to_size[7] = {
    SIZE_4K,
    SIZE_16K,
    SIZE_64K,
    SIZE_256K,
    SIZE_1M,
    SIZE_4M,
    SIZE_16M,
};

typedef enum pgsize_e {
    size_4k = 1,
    size_16k = 2,
    size_64k = 4,
    size_256k = 8,
    size_1m = 16,
    size_4m = 32,
    size_16m = 64,
    size_invalid,
    size_max = size_4m,
}pgsize_t;

//enumerations for TLB

union pgent_t {
         struct {
            /* For speed, same bits in same order as TLB */
                        //u32_t phys_addr       : 22; //addr
                        //u32_t global          : 1;
                        //u32_t trust           : 1;
                        //u32_t cache           : 4; //cfield
                        //u32_t usr             : 1;
                        //u32_t perm            : 3; //xrw
            u32_t perm      : 3;
            u32_t usr       : 1;
            u32_t cache     : 4;
            u32_t trust     : 1;
            u32_t global        : 1;
            u32_t phys_addr     : 22;
              } info; //X
              u32_t raw;
};

//#endif //end of T32
#endif /* RIL_DEFINES_H */
