/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QMUTEX_H
#define H2_QMUTEX_H

#include <stddef.h>
#include <qerror.h>
#include <qtypes.h>

/* XXX Copied this from BLAST 2.0.7 */

#ifdef __cplusplus
extern "C" {
#endif

#include <blast.h>

//  if qmutex_create is calling blast_rmutex_init, then that means qmutex_t must be blast_rmutex_t.
typedef blast_rmutex_t *qmutex_t;

/*****************************************************************/
/* FILE: qmutex.h                                                */
/*                                                               */
/* SERVICES: qmutex API                                          */
/*                                                               */
/* DESCRIPTION: Prototypes of qmutex API                         */
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
 * Init qmutex attribute object, setting QMUTX_LOCAL as default type 
 *
 * @param attr         qmutex attribute pointer 
 */
static inline void qmutex_attr_init(qmutex_attr_t *attr) { attr->type = QMUTEX_LOCAL; }

/**
 * Set mutex type 
 *
 * @param attr         qmutex attribute pointer 
 * @param type         mutex type (QMUTEX_LOCAL/QMUTEX_SHARED) 
 */
static inline void qmutex_attr_settype(qmutex_attr_t *attr, qmutex_type_t type)
{
    attr->type = type;
}
/**
 * Creates a mutex with specific attributes
 *
 * The qthread_create is used to create a mutex that is either local
 * to a process or shared across processes
 *
 * @param mutex  [OUT] Address of Mutex object. mutex will be initialized 
 *                     after the call 
 * @param attr         Specifies whether mutex is local to caller pd or
 *                     shared across processe. The current values are
 *                     QMUTEX_LOCAL or QMUTEX_SHARED
 * @return             EOK:     Creation is successful
 * @return             EVAL:    Mutex is not the right type QMUTEX_LOCAL/QMUTEX_SHARED
 * @return             EMEM:    Out of memory
 */
// int qmutex_create(qmutex_t *mutex, qmutex_attr_t attr);
static inline int qmutex_create(qmutex_t *mutex, qmutex_attr_t *attr __attribute__((unused))) 
{ if ((*mutex =(blast_rmutex_t *) blast_malloc(sizeof(blast_rmutex_t))) == NULL) {
    return EMEM;
  }
  blast_rmutex_init(*mutex);
  return EOK; 
}

///**
// * Initialize attribute object with default values.
// *
// * The default values are QMUTEX_MEM_SHARED for mutex type 
// * and null for mem_start and mem_size 
// *
// * @param attr  Attributes object
// */
//void qmutex_attr_init(qmutex_attr_t* attr);
//
///**
// * Set memory start and memory size for MEM_SHARED mutex
// *
// * @param attr  Attribute object
// * @param ch    Channel object
// */
//void qmutex_attr_setmemory(qmutex_attr_t* attr, void* buffer, size_t size);

/**
 * Deletes a mutex
 *
 * The qmutex_delete is used to delete a mutex specified by mutex_obj.
 * 
 * Note:  if qmutex_delete is called when someone is in the middle of
 * qmutex_lock/unlock/trylock on the same handle, the behaviour will be
 * unpredictable.  Users should guarantee nobody is using the lock before
 * deleting the mutex 
 *
 *
 * @param mutex  Mutex object
 * @return       EOK:      Deletion is successfully
 * @return       EINVALID: Mutex is not a valid handle
 */
//int qmutex_delete(qmutex_t mutex);

static inline int qmutex_delete(qmutex_t mutex) {blast_rmutex_destroy(((blast_rmutex_t *)mutex)); blast_free(mutex); return EOK; }

/**
 * Lock mutex
 *
 * The qthread_lock is used to acquire a mutex lock. The lock can be held
 * by only one thread at any point of time. If a thread tries to lock when
 * mutex is not available, then it is put to a wait state and added to a
 * queue where it will wait for its turn to acquire the mutex. The queue
 * is sorted based on thread priority to ensure that highest priority
 * waiting thread is given mutex when it is unlocked.
 *
 * @param mutex  Mutex object
 * @return       EOK:        Mutex is locked successfully
 * @return       EINVALID:   Mutex is not a valid handle
 */
// int qmutex_lock(qmutex_t mutex);

static inline int qmutex_lock(qmutex_t lock) { blast_rmutex_lock(lock); return EOK; }

/**
 * Try to lock a mutex
 * 
 * The qthread_trylock is used to try to acquire a mutex lock. If mutex is available,
 * it is acquired. Otherwise the calls returns immediately. 
 *
 * @param mutex  Mutex object
 * @return       EOK:       Mutex is locked successfully.
 * @return       EFAILED:   Mutex is not available.
 * @return       EINVALID:  Mutex is not a valid handle
 */
// int qmutex_trylock(qmutex_t mutex);
static inline int qmutex_trylock(qmutex_t lock) { return (blast_rmutex_trylock(lock) == 0) ? EOK:EFAILED;}

/**
 * Unlock mutex
 * 
 * The qthread_unlock is used to release a mutex lock. If one or more threads
 * are waiting, the thread with highest priority among waiting threads will
 * get the mutex.
 *
 * @param mutex  Mutex object
 * @return       EOK if mutex is unlocked successfully
 * @return       EINVALID is mutex is not a valid handle
 */
// int qmutex_unlock(qmutex_t mutex);

static inline int qmutex_unlock(qmutex_t lock) { blast_rmutex_unlock(lock); return EOK;}

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif
