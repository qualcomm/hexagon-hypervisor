/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef   QURTOS_INTERNAL_H
#define   QURTOS_INTERNAL_H

/*
||  qurtos_internal.h
||
||  QuRT OS has both a kernel portion which runs in monitor
||   mode and a qurtos portion which runs in guest-OS mode.
||
||  This include file contains macros, types, definitions,
||   declarations, etc., which are specific to the qurtos
||   portion of QuRT.
||
||  Nothing in this file is intended to be a public API and
||   everything in this file is to be considered subject to
||   change at any time.
*/
/* #ifndef MAKE_CONST_OBJ */
/* #include <qurt_config_vars.h> */
/* #endif */

/* #include <qurt_types.h> */
/* #include <stddef.h> */
/* #include <qurtk_stddef.h> */
/* #include <space.h> */
/* #include <consts.h> */
/* #include <context.h> */
/* #include <tlb.h> */
/* #include <qurt_util_macros.h> */

/* #include <qurt_alloc.h> */
/* #include <qurt_atomic_ops.h> */
/* #include <qurt_defs_internal.h> */
/* #include <qurt_memory.h> */
#include <qurt_mutex.h>
#include <qurt_rmutex.h>
#include <qurt_pimutex.h>
/* #include <qurt_qdi_internal.h> */
/* #include <qurt_qdi_driver.h> */
/* #include <qurt_space.h> */
/* #include <qurt_process.h> */
/* #include <qurt_utcb.h> */
/* #include <qurt_lifo.h> */
/* #include <qurt_int.h> */
/* #include <qurt_sem.h> */
/* #include <qurt_event.h> */
/* #include <qurt_thread_context.h> */
/* #include <qurt_island.h> */
#include <qurtos.h>
/* #include <qurtos_methods.h> */
/* #include <qurtos_constants.h> */
/* #include <qurtos_ktraps.h> */
/* #include <qurtos_shmem.h> */
#include <qurtos_timer_libs.h>
#include <qurtos_timer_server.h>
#include <qurt_timer_hw.h>
#include <qurt_sclk.h>
/* #include <obj_cache.h> */
/* #include <memheap_lite.h> */
/* #include <heap.h> */
#include <qurtos_sclk.h>
/* #include <debug_monitor.h> */
/* #include <stringK.h> */

#include <hexagon_protos.h>

/* struct qurtos_allocator { */
/*    /\* */
/*       A structure which contains pointers to matched */
/*       functions which provide malloc and free style */
/*       of memory allocation.  This is used to steer */
/*       allocations for any particular object to a */
/*       particular heap or object allocator in a way */
/*       that allows easy run-time selection of the */
/*       allocator on an object by object basis. */

/*       It is intended that structures contain a */
/*       pointer named pAllocator which points to */
/*       one of a small handful of const qurtos_allocator */
/*       structures: */

/*          struct xxx { */
/*             const struct qurtos_allocator *pAllocator; */
/*          }; */

/*       Example usage to create a new structure from */
/*       an existing structure, and to populate that */
/*       new structure with the same allocator: */

/*       new_obj = QURTOS_OBJECT_MALLOC(old_obj, sizeof(*new_obj)); */
/*       if (new_obj != NULL) { */
/*          QURTOS_OBJECT_SET_ALLOC(new_obj, old_obj); */
/*       } */

/*       Example usage to free an object with its own */
/*       allocator: */

/*       QURTOS_OBJECT_FREE(new_obj, new_obj); */

/*       Note that it's not mandatory for an object to */
/*       be allocated with its own allocator -- this can */
/*       happen if an object is allocated in one heap */
/*       but allocates its own objects from another heap. */
/*    *\/ */

/*    void *(*pfnMalloc)(size_t); */
/*    void (*pfnFree)(void *); */
/* }; */

/* #define QURTOS_OBJECT_MALLOC(obj, sz)            (*obj->pAllocator->pfnMalloc)(sz) */
/* #define QURTOS_OBJECT_FREE(obj, ptr)             (*obj->pAllocator->pfnFree)(ptr) */
/* #define QURTOS_OBJECT_SETALLOC(new_obj, old_obj) ((new_obj)->pAllocator = (old_obj)->pAllocator) */

/* struct qurtos_base_allocator { */
/*    const struct qurtos_allocator pAllocator[1]; */
/* }; */

/* extern const struct qurtos_base_allocator qurtos_main_allocator[1]; */
/* extern const struct qurtos_base_allocator qurtos_island_allocator[1]; */

