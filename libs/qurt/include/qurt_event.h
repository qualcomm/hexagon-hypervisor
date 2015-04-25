/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_EVENT_H
#define QURT_EVENT_H
/**
  @file qurt_event.h
  @brief Prototypes of Kernel event API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include "qurt_consts.h"
#include "qurt_thread.h"
#include "qurt_prelim.h"

/*
 * System environment object type.
 */
/**@addtogroup sys_env_types
@{ */
/*swap pool information type*/
typedef struct qurt_sysenv_swap_pools {
   /** @cond */
   unsigned int spoolsize; /*swap pool size*/
   unsigned int spooladdr;   /*swap pool start address*/
   /** @endcond */
}qurt_sysenv_swap_pools_t;

/*application heap information type*/
typedef struct qurt_sysenv_app_heap {
   /** @cond */
   unsigned int heap_base; /*heap base address*/
   unsigned int heap_limit; /*heap end address*/
   /** @endcond */
} qurt_sysenv_app_heap_t ;

/*architecture version information type*/
typedef struct qurt_sysenv_arch_version {
   /** @cond */
    unsigned int arch_version; /*architecture version*/
    /** @endcond */
}qurt_arch_version_t;

/*maximum hardware threads information type*/
typedef struct qurt_sysenv_max_hthreads {
   /** @cond */
   unsigned int max_hthreads; /*maximum number of hardware threads*/
   /** @endcond */
}qurt_sysenv_max_hthreads_t;

/*maximum pi priority information type*/
typedef struct qurt_sysenv_max_pi_prio {
     /** @cond */
    unsigned int max_pi_prio; /*max pi priority*/
     /** @endcond */
}qurt_sysenv_max_pi_prio_t;

/*timer information type*/
typedef struct qurt_sysenv_timer_hw {
     /** @cond */
   unsigned int base; /*time frame address base*/
   unsigned int int_num; /* timer interrupt number*/
    /** @endcond */
}qurt_sysenv_hw_timer_t;

/*process name information*/
typedef struct qurt_sysenv_procname {
     /** @cond */
   unsigned int asid; /*address space ID*/
   char name[QURT_MAX_NAME_LEN]; /*process name*/
    /** @endcond */
}qurt_sysenv_procname_t;

/*
 QuRT system error event
 */
typedef struct _qurt_sysevent_error_t
{   
    /** @cond */
    unsigned int thread_id;
    unsigned int fault_pc;
    unsigned int sp;
    unsigned int badva;
    unsigned int cause;
    unsigned int ssr;
    /** @endcond */
} qurt_sysevent_error_t ;

/** Page fault error event information. */
typedef struct qurt_sysevent_pagefault {
    qurt_thread_t thread_id; /**< Thread ID of the page fault thread. */
    unsigned int fault_addr; /**< Accessed address that caused the page fault. */
} qurt_sysevent_pagefault_t ;
/** @} */ /* @endaddtogroup sys_env_types */

/* EJP: some of this is implemented badly, in my opinion.  But I guess I understand the desire to 
 * be able to change the struct later on, instead of having to change the interface to add more values.
 * Still, it seems like this is just querying a set of keys that can return values.  Should be a better
 * interface than this stuff.
 */

/**
  Gets the environment swap pool 0 information from the kernel.

  @datatypes
  #qurt_sysenv_swap_pools_t
               
  @param[out] pools  Pointer to the pools information.

  @return            
  EOK -- SUCCESS.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_swap_spool0 (qurt_sysenv_swap_pools_t *pools )
{
	R_UNSUPPORTED;
}

/*
  Gets the environment swap pool 1 information from the kernel.
  
  @datatypes
  #qurt_sysenv_swap_pools_t

  @param[out] pools  Pointer to the pools information.

  @return
  EOK -- SUCCESS.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_swap_spool1(qurt_sysenv_swap_pools_t *pools )
{
	R_UNSUPPORTED;
}

/**@ingroup func_qurt_sysenv_get_app_heap  
  Gets information on the program heap from the kernel.
   
  @datatypes
  #qurt_sysenv_app_heap_t

  @param[out] aheap  Pointer to information on the program heap.

  @return
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter.
               
  @dependencies
  None.
*/

int qurt_sysenv_get_app_heap(qurt_sysenv_app_heap_t *aheap);

