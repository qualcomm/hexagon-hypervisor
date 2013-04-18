:mod:`vmop` -- Virtual Machine Startup
====================================================

.. module:: vmop

H2K_vmop
--------

.. cfunction:: s32_t H2K_vmop(vmop_t op, u32_t val1, u32_t val2, u32_t val3, u32_t val4, u32_t val5,  H2K_thread_context *me)

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

Returns error value if the requested operation is illegal.




H2K_vmop_boot
-------------

.. cfunction:: s32_t H2K_vmop_boot(vmop_t unused0, u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me)

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
initialized by calling :cfunc:`H2K_trap_config_vmblock_init()`.  The remaining
parameters define the thread that implements the first virtual CPU (see
:cfunc:`H2K_thread_create()`).  The error value is returned if the target VM
was not created by the calling VM, or if the target VM has running virtual
CPUS.

Functionality
~~~~~~~~~~~~~

Pass all arguments through to :cfunc:`H2K_thread_create()`.


H2K_vmop_status
---------------

.. cfunction:: s32_t H2K_vmop_status(vmop_t unused0, u32_t op, u32_t vm, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

	:param unused0: Unused parameter
	:param op: Status type requested
	:param vm: Index of target virtual machine
	:param me: Pointer to the current thread context
	:returns: The requested status of the target VM, or -1 on error

Description
~~~~~~~~~~~
Returns VM status according to the requested operation:

* VMOP_STATUS_STATUS: Returns the status word of the target VM, whose value is set by the most recent call to :cfunc:`H2K_thread_stop()`.  This value is not interpreted in any way by the H2 kernel (however, it should not conflict with the error value that may be returned by this function) .  

* VMOP_STATUS_CPUS: Returns the number of running virtual CPUS

The error value is returned if the target VM was not created by the calling VM
or an illegal status operation was requested.


H2K_vmop_free
-------------

.. cfunction:: s32_t H2K_vmop_free(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me)

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

Check that target VM is child of caller and its vcpu count is 0.  Call :cfunc: `H2K_mem_alloc_free()` on the target vmblock.  Set the vmblock pointer for the targ VM ID to null.
