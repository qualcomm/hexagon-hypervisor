/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_H
#define QURT_H 1

#include <h2.h>

#define RENAME_PREFIX qurt
#include <h2_rename.h>
#undef RENAME_PREFIX

#include <qurt_fd.h>
#include <qurt_tls.h>
#include <qurt_power.h>

#include <stddef.h>
#include <assert.h>

//  Signal used to indicate to anybody waiting for an interrupt that
//  it was deregistered.

#define SIGNAL_INT_ABORT 31
#define SIG_INT_ABORT (1<<SIGNAL_INT_ABORT)

#define EOK 0

#define QURT_MAX_HTHREADS MAX_HTHREADS
#define QURTK_MAX_HTHREADS QURT_MAX_HTHREADS

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

static inline void qurt_pmu_enable(int x) { }
static inline void qurt_pmu_disable(int x) { }

static inline void qurt_rmutex_destroy(qurt_rmutex_t *lock)
{
	return;
}

// because qurt_mutex_t is used for both mutex and rmutex
// qurt_mutex_t is typdeffed to h2_rmutex_t
static inline void qurt_mutex_lock(qurt_mutex_t *lock)
{
	h2_rmutex_lock(lock);
}

static inline void qurt_mutex_unlock(qurt_mutex_t *lock)
{
	h2_rmutex_unlock(lock);
}

static inline int qurt_mutex_trylock(qurt_mutex_t *lock)
{
	return h2_rmutex_trylock(lock);
}

static inline void qurt_mutex_init(qurt_mutex_t *lock)
{
	h2_rmutex_init(lock);
}
static inline void qurt_mutex_destroy(qurt_mutex_t *lock)
{
	return;
}

// fake up qurt_pimutex as h2_rmutex
static inline void qurt_pimutex_init(qurt_mutex_t *lock)
{
	h2_rmutex_init(lock);
}

static inline void qurt_pimutex_destroy(qurt_mutex_t *lock)
{
	return;
}

static inline void qurt_pimutex_lock(qurt_mutex_t *lock)
{
	h2_rmutex_lock(lock);
}

static inline void qurt_pimutex_unlock(qurt_mutex_t *lock)
{
	h2_rmutex_unlock(lock);
}

static inline int qurt_pimutex_trylock(qurt_mutex_t *lock)
{
	return h2_rmutex_trylock(lock);
}

int qurt_thread_join(int threadid, int *status);

static inline unsigned short qurt_sem_get_val(volatile h2_sem_t *sem)
{
	return sem->val;  
}

static inline void qurt_sem_destroy(volatile h2_sem_t *sem)
{
	return;
}

// because qurt_mutex_t is used for both mutex and rmutex
// qurt_mutex_t is typdeffed to h2_rmutex_t
static inline void qurt_cond_wait(h2_cond_t *cond, h2_rmutex_t *mutex)
{
	h2_cond_wait_rmutex(cond, mutex);
}

static inline void qurt_cond_destroy(h2_cond_t *cond) 
{
	return;
}

static inline void qurt_barrier_destroy(h2_barrier_t *barrier)
{
	return;
}

static inline void qurt_thread_set_name(unsigned long long name0, 
	unsigned long long name1)
{
	return;
}

/* int qurt_thread_create(void *pc, void *stack, void *arg,  */
/*    		unsigned int prio, unsigned int asid, unsigned int hw_bitmask); */

void qurt_thread_exit(int status);

static inline int qurt_prio_get(unsigned int threadid)
{
	return -1; 
}

static inline int qurt_prio_set(unsigned int threadid,unsigned int newprio)
{
	return h2_set_prio(threadid,newprio);
}

int qurt_register_interrupt(int int_num, 
	h2_anysignal_t *int_signal, int signal_mask);

unsigned int qurt_deregister_interrupt(int int_num);

static inline int qurt_ack_interrupt(int intno)
{
	return 0;
}

