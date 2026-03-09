/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/*
 * Picolibc Locking Implementation for H2
 *
 * This file provides the picolibc retargetable locking API implementation
 * using H2's pthread mutex primitives. All functions are marked weak so
 * applications can override them if needed.
 */

#include <pthread.h>
#include <stdatomic.h>

/*
 * Lock structure - wraps a pthread_mutex_t
 * This must match the opaque struct __lock expected by picolibc
 */
struct __lock {
  pthread_mutex_t mut;
};

/*
 * Define the lock pointer type as expected by picolibc
 */
typedef struct __lock *_LOCK_T;

/*
 * Global recursive mutex for libc
 * This is used by malloc, atexit, arc4random, getenv, timezone functions, etc.
 * Must be statically initialized.
 */
struct __lock __lock___libc_recursive_mutex = {
    .mut = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP};

/*
 * Pool of dynamic locks for stdio and other uses
 * Using atomics for thread-safe allocation without needing initialization
 */
#define MAX_LOCKS 64

static struct __lock locks[MAX_LOCKS];
static _Atomic int in_use[MAX_LOCKS];

/*
 * Initialize a new dynamic non-recursive lock
 *
 * Note: Despite the name, we create recursive mutexes for all locks
 * to ensure compatibility with picolibc's usage patterns.
 */
__attribute__((weak)) void __retarget_lock_init(_LOCK_T *lock) {
  int lock_id;

  /* Search for an available lock slot */
  for (lock_id = 0; lock_id < MAX_LOCKS; lock_id++) {
    int expected = 0;
    int desired = 1;

    /* Atomically claim this slot if it's free */
    if (atomic_compare_exchange_strong(&in_use[lock_id], &expected, desired)) {
      pthread_mutexattr_t mutexattr;

      /* Initialize mutex attributes for recursive mutex */
      pthread_mutexattr_init(&mutexattr);
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

      /* Initialize the mutex */
      pthread_mutex_init(&locks[lock_id].mut, &mutexattr);

      /* Return pointer to the lock */
      *lock = &locks[lock_id];
      return;
    }
  }

  /* Out of lock slots - return NULL */
  *lock = NULL;
}

/*
 * Initialize a new dynamic recursive lock
 */
__attribute__((weak)) void __retarget_lock_init_recursive(_LOCK_T *lock) {
  /* All our locks are recursive, so just call the regular init */
  __retarget_lock_init(lock);
}

/*
 * Close and deallocate a dynamic non-recursive lock
 */
__attribute__((weak)) void __retarget_lock_close(_LOCK_T lock) {
  if (lock == NULL)
    return;

  /* Find the lock in our pool */
  int lock_id = lock - locks;

  /* Validate the lock_id is in range */
  if (lock_id < 0 || lock_id >= MAX_LOCKS)
    return;

  /* Destroy the mutex */
  pthread_mutex_destroy(&locks[lock_id].mut);

  /* Mark the slot as free */
  int desired = 0;
  atomic_exchange(&in_use[lock_id], desired);
}

/*
 * Close and deallocate a dynamic recursive lock
 */
__attribute__((weak)) void __retarget_lock_close_recursive(_LOCK_T lock) {
  __retarget_lock_close(lock);
}

/*
 * Acquire a non-recursive lock
 */
__attribute__((weak)) void __retarget_lock_acquire(_LOCK_T lock) {
  if (lock == NULL)
    return;

  pthread_mutex_lock(&lock->mut);
}

/*
 * Acquire a recursive lock
 */
__attribute__((weak)) void __retarget_lock_acquire_recursive(_LOCK_T lock) {
  if (lock == NULL)
    return;

  pthread_mutex_lock(&lock->mut);
}

/*
 * Release a non-recursive lock
 */
__attribute__((weak)) void __retarget_lock_release(_LOCK_T lock) {
  if (lock == NULL)
    return;

  pthread_mutex_unlock(&lock->mut);
}

/*
 * Release a recursive lock
 */
__attribute__((weak)) void __retarget_lock_release_recursive(_LOCK_T lock) {
  if (lock == NULL)
    return;

  pthread_mutex_unlock(&lock->mut);
}
