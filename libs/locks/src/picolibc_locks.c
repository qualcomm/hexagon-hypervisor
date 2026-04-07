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

#include <h2_atomic.h>
#include <hexagon_protos.h>
#include <pthread.h>

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
 * Using a 64-bit bitmask for thread-safe allocation without needing
 * initialization Each bit represents one lock slot: 0 = free, 1 = in use
 */
#define MAX_LOCKS 64

static struct __lock locks[MAX_LOCKS];
static atomic_u64_t in_use = 0;

/*
 * Initialize a new dynamic non-recursive lock
 *
 * Note: Despite the name, we create recursive mutexes for all locks
 * to ensure compatibility with picolibc's usage patterns.
 */
__attribute__((weak)) void __retarget_lock_init(_LOCK_T *lock) {
  int lock_id;
  unsigned long long old_in_use;
  unsigned long long new_in_use;

  /* Use Hexagon-optimized bit operations to find and claim a free slot */
  do {
    old_in_use = in_use;

    /* Find the first 0 bit (free slot) using Hexagon intrinsic */
    lock_id = Q6_R_ct0_R(old_in_use);

    /* Check if we found a valid slot (ct0 returns 64 if all bits are 1) */
    if (lock_id >= MAX_LOCKS) {
      /* Out of lock slots - return NULL */
      *lock = NULL;
      return;
    }

    /* Set the bit to claim this slot */
    new_in_use = old_in_use | (1ULL << lock_id);

    /* Atomically claim the slot using Hexagon-optimized compare-swap */
  } while (h2_atomic_compare_swap64(&in_use, old_in_use, new_in_use) !=
           old_in_use);

  /* Successfully claimed the slot, now initialize the mutex */
  pthread_mutexattr_t mutexattr;

  /* Initialize mutex attributes for recursive mutex */
  pthread_mutexattr_init(&mutexattr);
  pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

  /* Initialize the mutex */
  pthread_mutex_init(&locks[lock_id].mut, &mutexattr);

  /* Return pointer to the lock */
  *lock = &locks[lock_id];
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

  /* Mark the slot as free using Hexagon-optimized atomic clear bit */
  h2_atomic_clrbit64(&in_use, lock_id);
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