static inline void qurt_register_fastint(int intno, int (*fn)(int))
{
	h2_register_fastint(32+intno,fn);
}

static inline void qurt_deregister_fastint(int intno)
{
	h2_deregister_fastint(32+intno);
}

static inline void qurt_exit(int status)
{
	exit(status);
}

/* from qurt_utcp.h */
#define qurt_get_my_utcb(pUgp)        __asm__ __volatile__ ( " %0 = ugp " :"=r"(pUgp) ) ; 

static inline void qurt_anysignal_destroy(qurt_anysignal_t *signal)
{
	return;
}

unsigned int qurt_get_my_anysignal();

//void qurt_deregister_fastint(int intno);

//  Not really a part of the QURT API, but it needs to happen somewhere.
void l2_controller_init(void);

/* Until we have something in H2 side to do similar things... */
static inline void qurt_profile_enable(int enable) { return; }

static inline void qurt_profile_reset_idle_pcycles(void) { return; }
static inline void qurt_profile_reset_thread_pcycles(int thread_id) { return; }

static inline void qurt_profile_get_idle_pcycles(unsigned long long *pcycles) { return; }
static inline void qurt_profile_get_thread_pcycles(int thread_id, unsigned long long  *pcycles) { *pcycles = h2_get_tcycles(); return; }

#define QURT_THREAD_ATTR_NAME_MAXLEN     16
#define QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT  0
#define QURT_THREAD_ATTR_PRIORITY_DEFAULT       256
#define QURT_THREAD_ATTR_ASID_DEFAULT           0
#define QURT_THREAD_ATTR_AFFINITY_DEFAULT      (-1)
#define QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT   (-2)
	
typedef struct _qurt_thread_attr {
	char name[QURT_THREAD_ATTR_NAME_MAXLEN]; /**< Thread name */
	unsigned char tcb_partition;  /**< Should the thread TCB reside in RAM or
					 on chip memory (i.e. TCM) */
	unsigned char affinity;       /**< HW bitmask indicating the threads it
					 can run on */
	unsigned short priority;      /**< Thread priority */
	unsigned short asid;          /**< Address space ID */
	unsigned short timetest_id;   /**< TIMETEST ID */
	unsigned int stack_size;      /**< Thread's stack size */
	void *stack_addr;             /**< Stack address base.
					 (stack_addr, stack_addr+stack_size-1)
					 is the range of the stack */
} qurt_thread_attr_t;
	
static inline void qurt_thread_attr_init (qurt_thread_attr_t *attr)
{

	attr->name[0] = 0;
	attr->tcb_partition = QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT;
	attr->priority = QURT_THREAD_ATTR_PRIORITY_DEFAULT;
	attr->asid = QURT_THREAD_ATTR_ASID_DEFAULT;
	attr->affinity = QURT_THREAD_ATTR_AFFINITY_DEFAULT;
	attr->timetest_id = QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT;
	attr->stack_size = 0;
	attr->stack_addr = 0;
}

static inline void qurt_thread_attr_set_name (qurt_thread_attr_t *attr, char *name)
{
	return;
}

static inline void qurt_thread_attr_set_tcb_partition (qurt_thread_attr_t *attr, unsigned short tcb_partition)
{
	attr->tcb_partition = tcb_partition;
}

static inline void qurt_thread_attr_set_priority (qurt_thread_attr_t *attr, unsigned short priority)
{
	attr->priority = priority;
}

static inline void qurt_thread_attr_set_affinity (qurt_thread_attr_t *attr, unsigned char affinity)
{
	attr->affinity = affinity;
}

static inline void qurt_thread_attr_set_timetest_id (qurt_thread_attr_t *attr, unsigned short timetest_id)
{
	attr->timetest_id = timetest_id;
}

static inline void qurt_thread_attr_set_stack_size (qurt_thread_attr_t *attr, unsigned int stack_size)
{
	attr->stack_size = stack_size;
}

