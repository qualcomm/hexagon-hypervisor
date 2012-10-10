:mod:`vmboot` -- Virtual Machine Startup
====================================================

.. module:: vmboot

H2K_vmboot
----------

.. cfunction:: s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me)

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


H2K_vmstatus
------------

.. cfunction:: u32_t H2K_vmstatus(u32_t vm, H2K_thread_context *me)

	:param vm: Index of target virtual machine
	:param me: Pointer to the current thread context
	:returns: The status word of the target VM, or 0 on error

Description
~~~~~~~~~~~

Returns the status word of the target VM, whose value is set by the most recent
call to :cfunc:`H2K_thread_stop()`.  This value is not interpreted in any way
by the H2 kernel.  The error value is returned if the target VM was not created
by the calling VM.
