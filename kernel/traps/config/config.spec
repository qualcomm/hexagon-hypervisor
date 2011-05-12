
:mod:`config` -- runtime kernel configuration
=============================================

.. module:: config

This module contains the runtime kernel configuration routines.


H2K_trap_config
---------------

.. cfunction:: u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: Configuration operation to do
	:param ptr: Operation-dependent pointer value
	:param val2: Operation-dependent value
	:param val3: Operation-dependent value
	:param me: Pointer to the current thread context
	:returns: The return code for the configuration option.

Description
~~~~~~~~~~~

Trap handler for kernel configuration.

Functionality
~~~~~~~~~~~~~

The configtype parameter determines which configuration routine is called.
If an invalid configtype parameter is given, we return imediately.

We look up which configuration routine in H2K_configtab, and jump to the 
appropriate routine.


H2K_trap_config_addthreads
--------------------------

.. cfunction:: u32_t H2K_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me)

	:param unused: Unused parameter
	:param ptr: Pointer for additional memory for use as thread contexts
	:param size: Size of the data area in bytes
	:param unused2: Unused parameter
	:param me: Pointer to the current thread context
	:returns: the number of thread contexts added

Description
~~~~~~~~~~~

Adds additional memory for use as thread contexts.

Functionality
~~~~~~~~~~~~~

First, the pointer should be adjusted so that it is 32-byte aligned.
This must be a rounding up.  Size must be decremented by any difference
in rounding.

Next, size should be rounded down to the nearest multiple of CONTEXT_SIZE.

Finally, the memory is chunked into size/CONTEXT_SIZE thread contexts.
Each context is cleared, and then inserted into the H2K_kg.free_threads list.

TBD: remove threads?
TBD: keep list of thread areas for better threadids.


H2K_trap_config_setfatal
------------------------

.. cfunction:: u32_t H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me)


	:param unused: Unused parameter
	:param handler: Function to be called in the event of a fatal error
	:param unused2: Unused parameter
	:param unused3: Unused parameter
	:param me: Pointer to the current thread context
	:returns: zero

Description
~~~~~~~~~~~

Configures the kernel fatal error handler function.


Functionality
~~~~~~~~~~~~~

Set H2K_fatal_kernel_handler to handler.

H2K_trap_config_vmblock_size
----------------------------

.. cfunction:: H2K_trap_config_vmblock_size(u32_t unused, u8_t max_cpus, u16_t num_ints, H2K_thread_context *me)

	:param unused: Unused parameter
	:param max_cpus: Max number of virtual CPUs
	:param num_ints: Number of virtual interrupts
	:param me: Pointer to current thread context
	:returns: Space in bytes needed for aligned storage of the VM data

Description
~~~~~~~~~~~

Calculates space needed to store VM data sized according to the given parameters.

Functionality
~~~~~~~~~~~~~

Compute space for VM struct plus interrupt mask arrays.  Allow extra space for alignment.

H2K_trap_config_vmblock_init
----------------------------

.. cfunction:: H2K_vmblock_t *H2K_trap_config_vmblock_init(u32_t unused, void *ptr, vmblock_init_op_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)

	:param unused: Unused parameter
	:param ptr: Pointer to space available for VM block, or pointer to VM block to modify
	:param op: Initialization operations to perform
	:param arg1: First parameter for operation
	:param arg2: Second parameter for operation
	:param me: Pointer to current thread context
	:returns: Valid VM block pointer, or NULL on error

Description
~~~~~~~~~~~

Performs the requested initialization operation on the VM block.  The valid operations are:

* SET_STORAGE_IDENT_PMAP: Initialize storage for VM block (ptr); set VM ID (arg1); set page-map pointer (arg2)
* SET_PRIO_TRAPMASK: Set best allowed priority (arg1); set mask for allowed traps (arg2)
* SET_CPUS_INTS: Set max number of virtual CPUs (arg1) and set number active to 0; set number of virtual interrupts (arg2) and clear enable/pending for each.
* MAP_PHYS_INTR: Map virtual interrrupt number (arg1) to physical interrupt number (arg2)

SET_STORAGE_IDENT_PMAP must be the first operation invoked.  Subesequent operations may occur in any order and should use the pointer value returned by SET_STORAGE_IDENT_PMAP.

Functionality
~~~~~~~~~~~~~

For SET_STORAGE_IDENT_PMAP, align pointer (config_vmblock_size allows space for alignment).

For all operations, check for bad args where possible, initialize VM block with given values.



Testing
-------

Samples
~~~~~~~

* Input: Storage size
* Input: Alignment of pointer
* Output: H2K_kg.free_threads
* For setfatal: handler function

Important Cases
~~~~~~~~~~~~~~~

* Invalid config parameter
* Aligned pointer
* Misaligned pointer
* Insufficient storage for any threads
* Sufficient storage for one thread
* Sufficient storage for many threads

Harness
~~~~~~~

Standalone environment, but linked with h2kernel.

We try several calls to H2K_trap_config, and check that H2K_kg.free_threads is
updated to hold the appropriate values, or that fatal_kernel_handler is changed.