/* struct qurtos_generic_method_handler { */
/*    int method __attribute__((aligned(8))); */
/*    unsigned handler; */
/* }; */

/* struct qurtos_client_info { */
/*    unsigned gp_value; */
/*    unsigned ssr_value; */
/*    unsigned ccr_value; */
/*    unsigned usr_value; */
/*    unsigned process_kill_capture_count; /\* used to distinguish kill vs exit *\/ */
/*    struct space *space;   /\* Pointer to space structure for this client *\/ */
/* }; */

/* struct qurtos_user_client { */
/*    qurt_qdi_obj_t qdiobj; */
/*    int handle; */
/*    unsigned asid; */
/*    const struct qurtos_allocator *pAllocator; */
/*    struct qurtos_client_info client_info; */
/*    int usermalloc_handle; */
/*    QURT_utcb_t *user_utcb; */
/*    int exitstatus; */
   
/*    qurt_mutex_t threadlist_mutex; */
/*    struct qurtos_thread_info *threadlist; */
/*    struct qurtos_user_mapping *mappingList; */
/*    qurt_mutex_t mappingListLock; */
/*    char name[64]; */

/*    qurt_qdi_obj_t **htable; */
/*    int loader_handle; */
/*    int hmax; */
/*    int htoffset; */
/*    void *err_thread_info; */
/*    void *memfree; */
/*    phys_pool_t hlos_pool; */
/*    struct phys_mem_pool_config hlos_pool_config; */
/* }; */

/* struct qurtos_thread_info { */
/*    qurt_qdi_obj_t qdiobj __attribute__((aligned(8)));    /\* Forces entire structure to be 8-byte aligned *\/ */
/*    QURTK_thread_context *pTcb; */
/*    QURTK_thread_context *pTcbParent; */
/*    struct qurtos_thread_info *exited_children; */
/*    struct qurtos_thread_info *pNext; */
/*    struct qurtos_thread_info *pPrev; */
/*    struct qurtos_thread_info *pJoiner; */
/*    struct qurtos_thread_info *pClientNext; */
/*    unsigned short asid; */
/*    unsigned short bFinalExit; */
/*    int thread_handle; */
/*    int exit_status; */
/*    int join_status; */
/*    int remote_client_handle; */
/*    void (*pfnCleanup)(void *); */
/*    void *pArgCleanup; */
/*    struct qurtos_thread_info *reaper_prev; */
/*    struct qurtos_thread_info *reaper_next; */
/*    void *pInvokeSP; */
/* }; */

/* mem_t *qurtos_mem_t_alloc(void); */
/* void qurtos_mem_t_free(mem_t *p); */
/* vma_node_t *qurtos_vma_node_t_alloc(void); */
/* void qurtos_vma_node_t_free(vma_node_t *p); */
/* phys_mem_region_t *qurtos_phys_mem_region_t_alloc(void); */
/* void qurtos_phys_mem_region_t_free(phys_mem_region_t *p); */
/* shmem_t *qurtos_shmem_t_alloc(void); */
/* void qurtos_shmem_t_free(shmem_t *p); */
/* int qurtos_usermalloc_create(qurt_qdi_obj_t *obj); */

/* void qurtos_thread_init(void); */
/* struct qurtos_thread_info *qurtos_thread_info_init(struct qurtos_thread_info *pInfo, QURTK_thread_context *pTcb, int client_handle); */

/* void qurtos_rand_late_init(void); */
/* void qurtos_app_heap_late_init(void); */
/* void qurtos_space_early_init(void); */
/* void qurtos_qdi_early_init(void); */
/* void qurt_qdi_initialize(void *td); */
/* void qurtos_memory_init(void); */
/* int qurt_qdi_client_signal_group_new(int, int *, int *); */
/* void qurtos_thread_generic_init(void); */
/* void qurtos_space_generic_init(void); */
/* void qurtos_user_client_generic_init(void); */
/* void qurtos_reaper_generic_init(void); */
/* void qurtos_thread_context_generic_init(void); */
/* void qurtos_timer_server_generic_init(void); */
/* void qurtos_sclk_generic_init(void); */
/* void qurtos_memory_generic_init(void); */
/* void qurtos_debugger_generic_init(void); */

