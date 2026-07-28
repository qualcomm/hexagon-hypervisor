:mod:`vmop` -- Virtual Machine Startup
====================================================

.. module:: vmop

H2K_vmop
--------

.. c:function:: s32_t H2K_vmop(vmop_t op, u32_t val1, u32_t val2, u32_t val3, u32_t val4, u32_t val5,  H2K_thread_context *me)

	:param op: Requested VM operation
	:param val1: Operation-dependent value
	:param val2: Operation-dependent value
	:param val3: Operation-dependent value
	:param val4: Operation-dependent value
	:param val5: Operation-dependent value
	:param me: Pointer to the current thread context
	:returns: Return code for the VM operation, or -1 on error.

Description
~~~~~~~~~~~

Multi-trap handler for VM operations.  Valid operations are:

* VMOP_BOOT:  Boot a VM.
* VMOP_STATUS: Get VM status.
* VMOP_FREE: Release vmblock memory and destroy VM.
* VMOP_KILL_THREAD: Request that another thread (or self) terminate.
* VMOP_KILL_CHILD: Request that every thread of a child VM terminate.

Returns error value if the requested operation is illegal.




H2K_vmop_boot
-------------

.. c:function:: s32_t H2K_vmop_boot(vmop_t unused0, u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param pc: Initial PC for the first virtual CPU
	:param sp: Initial stack pointer for the first virtual CPU
	:param arg: A value for r0 for the first virtual CPU
	:param prio: The priority for the first virtual CPU
	:param vm: Index of target virtual machine
	:param me: Pointer to the current thread context
	:returns: The thread ID of the new thread executing the virtual CPU, or -1 on failure.

Description
~~~~~~~~~~~

Starts the virtual machine described by the given vm ID, which must have been
initialized by calling :c:func:`H2K_trap_config_vmblock_init()`.  The remaining
parameters define the thread that implements the first virtual CPU (see
:c:func:`H2K_thread_create()`).  The error value is returned if the target VM
was not created by the calling VM, or if the target VM has running virtual
CPUS.

Functionality
~~~~~~~~~~~~~

Pass all arguments through to :c:func:`H2K_thread_create()`.


H2K_vmop_status
---------------

.. c:function:: s32_t H2K_vmop_status(vmop_t unused0, u32_t op, u32_t vm, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param op: Status type requested
	:param vm: Index of target virtual machine; VMOP_STATUS_VMIDX_SELF to query self
	:param me: Pointer to the current thread context
	:returns: The requested status of the target VM, or -1 on error

Description
~~~~~~~~~~~
Returns VM status according to the requested operation:

* VMOP_STATUS_STATUS: Returns the status word of the target VM, whose value is set by the most recent call to :c:func:`H2K_thread_stop()`.  This value is not interpreted in any way by the H2 kernel (however, it should not conflict with the error value that may be returned by this function) .  

* VMOP_STATUS_CPUS: Returns the number of running virtual CPUS

The error value is returned if the target VM was not created by the calling VM
or if the target is not the calling VM itself, or an illegal status operation
was requested.


H2K_vmop_free
-------------

.. c:function:: s32_t H2K_vmop_free(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param vm: Index of target virtual machine
	:returns: 0 on success, -1 on error

Description
~~~~~~~~~~~
Frees the monitor memory used by the given VM; makes the VM ID available for re-use.

The error value is returned if the target VM was not created by the calling VM
or the target VM has running CPUs (vcpu count is not 0).

Functionality
~~~~~~~~~~~~~

Check that target VM is child of caller and its vcpu count is 0.  Call :c:func: `H2K_mem_alloc_free()` on the target vmblock.  Set the vmblock pointer for the targ VM ID to null.


H2K_vmop_kill_thread
--------------------

.. c:function:: s32_t H2K_vmop_kill_thread(vmop_t unused0, u32_t id, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param id: Raw H2K_id_t value (vmidx + cpuidx) identifying the target thread
	:param me: Pointer to the current thread context
	:returns: 0 on success (including when the target is already dead or its VM is gone), -1 on error

Description
~~~~~~~~~~~

Request that the thread identified by ``id`` terminate.  The target's
``H2K_VMSTATUS_KILL`` and ``H2K_VMSTATUS_VMWORK`` bits are set atomically, and
the target is forced out of whatever state it is in (BLOCKED, INTBLOCKED,
VMWAIT, RUNNING) so it runs :c:func:`H2K_vm_do_work` and self-stops.  Unlike
interrupt delivery, kill ignores the target's guest IE bit.

The target may be the calling thread itself; this proceeds through the same
path (self-IPI from the RUNNING case).

The error value is returned if the target's VM is not the caller's VM or a
child of the caller's VM, or if the id is malformed.  Killing an
already-dead thread, or a thread whose VM has already been freed, returns 0.


H2K_vmop_kill_child
-------------------

.. c:function:: s32_t H2K_vmop_kill_child(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param vm: Index of target child VM
	:param me: Pointer to the current thread context
	:returns: 0 on success (including when the target VM has already been freed), -1 on error

Description
~~~~~~~~~~~

Iterates over every thread context of the target VM and applies the same
kill machinery as :c:func:`H2K_vmop_kill_thread`.  Already-dead slots are
skipped.  Each vcpu wakes, hits :c:func:`H2K_vm_do_work`, and self-stops;
the last one out signals the parent through ``H2K_VM_CHILDINT`` and, if no
further references exist, frees the vmblock through the usual
:c:func:`H2K_thread_stop` path.

The calling thread is included in the kill loop if it is a member of the
target VM.  It receives ``KILL|VMWORK`` and an IPI, which fires on the
first userspace instruction after this call returns.  Callers must not
attempt to acquire any mutex or enter a blocking primitive after this
call -- sibling threads may have died holding arbitrary locks.

The caller must be the parent of the target VM or a thread of the target VM
itself.  Unrelated callers are rejected with -1.
