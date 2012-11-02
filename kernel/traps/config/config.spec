
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

H2K_trap_config_vmblock_init
----------------------------

.. cfunction:: u32_t H2K_trap_config_vmblock_init(u32_t unused, void *ptr, vmblock_init_op_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)

	:param unused: Unused parameter
	:param ptr: Actually a u32_t containing the VM ID
	:param op: Initialization operations to perform
	:param arg1: First parameter for operation
	:param arg2: Second parameter for operation
	:param me: Pointer to current thread context
	:returns: Valid VM ID, or 0 on error

Description
~~~~~~~~~~~

Performs the requested initialization operation on a VM block.  These operations are allowed only to create a new VM block, or to modify an existing VM block that was previously created by the caller.  Modification is allowed only for a VM that has 0 started CPUs.  The valid operations are:

* SET_CPUS_INTS: Allocate storage for VM and associated CPU contexts (always creates a new VM block).  Set max number of virtual CPUs (arg1) and set number active to 0; set number of virtual interrupts (arg2) and clear enable/pending for each.  Initialize CPU contexts for the given number of cpus.  Ignore VM ID parameter.
* SET_PMAP_TYPE: Set page-map pointer (arg1).  If arg1 is NULL, use ptb of current thread as page map.  Set page-map translation type (linear, page tables, or offset) (arg2).  If type is 'offset', then arg1 contains an offset descriptor, [pages (20 bits) : xwru (4 bits) : cccc (4 bits) : size (4 bits)], that is used to generate translations on the fly.  The offset is an unsigned value; negative offsets are specified as large positive values that wrap around: (0xffffffff - offset).
* SET_FENCES: For the offset translation type, set the low (arg1) and high (arg2) physical address of the memory region accessible by the VM.  These are stored as page numbers in the vmblock.
* SET_PRIO_TRAPMASK: Set best allowed priority (arg1); set mask for allowed traps (arg2).
* MAP_PHYS_INTR: Map virtual interrrupt number (arg1) to [physical interrupt number (16 bits) : virtual CPU index (16 bits)] (arg2).  Also sets up the passthru interrupt handler for the given physical interrupt, which will post the virtual interrupt (to the given CPU index in the case of a per-CPU interrupt).  If the virtual interrupt number is not in the per-CPU range, then the CPU-index argument is ignored.  

SET_CPUS_INTS must be the first operation invoked.  Subesequent operations should use the ID value returned by SET_CPUS_INTS .  Other operations may be invoked in arbitrary order.

Functionality
~~~~~~~~~~~~~

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