static inline void qurt_thread_attr_set_stack_addr (qurt_thread_attr_t *attr, void *stack_addr)
{
	attr->stack_addr = stack_addr;
}

typedef unsigned int qurt_thread_t;

int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void (*entrypoint) (void *), void *arg);
//static inline int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void *entrypoint, void *arg)
/* { */
/* 	*thread_id = h2_thread_create((void *)entrypoint, attr->stack_addr, arg, (unsigned int)attr->priority); */
/* 	return (*thread_id == 0) ? QURT_EFATAL : EOK; */
/* } */

//Stub, but probably fine
static inline int qurt_thread_resume(unsigned int thread_id) { 
	assert(0);
	return EOK;
}

unsigned int qurt_interrupt_status(int int_num, int *status);

unsigned int qurt_interrupt_clear(int int_num);

// Thread configs are ignored now, but the need to be defined for sources:
#define QURT_THREAD_CFG_BITMASK_HT0      0x00000001
#define QURT_THREAD_CFG_BITMASK_HT1      0x00000002
#define QURT_THREAD_CFG_BITMASK_HT2      0x00000004
#define QURT_THREAD_CFG_BITMASK_HT3      0x00000008
#define QURT_THREAD_CFG_BITMASK_HT4      0x00000010
#define QURT_THREAD_CFG_BITMASK_HT5      0x00000020
#define QURT_THREAD_CFG_BITMASK_ALL      0x000000ff

#define QURT_THREAD_CFG_USE_RAM          0x00000000
#define QURT_THREAD_CFG_USE_TCM          0x00000100

#define qurt_thread_wait_for_idle  qurt_power_wait_for_idle
#define qurt_thread_wait_for_active qurt_power_wait_for_active

#define     qurt_allsignal_set                    qurt_allsignal_signal          
#define         qurt_profile_get_thread_pcycles	  qurt_get_pcycles               
#define         qurt_profile_get_thread_tcycles	  qurt_get_tcycles               
#define     qurt_fastint_register		  qurt_register_fastint          
#define     qurt_fastint_deregister		  qurt_deregister_fastint        
#define     qurt_isr_deregister			  qurt_isr_deregister            
#define     qurt_isr_register			  qurt_isr_register              
						                                 
#define     qurt_interrupt_get_config		  qurt_interrupt_getconfig       
#define     qurt_interrupt_get_registered	  qurt_get_registered_interrupts 
#define     qurt_interrupt_register		  qurt_register_interrupt        
#define     qurt_interrupt_deregister		  qurt_deregister_interrupt      
#define     qurt_interrupt_set_config		  qurt_interrupt_setconfig       
#define     qurt_interrupt_acknowledge		  qurt_ack_interrupt             
						                                 
#define     qurt_mapping_remove			  qurt_remove_mapping            
#define     qurt_mapping_create			  qurt_create_mapping            
						                                 
#define   qurt_mutex_try_lock			  qurt_mutex_trylock             
#define   qurt_pimutex_try_lock			  qurt_pimutex_trylock           
#define   qurt_pipe_receive			  qurt_pipe_recv                 
#define   qurt_pipe_try_receive			  qurt_pipe_try_recv             
#define   qurt_power_wait_for_idle		  qurt_thread_wait_for_idle      
#define   qurt_power_wait_for_active		  qurt_thread_wait_for_active    
						                                 
#define   qurt_thread_set_priority		  qurt_prio_set                  
#define   qurt_thread_get_priority		  qurt_prio_get                  
#define   qurt_rmutex_try_lock			  qurt_rmutex_trylock            
#define   qurt_rmutex_try_lock_block_once	  qurt_rmutex_trylock_block_once 
						                                 
#define   qurt_sem_try_down			  qurt_sem_trydown               
#define   qurt_thread_get_timetest_id		  qurt_thread_get_tid            
#define   qurt_thread_set_timetest_id		  qurt_thread_set_tid            
#define   qurt_thread_get_id			  qurt_thread_myid               
						                                 