/**@ingroup func_qurt_sysenv_get_hw_timer
  Gets the memory address of the hardware timer from the kernel.
  
  @datatypes
  #qurt_sysenv_hw_timer_t

  @param[out] timer  Pointer to the memory address of the hardware timer.

  @return
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_hw_timer(qurt_sysenv_hw_timer_t *timer )
{
	R_UNSUPPORTED;
}

/**@ingroup func_qurt_sysenv_get_arch_version
  Gets the Hexagon processor architecture version from the kernel.

  @datatypes
  #qurt_arch_version_t

  @param[out] vers  Pointer to the Hexagon processor architecture version.

  @return 
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_arch_version(qurt_arch_version_t *vers)
{
	vers->arch_version = 0x6F56;
	return QURT_EOK;
}

/**@ingroup func_qurt_sysenv_get_max_hw_threads
  Gets the number of hardware threads supported in the Hexagon processor from the kernel.
 
  @datatypes
  #qurt_sysenv_max_hthreads_t

  @param[out] mhwt  Pointer to the number of hardware threads supported in the Hexagon processor.

  @return
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_max_hw_threads(qurt_sysenv_max_hthreads_t *mhwt )
{
	mhwt->max_hthreads = 4;
	return QURT_EOK;
}

/**@ingroup func_qurt_sysenv_get_max_pi_prio
  Gets the maximum priority inheritance mutex priority from the kernel.
  
  @datatypes
  #qurt_sysenv_max_pi_prio_t

  @param[out] mpip  Pointer to the maximum priority inheritance mutex priority.

  @return            
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_max_pi_prio(qurt_sysenv_max_pi_prio_t *mpip )
{
	mpip->max_pi_prio = 8;
	return QURT_EOK;
}

/**@ingroup func_qurt_sysenv_get_process_name
  Gets information on the system environment process names from the kernel.
  
  @datatypes
  #qurt_sysenv_procname_t

  @param[out] pname  Pointer to information on the process names in the system.

  @return
  QURT_EOK -- Success. \n
  QURT_EVAL -- Invalid parameter.

  @dependencies
  None.
*/
static inline int qurt_sysenv_get_process_name(qurt_sysenv_procname_t *pname )
{
	strcpy(pname->name,"PROC");
	pname->asid = 0;
	return QURT_EOK;
}

/**@ingroup func_qurt_exception_wait  
  Registers the program exception handler.
  This function assigns the current thread as the QuRT program exception handler and suspends the
  thread until a program exception occurs.

  When a program exception occurs, the thread is awakened with error information
  assigned to this operations parameters.

  @note1hang If no program exception handler is registered, or if the registered handler
             itself calls exit, then QuRT raises a kernel exception.
             If a thread runs in Supervisor mode, any errors are treated as kernel
             exceptions.

  @param[out]  ip      Pointer to the instruction memory address where the exception occurred.
  @param[out]  sp      Stack pointer.
  @param[out]  badva   Pointer to the virtual data address where the exception occurred.
  @param[out]  cause   Pointer to the QuRT error result code.   

  @return
  Registry status: \n
  - Thread identifier -- Handler successfully registered. \n
  - QURT_EFATAL -- Registration failed.

  @dependencies
  None.
*/
static inline unsigned int qurt_exception_wait (unsigned int *ip, unsigned int *sp,
                                  unsigned int *badva, unsigned int *cause)
{
	/* EJP: do not support exception events this way, so wait forever */
	h2_sem_t sem;
	h2_sem_init_val(&sem,0);
	while (1) h2_sem_down(&sem);
	return QURT_EFATAL;
}

unsigned int  qurt_exception_wait_ext (qurt_sysevent_error_t * sys_err); // should be inlined away

/**@ingroup func_qurt_exception_wait2
  Registers the current thread as the QuRT program exception handler, and suspends the thread until a
  program exception occurs.

  When a program exception occurs, the thread is awakened with error information assigned to the specified
  error event record.
 
  If a program exception is raised when no handler is registered (or when a handler is registered, but it calls
  exit), the exception is treated as fatal.\n
  @note1hang If a thread runs in monitor mode, all exceptions are treated as kernel exceptions.\n
  @note1cont This function differs from qurt_exception_wait() by returning the error information in a data
              structure rather than as individual variables. It also returns additional information (i.e., ssr).
  @datatypes
  #qurt_sysevent_error_t

  @param[out] sys_err  Pointer to the error event record. Values:
                       - ip -- Instruction memory address where the exception occurred. \n
                       - sp -- Stack pointer. \n
                       - badva -- Virtual data address where the exception occurred. \n
                       - cause -- QuRT error result code (Section @xref{dox:error_results}). \n
                       - ssr -- Supervisor status register. @tablebulletend

  @return
  Registry status: \n
  QURT_EFATAL -- Failure. \n
  Thread ID -- Success.

  @dependencies
  None.
*/

static inline unsigned int qurt_exception_wait2(qurt_sysevent_error_t * sys_err)
{
	/* EJP: do not support exception events this way, so wait forever */
	h2_sem_t sem;
	h2_sem_init_val(&sem,0);
	while (1) h2_sem_down(&sem);
	return QURT_EFATAL;
}

/**@ingroup func_qurt_exception_raise_nonfatal
  Raises a nonfatal program exception in the QuRT program system.

  For more information on program exceptions see Section @xref{dox:exception_handling}.

  This operation never returns -- the program exception handler is assumed to perform all
  exception handling before terminating or reloading the QuRT program system.

  @note1hang The C library function abort() calls this operation to indicate software
             errors.

  @param[in] error QuRT error result code (Section @xref{dox:error_results}).

  @return
  Integer -- Unused.

  @dependencies
  None.
*/
// static inline int qurt_exception_raise_nonfatal (int error) __attribute__((noreturn)) { R_UNSUPPORTED; }
static inline int qurt_exception_raise_nonfatal (int error) { R_UNSUPPORTED; }

