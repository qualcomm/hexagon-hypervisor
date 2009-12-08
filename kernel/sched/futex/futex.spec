
UNIT: futex

FUNCTION: s32_t H2K_futex_wait(u32_t *ptr, u32_t expected, H2K_thread_context *me)

DESCRIPTION:

H2K_futex_wait asks the kernel to block, but only if the value pointed to by "ptr" is equal
to "expected".  

If the value has changed, the kernel returns -1.

If the thread went to sleep, the kernel returns 0.

INPUT:

Argument 0: "ptr", which is a user-specified pointer to a word in memory
Argument 1: "expected", which is the expected value for *ptr
Argument 2: "me", which is the pointer to the current thread.

OUTPUT:

Returns -1 or 0, depending on whether the thread blocked or not.

FUNCTIONALITY:

First, the hash key is computed.  The hash key is based on the address used
by the kernel.  This should either be based only on the lowest bits of the
address, that are unchagned by virtual to physical translation, or should be
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

Otherwise, we remove the current thread from the list of running threads, Add
it to the futex hash table using the hash key.  We then call for a new thread 
to be scheduled.  The return value must be zero.


FUNCTION: u32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)

DESCRIPTION:

H2K_futex_resume wakes threads waiting on the location specified by "lock".  A maximum of 
"n_to_wake" threads are awoken.  

The kernel returns the number of woken threads.

INPUT:

Argument 0: "ptr", which is a user-specified value
Argument 1: "n_to_wake", which is a user-specified maximum number of threads to wake
Argument 2: "me", which is the pointer to the current thread.

OUTPUT:

Returns the number of threads woken.

FUNCTIONALITY:

If the number of threads to wake is zero, there is nothing to be done.  We return 0.

We compute the hash key based on the specified "lock" value, the same way as we do for 
H2K_futex_wait.  

Next, we acquire the BKL.

If there is nothing in the futex hash at the appropriate bucket, we return,
since there is no thread waiting on the lock.

Otherwise, we search through the threads at the bucket for matching threads.
Matching threads, up to n_to_wake, are removed from the futex hash bucket and
added to the ready queue.  

If no threads were woken, we can simply unlock the BKL and return 0.  

Otherwise, we check sanity, unlock, and return the number of woken threads.

IMPLEMENTATION CHOICES TBD:

The futex hash bin can be kept in sorted order.  Sorting can be done on priority and/or 
futex address.  This increases the cost of blocking, but decreases the cost of
waking up the highest priority threads.