#define   qurt_thread_set_affinity		  qurt_thread_set_hw_bitmask     
						                                 
#define    qurt_tlb_delete_mapping		  qurt_tlb_mapping_delete        
#define    qurt_tlb_create_mapping		  qurt_tlb_mapping_create        
#define    qurt_tlb_set_entry			  qurt_tlb_setentry              
#define    qurt_tlb_get_entry			  qurt_tlb_getentry              
#define    qurt_tlb_get_mapping			  qurt_tlb_query                 
	   					  			       
#define    qurt_tls_delete_key			  qurt_tls_key_delete            
#define    qurt_tls_set_specific		  qurt_tls_setspecific           
#define    qurt_tls_get_specific		  qurt_tls_getspecific           
#define    qurt_tls_create_key			  qurt_tls_key_create            

#define    qurt_pmu_set(...) /*  qurt_pmu_setreg */
#define    qurt_pmu_get(...) /* qurt_pmu_getreg */ 0

#define    qurt_get_pcycles                   qurt_profile_get_thread_pcycles
#define    qurt_profile_get_threadid_pcycles  qurt_profile_get_thread_pcycles 
#define    qurt_profile_reset_threadid_pcycles qurt_profile_reset_thread_pcycles

#define    qurt_system_sclk_attr_gethwticks qurt_sysclock_get_hw_ticks 

	/* Some qurt tests call blast_malloc, grr */
#define blast_malloc qurt_malloc

/* Internal */
/* #define blast_tls_init               qurt_tls_init */
/* #define BLAST_tls_reserve            QURT_tls_reserve */

/* #define BLAST_ugp_ptr        QURT_ugp_ptr */
/* #define BLAST_MAX_TLS        QURT_MAX_TLS */
/* #define BLAST_MAX_TLS_INDEX        QURT_MAX_TLS_INDEX */
/* #define BLAST_utcb_t        QURT_utcb_t */
/* #define BLAST_UTCB        QURT_UTCB */

/* #define blast_get_trace_marker        qurt_trace_get_marker    */
/* #define blast_has_preempt_trace(a)    qurt_trace_changed(a,0x3)   */

/* #define blast_perm_t        qurt_perm_t */
/* #define BLAST_MEM_CACHE_WRITETHROUGH_L2CACHEABLE        QURT_MEM_CACHE_WRITETHROUGH_L2CACHEABLE */
/* #define BLAST_MEM_CACHE_NONE_SHARED        QURT_MEM_CACHE_NONE_SHARED */
/* #define BLAST_MEM_CACHE_WRITETHROUGH_NONL2CACHEABLE        QURT_MEM_CACHE_WRITETHROUGH_NONL2CACHEABLE */
/* #define BLAST_MEM_CACHE_WRITEBACK_NONL2CACHEABLE        QURT_MEM_CACHE_WRITEBACK_NONL2CACHEABLE */
/* #define BLAST_PERM_EXECUTE        QURT_PERM_EXECUTE */
/* #define BLAST_MEM_CACHE_WRITEBACK_L2CACHEABLE        QURT_MEM_CACHE_WRITEBACK_L2CACHEABLE */
/* #define BLAST_PERM_READ        QURT_PERM_READ */
/* #define BLAST_PERM_FULL        QURT_PERM_FULL */
/* #define BLAST_PERM_WRITE        QURT_PERM_WRITE */
/* #define BLAST_MEM_CACHE_NONE        QURT_MEM_CACHE_NONE */
 
/* #define blast_printf qurt_printf */
/* #define blast_power_shutdown(a)  qurt_power_shutdown_enter((unsigned int)QURT_POWER_SHUTDOWN_TYPE_L2NORET) */

#define qurt_power_shutdown_enter(a) qurt_power_shutdown()

/* #define blast_system_sclk_attr_gethwticks  qurt_sysclock_get_hw_ticks */

