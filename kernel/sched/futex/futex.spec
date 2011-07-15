
:mod:`futex` -- Generic Blocking / Unblocking Services
======================================================

.. module:: futex

H2K_futex_wait
--------------

.. cfunction:: s32_t H2K_futex_wait(u32_t *ptr, u32_t expected, H2K_thread_context *me)

	:param ptr: a user-specified pointer to a word in memory
	:param expected: the expected value for `*ptr`
	:param me: pointer to the current thread.
	:returns: 0 if the thread blocked, -1 otherwise.

Description
~~~~~~~~~~~

:cfunc:`H2K_futex_wait()` asks the kernel to block, but only if the value pointed to by "ptr" is equal
to "expected".  

If the value has changed, the kernel returns -1.

If the thread went to sleep, the kernel returns 0.

Functionality
~~~~~~~~~~~~~

First, the hash key is computed.  The hash key is based on the address used
by the kernel.  This should either be based only on the lowest bits of the
address, that are unchanged by virtual to physical translation, or should be
based on the physical address.  Otherwise, futexes shared between two processes
with different address spaces may not function correctly.

The hash key is computed outside the lock, because the hash key is
independent of any value that may be changed by the kernel, and we always wish
to hold the lock as little as possible.  

Next, we lock the BKL.

After locking the BKL, we can check the value at the pointer.  Care must be
taken while checking the value pointed to, as we cannot trust that the user
pointer is correct.

One mechanism for safely checking the pointer follows:

* Lock the TLB
* Probe the TLB for a translation
* If the translation exists, check for Read permissions
* If the translation exists and has read permissions, read the value
* Unlock the TLB

If the value was unreadable or does not match the expected value, unlock the
BKL and return -1.

Otherwise, we remove the current thread from the list of running threads, set
the `r0100` field in the context to 0 (which will be the return value), update
the `status` field to `H2K_STATUS_BLOCKED`, and add it to the futex hash table
using the hash key.  We then call :cfunc:`H2K_dosched()` for a new thread to be
scheduled.  

H2K_futex_resume
----------------

.. cfunction:: u32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)

	:param lock: a user-specified value
	:param n_to_wake: a user-specified maximum number of threads to wake
	:param me: pointer to the current thread.
	:returns: the number of threads woken

Description
~~~~~~~~~~~

:cfunc:`H2K_futex_resume()` wakes threads waiting on the location specified by "lock".  A maximum of 
"n_to_wake" threads are awoken.  

The kernel returns the number of woken threads.

Functionality
~~~~~~~~~~~~~

If the number of threads to wake is zero, there is nothing to be done.  We return 0.
This can happen if n_to_wake == 0, or if there are no threads that can be made ready.

We compute the hash key based on the specified "lock" value, the same way as we do for 
:cfunc:`H2K_futex_wait()`.  

Next, we acquire the BKL.

If there is nothing in the futex hash at the appropriate bucket, we return,
since there is no thread waiting on the lock.

Otherwise, we search through the threads at the bucket for matching threads.
Matching threads, up to n_to_wake, are removed from the futex hash bucket and
added to the ready queue.  Threads have their `status` field modified to be
`H2K_STATUS_READY`.

Finally, we sibcall to :cfunc:`H2K_check_sanity_unlock()`, asking it to return
the number of woken threads.

IMPLEMENTATION CHOICES TBD:
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The futex hash bin can be kept in sorted order.  Sorting can be done on priority and/or 
futex address.  This increases the cost of blocking, but decreases the cost of
waking up the highest priority threads.


H2K_futex_init
----------------

.. cfunction:: void H2K_futex_init(void)

Description
~~~~~~~~~~~

:cfunc:`H2K_futex_init()` initializes the futex data structures.

Functionality
~~~~~~~~~~~~~

H2K_futexhash is set to NULL for each element in the array.




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

.. cfunction:: u32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me)

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


Samples
~~~~~~~

* FUTEX_HASHSIZE - define (default 6)
* lock - pointer to futex

n_to_wake (H2K_futex_resume)

H2K_futex_wait return value
H2K_futex_resume return value

Interesting Cases
~~~~~~~~~~~~~~~~~

Lower 12 bits of futex address all the same
Lower 12 bits of futex address all different

Wait on a valid lock
Wait on an invalid lock

Wake 0 threads (via n_to_wake == 0, or n_to_wake > 0 with no threads ready)
Wake 1 thread
Wake multiple threads


PI raising priority
PI not raising priority (and not lowering)
PI handoff to waiting thread
PI handoff to no thread waiting

Harness
~~~~~~~

Testcases will instantiate the full kernel and make use of multiple software threads.

Future tests may test the hashing function more thoroughly


