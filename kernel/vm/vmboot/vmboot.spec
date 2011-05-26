:mod:`vmboot` -- Virtual Machine Startup
====================================================

.. module:: vmboot

H2K_vmboot
---------------------

.. cfunction:: s32_t H2K_vmboot(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)

	:param pc: Initial PC for the first virtual CPU
	:param sp: Initial stack pointer for the first virtual CPU
	:param arg: A value for r0 for the first virtual CPU
	:param prio: The priority for the first virtual CPU
	:param vmblock: Pointer to VM configuration block
	:param me: Pointer to the current thread context
	:returns: the thread ID of the new thread executing the virtual CPU, or -1 on failure.

Description
~~~~~~~~~~~

Starts the virtual machine described by the given vmblock, which must have been
initialized by calling :cfunc:`H2K_trap_config_vmblock_size()` and
:cfunc:`H2K_trap_config_vmblock_init()`.  The remaining parameters define the thread
that implements the first virtual CPU (see :cfunc:`H2K_thread_create()`).

Functionality
~~~~~~~~~~~~~

Pass all arguments through to :cfunc:`H2K_thread_create()`.

