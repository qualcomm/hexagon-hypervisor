
:mod:`futex_pi` -- priority inheritance mutex Services
======================================================

H2K_futex_lock_pi
-----------------

.. cfunction:: u32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me)

	:param lock: A user-specified pointer to a word in memory containing the mutex
	:param me: pointer to the current thread

Description
~~~~~~~~~~~

This function implements a priority-inheritance simple mutex lock on the kernel side.

If the lock is held by a thread, that thread will be boosted to the current thread priority,
if the current thread is a better priority.

Returns 0 on success, nonzero on failure.

Functionality
~~~~~~~~~~~~~

Modifications outside the kernel are only allowed using atomic primitives, and only 
if the modification is from 0 (free, no waiters) to TID, or from TID to 0 (for unlock).

If the word pointed to by lock is zero, the lock was freed.  We set it to me and return.

If the word pointed to by lock is nonzero, we attempt to elevate the priority of the 
holding thread.  We set the least significant bit of the lock.

H2K_futex_unlock_pi
-------------------

.. cfunction:: s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me)

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

We pick the highest priority waiting thread.  If no threads are waiting, we set
the lock to zero.  Otherwise, we place the woken thread in the ready queue.  We
then set the lock to that thread ID and set the LSB to indicate that there may
be additional threads waiting.


Testing
-------