/* void qurtos_space_inform_process_exit(int asid, int status); */
/* space_t *get_space_from_client_handle(int handle); */
/* int get_client_handle_from_asid(int asid); */
/* void qurtos_mark_trace_memory(unsigned, unsigned); */
/* void zero_mem_range_c(unsigned, unsigned); */
/* int tlb_remove_mapping(unsigned); */
/* void qurtos_timer_init(void); */
/* int wrap_mem_region(int client_handle, vma_node_t *vma, bool is_static); */
/* int qurtos_mem_fs_change_page_attr ( u32_t addr, u32_t len, rwx_t perm, int client_handle); */
/* int qurtos_mem_fs_region_create (u32_t *region_handle, u32_t addr, size_t size, int flags, int client_handle); */
/* int qurtos_mem_fs_region_get_virtaddr (unsigned int *vaddr, unsigned int region_obj); */
/* int qurtos_mem_fs_region_delete (unsigned int region_handle, int client_handle); */
/* int qurtos_mem_fs_region_query (u32_t *region_handle, unsigned int addr, int client_handle); */
int qurtos_sysclock_get_hw_ticks(unsigned long long *ticks);
h2_u64_t hw_timer_match_val(void);
void qurt_sysclock_init(void);
/* unsigned qurtos_rand32(void); */
/* unsigned qurtos_randrange(unsigned top); */
/* void qurtos_rand_prestart(void); */
/* void qurtos_revectored_exit(void); */
/* int qurtos_process_kill_client(int client_handle, struct qurtos_user_client *user_client, unsigned int exclude_tcb); */
/* void shmem_region_create_from_static_mapping( */
/*                 mem_pool_t*, */
/*                 vma_node_t **vma,  */
/*                 space_t *space,  */
/*                 u32_t page_count,  */
/*                 u32_t pageno_phys, */
/*                 u32_t pageno_virt,  */
/*                 qurt_pgattr_t pga, */
/*                 perm_t perm); */

/* void qurtos_heap_add_random(qurtos_mem_heap_type *, unsigned, unsigned); */

/* u32_t lookup_user_mapping (space_t *space, u32_t vaddr); */
/* qurt_qdi_obj_t *qurtos_get_local_client(void); */

/* int qurt_qdi_state_local_new_handle_from_obj(qurt_qdi_obj_t **table, */
/*                                              int max, */
/*                                              qurt_qdi_obj_t *objptr); */
/* int qurt_qdi_state_local_release_handle(qurt_qdi_obj_t **table, */
/*                                         int max, */
/*                                         int h); */
/* void qurtos_start_reaper(unsigned long long *stack, */
/*                          unsigned stack_size, */
/*                          struct qurtos_thread_info *info, */
/*                          struct QURT_ugp_ptr *ugp); */

/* int qurtos_qdi_generic_entry(int client_handle, qurt_qdi_obj_t *objptr, int method, */
/*                              qurt_qdi_arg_t a1, qurt_qdi_arg_t a2, qurt_qdi_arg_t a3, */
/*                              qurt_qdi_arg_t a4, qurt_qdi_arg_t a5, qurt_qdi_arg_t a6, */
/*                              qurt_qdi_arg_t a7, qurt_qdi_arg_t a8, qurt_qdi_arg_t a9); */

/* void qurtos_qdi_generic_early_init(void); */

/* void qurtos_qdi_generic_register_methods(const struct qurtos_generic_method_handler *, unsigned); */
/* void QURTK_zero_mem_range(void *mem_start, void *mem_end); */
/* void qurtos_object_init(void); */

/* void qurtos_island_error(int client_handle, qurt_qdi_obj_t *obj, int method, qurt_qdi_arg_t a1); */

/* #define QURTOS_ASIZE(x) (sizeof(x)/sizeof((x)[0])) */
/* #define QURTOS_GENERIC_FN(pfn) ((unsigned)(pfn)) */
/* #define QURTOS_GENERIC_OBJ(objptr) ((unsigned)(objptr)+1) */
/* #define QURTOS_ERROR(e) ({int __e=(e); (__e < 0) ? (__e) : (-__e);})   // Force any error code to be negative for QDI */

/* #include <qurtos_global_vars.h> */

#ifdef CONFIG_PRIORITY_INHERITANCE
#define qurtos_timer_lock(lock)     qurt_pimutex_lock(lock)
#define qurtos_timer_unlock(lock)   qurt_pimutex_unlock(lock)
#else
#define qurtos_timer_lock(lock)     qurt_rmutex_lock(lock)
#define qurtos_timer_unlock(lock)     qurt_rmutex_unlock(lock)
#endif

#ifndef MAIN_ONLY
#define INCLUDE_ISLAND_CONTENTS
#endif
#ifndef ISLAND_ONLY
#define INCLUDE_MAIN_CONTENTS
#endif

#define PROCESS_KILL_CAPTURECNT_MAGIC   0x10000  
#endif // QURTOS_INTERNAL_H
