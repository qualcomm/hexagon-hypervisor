
:mod:`futex_classic` -- Generic Blocking / Unblocking Services
==============================================================

.. module:: futex_classic

H2K_futex_wait
--------------

.. c:function:: s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me)

	:param lock: a user-specified pointer to a word in memory
	:param val: the expected value for `*lock`
	:param me: pointer to the current thread.
	:returns: 0 if the thread blocked, -1 otherwise.

Description
~~~~~~~~~~~

:c:func:`H2K_futex_wait()` asks the kernel to block, but only if the value pointed to by "lock" is equal
to "val".

If the value has changed, the kernel returns -1.

If the thread went to sleep, the kernel returns 0.

Per the VMWORK contract (see :doc:`../../util/vmdefs/vmdefs`), -1 may
also be returned spuriously when VMWORK is observed after the BKL is
acquired. In case of invalid translation -1 is returned.
Callers must treat -1 as "did not block -- re-check the
wait condition and retry (best effort)."

Functionality
~~~~~~~~~~~~~

We lock the BKL.

If VMWORK is pending on the current thread, we call
:c:func:`H2K_vm_do_work_withlock()`, unlock the BKL, and return -1.
Otherwise, while still holding the BKL, we check the value at the pointer.
Care must be taken while checking the value pointed to, as we cannot trust
that the user pointer is correct.  This is done using
:c:func:`H2K_safemem_check_and_lock()`, which also returns the physical
address backing the pointer.  If the translation is invalid, we unlock the
BKL and return -1.

If the value read does not match "val", we unlock the BKL and return -1.

Otherwise, we save the physical address (shifted to a page/word-granular
value) in the thread's `futex_ptr` field, remove the current thread from the
list of running threads, set the `r0100` field in the context to 0, update
the `status` field to `H2K_STATUS_BLOCKED`, and add it to the futex hash table
using a hash key derived from the physical address.
We then call :c:func:`H2K_dosched()` for a new thread to be scheduled.

H2K_futex_resume
----------------

.. c:function:: s32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me)

	:param lock: a user-specified value
	:param n_to_wake: a user-specified maximum number of threads to wake
	:param me: pointer to the current thread.
	:returns: the number of threads woken, negative value on failure

Description
~~~~~~~~~~~

:c:func:`H2K_futex_resume()` wakes threads waiting on the location specified by "lock".  A maximum of
"n_to_wake" threads are awoken.

The kernel returns the number of woken threads.

Functionality
~~~~~~~~~~~~~

If the number of threads to wake is zero, there is nothing to be done.  We return 0.
This can happen if n_to_wake == 0, or if there are no threads that can be made ready.

Before acquiring the BKL, we translate "lock" to a physical address using
:c:func:`H2K_safemem_check_and_lock()`, and compute the hash key from that
physical address.  If the translation fails, we return -1.

Next, we acquire the BKL.

If there is nothing in the futex hash at the appropriate bucket, we unlock the
BKL and return 0, since there is no thread waiting on the lock.

Otherwise, we repeatedly call :c:func:`H2K_futex_hash_remove_one()` to remove
and ready matching threads from the bucket, up to n_to_wake, or until no more
matching threads remain.  Threads that are removed have their `status` field
modified to be `H2K_STATUS_READY`.