/* #define blast_system_sclk_register qurt_sysclock_register */
/* #define blast_system_sclk_alarm  qurt_sysclock_alarm_create */
/* #define blast_system_sclk_timer  qurt_sysclock_timer_create */

/* #define blast_reg_error_handler  qurt_exception_wait */
/* #define blast_exit  qurt_exception_raise_nonfatal */
/* #define blast_fatal_exit qurt_exception_raise_fatal */

/* #define blast_mem_region_attr_t qurt_mem_region_attr_t */
/* #define BLAST_PMUEVTCFG                   QURT_PMUEVTCFG     */
/* #define BLAST_PMUCFG                      QURT_PMUCFG  */

/* #define EOK                             QURT_EOK  */
/* #define EVAL                            QURT_EVAL */
/* #define EMEM                            QURT_EMEM */
/* #define EINVALID                        QURT_EINVALID */
/* #define EFAILED                         QURT_EFAILED */
/* #define ENOTALLOWED                     QURT_ENOTALLOWED */
/* #define EDUP_CLSID                      QURT_EDUPCLSID */
/* #define EBADPARM                        QURT_EINVALID */
/* #define EINVALIDITEM                    QURT_EINVALID */
/* #define EBADHANDLE                      QURT_EINVALID */
/* #define ENO_INTERRUPTS                  QURT_ENOREGISTERED */
/* #define EPC_ISDB                        QURT_EISDB */
/* #define EPC_NOSTM                       QURT_ESTM */
/* #define E_TLS_NOAVAIL                   QURT_ETLSAVAIL   */
/* #define E_TLS_NOENT                     QURT_ETLSENTRY    */
/* #define EINT                            QURT_EINT   /\**< Invalid interrupt number (not registered). *\/ */
/* #define ESIG                            QURT_ESIG   /\**< Invalid signal bitmask (can't set more than one signal at a time) *\/ */
/* #define E_OUTOFHEAP                     QURT_EHEAP   /\**< Out of heap *\/ */
/* #define E_MM_OUT_OF_RANGE               QURT_EMEMMAP */
/* #define ENO_THREAD                      QURT_ENOTHREAD */
/* #define E_L2CACHABLE_NOT_SUPPORTED      QURT_EL2CACHE */
/* #define E_FATAL                         QURT_EFATAL    /\**< FATAL error that shall never happens *\/ */
/* #define E_INT_DEREGISTER                QURT_EDEREGISTERED    /\**< error interrupt is already de-registered*\/ */
/* #define E_TLB_CREATE_SIZE               QURT_ETLBCREATESIZE   /\**< TLB create error: incorrect sizes*\/ */
/* #define E_TLB_CREATE_UNALIGNED          QURT_ETLBCREATEUNALIGNED    /\**< TLB create error: unaligned address*\/ */

/* #define EVENT_PAGEFAULT      QURT_EVENT_PAGEFAULT /\**< Page fault Event *\/ */
/* #define EVENT_SYSTEM_ERR     QURT_EVENT_SYSTEM_ERR /\**< System Error Event *\/ */
/* #define EVENT_SUSPEND        QURT_EVENT_SUSPEND */

/* #define ENV_OBJ_SWAP_POOLS   QURT_ENV_OBJ_SWAP_POOLS /\**< Swap spool object *\/ */
/* #define ENV_OBJ_APP_HEAP     QURT_ENV_OBJ_APP_HEAP /\**< Application heap object *\/ */
/* #define ENV_OBJ_TIMER        QURT_ENV_OBJ_TIMER /\**< Timer object *\/ */
/* #define ENV_OBJ_ARCH_VER     QURT_ENV_OBJ_ARCH_VER /\**< Architecture version object *\/ */

/* #define PIPE_MAGIC  QURT_PIPE_MAGIC */

/* #define MAX_QURT_SCLK_CLIENTS      QURT_SCLK_CLIENTS_MAX */

/* #define BLAST_MAX_HTHREAD_LIMIT  QURT_MAX_HTHREAD_LIMIT */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

