
:mod:`config` -- runtime kernel configuration
=============================================

.. module:: config

This module contains the runtime kernel configuration routines.


H2K_trap_config
---------------

.. cfunction: void H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: Configuration operation to do
	:param ptr: Operation-dependent pointer value
	:param val2: Operation-dependent value
	:param val3: Operation-dependent value
	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Trap handler for kernel configuration.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The configtype parameter determines which configuration routine is called.
If an invalid configtype parameter is given, we return imediately.

We look up which configuration routine in H2K_configtab, and jump to the 
appropriate routine.


H2K_trap_config_addthreads
--------------------------

.. cfunction: void H2K_trap_config_addthreads(u32_t unused, void *ptr, u32_t size, u32_t unused2, H2K_thread_context *me)

	:param unused: Unused parameter
	:param ptr: Pointer for additional memory for use as thread contexts
	:param size: Size of the data area in bytes
	:param unused2: Unused parameter
	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Adds additional memory for use as thread contexts.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

First, the pointer should be adjusted so that it is 32-byte aligned.
This must be a rounding up.  Size must be decremented by any difference
in rounding.

Next, size should be rounded down to the nearest multiple of CONTEXT_SIZE.

Finally, the memory is chunked into size/CONTEXT_SIZE thread contexts.
Each context is cleared, and then inserted into the H2K_free_threads list.

TBD: remove threads?
TBD: keep list of thread areas for better threadids.


H2K_trap_config_setfatal
------------------------

.. cfunction: void H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, H2K_thread_context *me)


	:param unused: Unused paramater
	:param handler: Function to be called in the event of a fatal error
	:param unused2: Unused parameter
	:param unused3: Unused parameter
	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Configures the kernel fatal error handler function.


Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Set H2K_fatal_kernel_handler to handler.


Testing
-------

Samples
~~~~~~~

* Input: Storage size
* Input: Alignment of pointer
* Output: H2K_free_threads
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

We try several calls to H2K_trap_config, and check that H2K_free_threads is
updated to hold the appropriate values, or that fatal_kernel_handler is changed.


