/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QTHREAD_H
#define H2_QTHREAD_H

#include <stddef.h>
#include <string.h>
#include <qtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int qthread_t;
typedef struct qthread_attr_t
{
    size_t  stacksize;
    unsigned int  stackaddr;
    int     priority;
    int     tid;
    char    name[QTHREAD_NAME_LEN];
    unsigned int hw_bitmask;
} qthread_attr_t;

/*****************************************************************/
/* FILE: qthread.h                                               */
/*                                                               */
/* SERVICES: qthread API                                         */
/*                                                               */
/* DESCRIPTION: Prototypes of qthread API                        */
/*****************************************************************/

/************************ COPYRIGHT NOTICE ***********************/

/* All data and information contained in or disclosed by this    */
/* document is confidential and proprietary information of       */
/* QUALCOMM, Inc and all rights therein are expressly reserved.  */
/* By accepting this material the recipient agrees that this     */
/* material and the information contained therein is held in     */
/* confidence and in trust and will not be used, copied,         */
/* reproduced in whole or in part, nor its contents revealed in  */
/* any manner to others without the express written permission   */
/* of QUALCOMM, Inc.                                             */
/*****************************************************************/

/**
 * Creates a thread with specific attributes
 *
 * The qthread_create is used to create a thread in suspended state
 * with attribtues specified by attr. If attr is NULL, then default
 * values are used. The attributes that can be specified are thread
 * name, stack size, stack address and priority. If stack address is
 * non-zero, it will used as a stack for the thread. Otherwise the
 * value of stack size is used to dynamically allocate stack.
 *
 * @param thr   [OUT] Thread identifier
 * @param attr        Thread attributes
 * @return            EOK if thread creation is successful, 
 *                    EVAL if thread object is NULL or invalid values
 *                     are specified for attributes,
 *                    EMEM if memory allocation for stack fails,
 *                    EUNKNOWN if misc. error occurs
 */
int qthread_create(qthread_t *thr, qthread_attr_t *attr);

/**
 * Initialize thread attributes. The attributes stack size and priority
 * are set to their default values (QTHREAD_MIN_STACKSIZE and QTHREAD_
 * MIN_PRIORITY)
 *
 * @param attr  [OUT] Thread attributes object
 */
    static inline void qthread_attr_init(qthread_attr_t *attr);
static inline void qthread_attr_init(qthread_attr_t *attr)
{
    /* Set default values for all attributes */
    memset(attr->name, '\0', QTHREAD_NAME_LEN);
    attr->stacksize = QTHREAD_MIN_STACKSIZE;
    attr->stackaddr = 0;
    attr->priority  = QTHREAD_MIN_PRIORITY;
    attr->tid  = -2; // Default TID
    attr->hw_bitmask = 0xff;  //BLAST_THREAD_CFG_BITMASK_ALL 
}

/**
 * Set stack size of a thread
 *
 * @param attr  Thread attributes object
 * @param size  Size of the stack. The value should be between
 *              QTHREAD_MIN_STACKSIZE and QTHREAD_MAX_STACKSIZE
 */
static inline void qthread_attr_sethwbitmask(qthread_attr_t *attr, unsigned int hw_bitmask)
{
    /* store stack size into attributes object */
    attr->hw_bitmask = hw_bitmask;
}
/**
 * Set name of a thread. The name is used only for debugging purpose. The name
 * should be null terminated and must not exceed QTHREAD_NAME_LEN characters (
 * including null character). Otherwise it will be truncated.
 *
 * @param attr  Thread attributes object
 * @param name  Name of the thread. The maximum length is limited to
 *              QTHREAD_NAME_LEN
 */
static inline void qthread_attr_setname(qthread_attr_t *attr, const char *name)
{
    /* copy first QTHREAD_NAME_LEN-1 characters */
    strncpy(attr->name, name, QTHREAD_NAME_LEN-1);
}

/**
 * Get name of a thread
 *
 * @param attr  Thread attributes object
 * @param name  Location where name is copied
 * @param size  Size of buffer where name is copied. If size is less than length
 *              of the name, it will be truncated
 */
static inline void qthread_attr_getname(const qthread_attr_t *attr, char *name, int size)
{
    /* copy name string */
    name[size-1] = '\0';
    strncpy(name, (char *)attr->name, size-1);
}

/**
 * Set stack size of a thread
 *
 * @param attr  Thread attributes object
 * @param size  Size of the stack. The value should be between
 *              QTHREAD_MIN_STACKSIZE and QTHREAD_MAX_STACKSIZE
 */
static inline void qthread_attr_setstacksize(qthread_attr_t *attr, size_t size)
{
    /* store stack size into attributes object */
    attr->stacksize = size;
}

/**
 * Get stack size of a thread
 *
 * @param attr  Thread attributes object
 * @param size  Location where stack size is stored
 */