/**@ingroup func_qurt_exception_raise_fatal
  Raises a fatal program exception in the QuRT system.

  Fatal program exceptions terminate the execution of the QuRT system without invoking 
  the program exception handler.

  For more information on fatal program exceptions see Section @xref{dox:exception_handling}.

  This operation always returns, so the calling program can perform the necessary shutdown 
  operations (data logging, etc.).

  @note1hang Context switches do not work after this operation has been called.
 
  @return 
  None.
 
  @dependencies
  None.
*/
void qurt_exception_raise_fatal (void);

/**@ingroup func_qurt_exception_shutdown_fatal 
  Performs the fatal shutdown procedure for handling a fatal program exception.

  For more information on the fatal shutdown procedure see Section @xref{dox:exception_handling}.
 
  @note1hang This operation does not return, as it shuts down the system.

  @return 
  None.
 
  @dependencies
  None.
*/
// static inline void qurt_exception_shutdown_fatal(void) __attribute__((noreturn)) { R_UNSUPPORTED; }
static inline void qurt_exception_shutdown_fatal(void) { UNSUPPORTED; }

/**@ingroup func_qurt_exception_shutdown_fatal2 
  Performs the fatal shutdown procedure for handling a fatal program exception.
  This operation always returns, so the calling program can complete the fatal shutdown procedure.
  For more information on the fatal shutdown procedure see Section @xref{dox:exception_handling}.

  @note1hang This function differs from qurt_exception_shutdown_fatal() by always returning to the caller.

  @return 
  None.
 
  @dependencies
  None.
*/
void qurt_exception_shutdown_fatal2(void);

/**@ingroup func_qurt_exception_register_fatal_notification
  Registers a fatal exception notification handler with the RTOS kernel.

  The handler function is intended to perform the final steps of system 
  shutdown after all the other shutdown actions have been performed (e.g., 
  notifying the host processor of the shutdown). It should perform only a
  minimal amount of execution.\n
  @note1hang The fatal notification handler executes on the Hexagon processor in user mode. 
 
  @param[in] entryfuncpoint    Pointer to the handler function.
  @param[in] argp   Pointer to the argument list passed to handler function when it
                    is invoked.

  @return
  Registry status: \n
  QURT_EOK -- Success \n
  QURT_EVAL -- Failure; invalid parameter  

  @dependencies
  None.
*/
static inline unsigned int qurt_exception_register_fatal_notification ( void(*entryfuncpoint)(void *), void *argp)
{
	R_UNSUPPORTED;
}

/**@ingroup func_qurt_exception_enable_fp_exceptions
  Enables the specified floating point exceptions as QuRT program exceptions.

  The exceptions are enabled by setting the corresponding bits in the Hexagon 
  control register USR.

  The mask argument specifies a mask value identifying the individual floating 
  point exceptions to be set. The exceptions are represented as defined symbols 
  which map into bits 0-31 of the 32-bit flag value.
  Multiple floating point exceptions are specified by OR'ing together the individual 
  exception symbols.\n
  @note1hang This function must be called before any floating point operations are performed.
 
  @param mask Floating point exception types. Values: \n
             - QURT_FP_EXCEPTION_ALL    \n    
             - QURT_FP_EXCEPTION_INEXACT    \n
             - QURT_FP_EXCEPTION_UNDERFLOW  \n
             - QURT_FP_EXCEPTION_OVERFLOW  \n
             - QURT_FP_EXCEPTION_DIVIDE0    \n
             - QURT_FP_EXCEPTION_INVALID   @tablebulletend  

  @return 
  Updated contents of the USR register.

  @dependencies
  None.
*/

static inline unsigned int qurt_enable_floating_point_exception(unsigned int mask) { return 0; }

static inline unsigned int qurt_exception_enable_fp_exceptions(unsigned int mask) { return 0; }

/**@ingroup func_qurt_exception_wait_pagefault
  Registers the page fault handler.
  This function assigns the current thread as the QuRT page fault handler and
  suspends the thread until a page fault occurs.

  When a page fault occurs, the thread is awakened with page fault information
  assigned to the parameters of this operation.

  @param[out] sys_pagefault   Pointer to the page fault event record, the instruction 
                              memory address where the exception occurred.
 
  @return
  Registry status: \n
  QURT_EOK -- Success. \n
  QURT_EFAILED -- Failure due to existing pager registration. \n

  @dependencies
  None.
*/
 
static inline unsigned int qurt_exception_wait_pagefault (qurt_sysevent_pagefault_t *sys_pagefault)
{
	/* EJP: do not support pagefault events this way, so wait forever */
	h2_sem_t sem;
	h2_sem_init_val(&sem,0);
	while (1) h2_sem_down(&sem);
	return QURT_EFATAL;
}

#endif /* QURT_EVENT_H */

