:mod:`safemem` -- Access user-defined pointers safely
=====================================================

.. module:: safemem

H2K_safemem_check_and_lock
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. cfunction:: u32_t H2K_safemem_check_and_lock(u32_t *user_va, u32_t perms, pa_t *pa_out,
		H2K_thread_context *me)

	:param user_va: user-specified virtual address
	:param perms: Permissions to check
	:param pa_out: if successful, write the physical address to this parameter
	:param me: Pointer to the current thread context
	:returns: 1 on success, 0 on failure

Description
~~~~~~~~~~~

:cfunc:`H2K_safemem_writelock()` prepares the monitor for safe access of a
user-specified address.  If memory at the specified address can be safely
accessed, the TLB is locked and the physical address and a success indication
is returned.  Otherwise, if the memory can not be read safely, an indication of
failure is returned and the TLB is not locked.

Functionality
~~~~~~~~~~~~~

First, we check that the user VA is a valid word pointer.  If not, we fail.

Next, we lock the TLB to prevent further modifications.

Next, we probe the TLB for the supplied address.  If there is no matching TLB
entry, we unlock the TLB and return failure.

Next, we read the TLB entry that the probe returned.  We check that the 
permissions are acceptable according to the provided value.  Additionally, if
the thread is in user mode, we check that the translation has user permission.  
If permissions are insufficient, we unlock the TLB and return failure.

We use the information in the TLB entry to form a physical address, written
to the provided location.  Finally, we return success without releasing the
TLB lock; it is the responsibility of the caller to call 
:cfunc:`H2K_safemem_unlock()`.


H2K_safemem_unlock
~~~~~~~~~~~~~~~~~~

.. cfunction:: void H2K_safemem_unlock()

Description
~~~~~~~~~~~

:cfunc:`H2K_safemem_unlock()` unlocks the TLB that has been temporarily locked 
in order to allow the monitor to read or write a user-defined pointer safely.

Insert TLB-Locked assertion here?

Functionality
~~~~~~~~~~~~~

Unlock the TLB Lock







