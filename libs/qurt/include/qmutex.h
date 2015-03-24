/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QMUTEX_H
#define QMUTEX_H

/* EJP: DONE: TURN INTO qurt_pimutex */

#include <stddef.h>
#include <qurt_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <qurt.h>

/**
 * Mutex type
 *
 * Local mutex is only accessable within a PD and shared mutex
 * can be used across PD 
 */ 
typedef enum {
    QMUTEX_LOCAL=0,
    QMUTEX_SHARED,
    QMUTEX_PRIORITY
} qmutex_type_t;

typedef struct {
 qmutex_type_t type;
} qmutex_attr_t;

typedef struct
{
  qmutex_type_t type;
  qurt_mutex_t qurt_mutex;
} qmutex_struct;

typedef qmutex_struct *qmutex_t;

/**
 * Init qmutex attribute object, setting QMUTX_LOCAL as default type 
 *
 * @param attr         qmutex attribute pointer 
 */
static inline void qmutex_attr_init(qmutex_attr_t *attr) 
{ attr->type = QMUTEX_LOCAL; }

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
 *
 * @return             EOK:     Creation is successful
 * @return             EVAL:    Mutex is not the right type QMUTEX_LOCAL/QMUTEX_SHARED
 * @return             EMEM:    Out of memory
 *
 * EJP: &*W@$$#@&( out variable?  OK, guess we'll malloc
 *
 */
static inline int qmutex_create(qmutex_t *mutex, qmutex_attr_t *attr) {
	if ((*mutex = qurt_malloc(sizeof(qmutex_struct))) == NULL) {
		return QURT_EMEM;
	}
	*mutex->type = (attr == NULL) ? QMUTEX_LOCAL : attr->type;
	qurt_pimutex_init(&(*mutex)->qurt_mutex);
	return QURT_EOK;
}

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
 * @param mutex  Mutex object
 *
 * @return       EOK:      Deletion is successfully
 * @return       EINVALID: Mutex is not a valid handle
 */
static inline int qmutex_delete(qmutex_t mutex)
{
	qurt_pimutex_destroy(&(mutex->qurt_mutex));
	qurt_free(mutex);
	return QURT_EOK;
}

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
static inline int qmutex_lock(qmutex_t mutex)
{
	qurt_pimutex_lock(&mutex->qurt_mutex);
	return QURT_EOK;
}

/**
 * Try to lock a mutex
 * 
 * The qthread_try_lock is used to try to acquire a mutex lock. If mutex is available,
 * it is acquired. Otherwise the calls returns immediately. 
 *
 * @param mutex  Mutex object
 *
 * @return       EOK:       Mutex is locked successfully.
 * @return       EFAILED:   Mutex is not available.
 * @return       EINVALID:  Mutex is not a valid handle
 */
static inline int qmutex_trylock(qmutex_t mutex)
{
	return (qurt_pimutex_try_lock(&mutex->qurt_mutex) == 0) ? EOK : EFAILED;
}

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
static inline int qmutex_unlock(qmutex_t mutex)
{
	qurt_pimutex_unlock(&mutex->qurt_mutex);
	return EOK;
}

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* QMUTEX_H */