static inline void qthread_attr_getstacksize(qthread_attr_t *attr, size_t *size)
{
    /* copy stack size */
    *size = attr->stacksize;
}

/**
 * Set stack address of a thread. This elimates the need for dynamically
 * allocating stakc when qthread_create is invoked
 *
 * @param attr  Thread attributes object
 * @param addr  Address of stack
 */
static inline void qthread_attr_setstackaddr(qthread_attr_t *attr, unsigned int addr)
{
    /* store stack address into attributes object */
    attr->stackaddr = addr;
}

/**
 * Get stack address
 *
 * @param attr  Thread attributes object
 * @param addr  Address of the stack
 */
static inline void qthread_attr_getstackaddr(qthread_attr_t *attr, unsigned int *addr)
{
    /* copy stack address */
    *addr = attr->stackaddr;
}

/**
 * Set priority of a thread. The valid range for priority is
 * QTHREAD_MIN_PRIO to QTHREAD_MAX_PRIO
 *
 * @param attr      Thread attributes object
 * @param priority  Thread Priority. The valid range for priority is
 *                  QTHREAD_MIN_PRIO and QTHREAD_MAX_PRIO
 */
static inline void qthread_attr_setpriority(qthread_attr_t *attr, int priority)
{
    /* store priority into attributes object */
    attr->priority = priority;
}

/**
 * Get priority
 *
 * @param attr      Thread attributes object
 * @param priority  Location where priority is stored
 */
static inline void qthread_attr_getpriority(qthread_attr_t *attr, int *priority)
{
    /* copy priority */
    *priority = attr->priority;
}

/**
 * get tid
 *
 * @param attr      Thread attributes object
 * @param tid       location where tid is stored
 */
static inline void qthread_attr_gettid(qthread_attr_t *attr, unsigned int *tid)
{
    /* copy tid */
    *tid = attr->tid;
}

/**
 * Set tid of a thread. The valid range for tid is 0-255
 *
 * @param attr      Thread attributes object
 * @param tid       Thread tid. 
 */
static inline void qthread_attr_settid(qthread_attr_t *attr, unsigned int tid)
{
    /* store tid into attributes object */
    attr->tid = tid;
}

/**
 * Get attributes of a thread
 *
 * @param thr         Thread identifier
 * @param attr  [OUT] Thread attributes
 * @return            EOK if successful,
 *                    EINVALID if thr is not a valid handle
 *                    EVAL if attr is NULL
 */
int qthread_get_attr(qthread_t thr, qthread_attr_t *attr);

/**
 * Starts a thread
 *
 * @param thr         Thread identifier
 * @param start_func  Startup function where thread starts executing
 * @param arg         Argument to startup function
 * @return            EOK if successful,
 *                    EVAL if starting function address is NULL
 *                    EINVALID if thr is invalid
 */
int qthread_start(qthread_t thr, void (*start_func) (void *), void *arg);

/**
 * Deletes a thread
 *
 * The qthread_delete is used to forcibly terminates thread specified by thr.
 *
 * @param thr   Thread identifier
 * @return      EOK thread is deleted successfully,
 *              EINVALID if thr is invalid
 */
static inline int qthread_delete(qthread_t thr) { return EOK; }

/**
 * Exit from a thread. This function should be in the context of the thread
 * that should exit.
 *
 * @param status  Exit status of the thread. This is available to other threads
 *                within the same PD until another thread joins the thread
 */
void qthread_exit(int status);

/**
 * Waits for a thread to finish
 *
 * When qthread_join is called, the caller thread is blocked until the thread
 * thr exits either by calling qthread_exit or by being terminated (using
 * qthread_delete).  If thread thr has already exited, then the call returns
 * immediately with status set to EINVALID
 *
 * @param thr     Thread identifier
 * @param status  Location where exit status of the thread is stored. The status
 *                is set to QTHREAD_STATUS_DELETED if the thread is forcibly
 *                terminated.
 * @return        EOK thread is joined successfully,
 *                EINVALID if thr is invalid,
 *                EVAL if status is NULL or another thread is already waiting
 *                 for thr to exit
 */
int  qthread_join(qthread_t thr, int *status);

//void qthread_set_name(unsigned long long name0, unsigned long long name1);
#define qthread_set_name blast_thread_set_name

/**
 * Get current thread's identifier
 *
 * @return  Current thread's identifier
 */
qthread_t qthread_myself(void);

/**
 * Sleep till no other thread is ready to run in the system. It gets schedule
 * when it becomes the only thread runnable. The other threads are either
 * waiting on a resource or waiting on timer.
 *
 * @return             void
 */
void qthread_wait_for_idle (void);

/**
 * Sleep till any other thread is ready to run in the system. The calling threads
 * gets immediately scheduled if any of threads is already running. If no other
 * thread runs when the API is called, the thread goes to sleep and wake up with
 * an interrupt.
 *
 * @return             void
 */
void qthread_wait_for_active (void);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
