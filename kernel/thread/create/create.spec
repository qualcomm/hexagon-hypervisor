
:mod:`thread_create` -- create a new thread
============================================

.. module:: thread_create

H2K_thread_create
-----------------

.. cfunction:: s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg, u32_t prio, u32_t trapmask, H2K_thread_context *me)

Description
~~~~~~~~~~~

The H2K_thread_create function creates a new thread.  The calling thread
specifies attributes for the thread, and the new thread is created
with those attributes.  New threads can have reduced features, by
specifying the mask of allowed traps.

The call to thread_create will return -1 on error, or an ID for the
thread on success.

Input
~~~~~
Argument 0: The PC that the new thread should start at
Argument 1: The SP that the new thread should start with
Argument 2: A value for r0 for the new thread
Argument 3: The priority for the new thread
Argument 4: The mask for what traps are allowed for the new thread
Argument 5: Pointer to the current thread context

Output
~~~~~~

Returns -1 or the thread ID of the new thread


Functionality
~~~~~~~~~~~~~

Arguments should be checked:
* Priority should be less than MAX_PRIO
* ASID should be less than MAX_ASID (TBD: redo mem stuff)
* SP must be 8-byte aligned

Any incorrect argument causes -1 to be returned.

If the arguments are correct, we acquire the Big Kernel Lock.

If H2K_free_threads is NULL, we free the BKL and -1 is returned (there are no
available threads to create).

Otherwise, we pop a thread off the H2K_free_threads list to use as a new thread.

As a convienience, the new thread inherits GP and UGP from the calling thread.
The thread is free to change these values if desired.

We then set the following parameters in the new thread context:

* SP is set to the SP argument value
* SSR is set to the Kernel Default SSR Value
* ELR is set to the PC argument value
* R0 is set to the arg argument value
* priority is set to the prio argument value
* valid is set to VALID
* The continuation is set to interrupt restore continuation

We then add the new thread to the readylist, and call check_sanity_unlock
before returning.

For security, we need to assure that no values are in registers incorrectly.
We accomplish this by clearing the thread context during initialization and 
at H2K_thread_stop time.  This allows for faster thread_create calls.



Testing
-------


Samples
~~~~~~~

* Input: new thread PC
* Input: new thread SP
* Input: new thread argument
* Input: new thread priority
* Input: new thread trapmask
* Input: Pointer to current thread context

Important cases
~~~~~~~~~~~~~~~

* Free thread list is empty
* Free thread list is non-empty

* PC misaligned
* Stack Pointer misaligned
* Invalid thread priority

* Ready threads at new thread priority
* No ready threads at new thread priority

Harness
~~~~~~~

We link directly with the create object file, and also the readylist object file.

We define H2K_check_sanity_unlock() to set a flag indicating that the function was called.

The test harness will call H2K_thread_create with various inputs and check to make sure
that the appropriate action was taken:

* Check appropriate input arguments, return -1 if obviously erroneous
* If available, a thread should be dequeued from the free thread list.  Otherwise return -1.
* Check that the appropriate fields in the new thread were set correctly:
	* Priority and valid fields
	* SSR and ELR values
	* Argument and Stack Pointer values
	* Trap Mask
	* Continuation
* Check to make sure the thread was inserted into the ready queue at the appropriate location.

