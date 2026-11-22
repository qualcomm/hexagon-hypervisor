
:mod:`futex_pi` -- priority inheritance mutex Services
======================================================

H2K_futex_lock_pi
-----------------

.. c:function:: u32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me)

	:param lock: A user-specified pointer to a word in memory containing the mutex
	:param me: pointer to the current thread

Description
~~~~~~~~~~~

This function implements a priority-inheritance simple mutex lock on the kernel side.

If the lock is held by a thread, that thread will be boosted to the current thread priority,
if the current thread is a better priority.

Returns 0 on success, nonzero on failure.  Per the VMWORK contract
(see :doc:`../../util/vmdefs/vmdefs`), -1 may also be returned
spuriously when VMWORK is observed after the BKL is acquired; the
caller must re-check the lock state and retry.

Functionality
~~~~~~~~~~~~~

Modifications outside the kernel are only allowed using atomic primitives, and only 
if the modification is from 0 (free, no waiters) to TID, or from TID to 0 (for unlock).

If the word pointed to by lock is zero, the lock was freed.  We set it to me and return.

If the word pointed to by lock is nonzero, we attempt to elevate the priority of the 
holding thread.  We set the least significant bit of the lock.

H2K_futex_unlock_pi
-------------------

.. c:function:: s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me)

	:param lock: A user-specified pointer to a word in memory containing the mutex
	:param me: pointer to the current thread

Description
~~~~~~~~~~~

This function implements a priority-inheritance simple mutex unlock on the kernel side.

The mutex will be handed off to the highest priority waiting thread, or cleared if 
no thread is waiting.

Returns 0 on success, nonzero on failure.

Functionality
~~~~~~~~~~~~~

Modifications outside the kernel are only allowed using atomic primitives, and only
if the modification is from 0 (free, no waiters) to TID, or from TID to 0 (for unlock).

We translate "lock" to a physical address and look in the corresponding futex
hash bucket for a thread blocked on that address, starting from the head of
the bucket's ring via :c:func:`H2K_futex_hash_remove_one()`.  Buckets are kept
sorted by priority: :c:func:`H2K_futex_lock_pi()` inserts each blocked thread
in priority order, and :c:func:`H2K_futex_pi_raise()` re-sorts a thread's
position if its priority is boosted while blocked.  Because of this
invariant, the first matching thread found is the highest-priority thread
waiting on this particular lock; :c:func:`H2K_futex_hash_remove_one()` removes
it from the ring and places it in the ready queue.

If no matching thread is found, we set the lock to zero.  Otherwise, we set
the lock to the woken thread's ID and set the LSB to indicate that there may
be additional threads waiting.

Finally, we restore the calling thread's own priority to its base priority,
since it may have been boosted by a waiter while it held the lock.


Testing
-------


