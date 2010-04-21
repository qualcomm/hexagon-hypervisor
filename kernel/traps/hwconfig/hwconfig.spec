
:mod:`hwconfig` -- Runtime Hardware Configuration
=================================================

.. module:: hwconfig

H2K_trap_hwconfig
-----------------

.. cfunction:: u32_t H2K_trap_hwconfig(u32_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: Configuration operation to do
	:param ptr: Operation-dependent pointer value
	:param val2: Operation-dependent value
	:param val3: Operation-dependent value
	:param me: Pointer to the current thread context
	:returns: The return code for the configuration option.

Description
~~~~~~~~~~~

Trap handler for hardware configuration


Functionality
~~~~~~~~~~~~~

The configtype parameter determines which configuration routine is called.
If an invalid configtype parameter is given, we return imediately.

We look up which configuration routine in H2K_hwconfigtab, and jump to the 
appropriate routine.

H2K_trap_hwconfig_l2cache_size
------------------------------

.. cfunction:: u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me)

	:param unused: Unused parameter
	:param unusedp: Unused parameter
	:param size: Requested size of the L2 cache
	:param use_wb: Configure the L2 cache in WB mode
	:param me: Pointer to the current thread context
	:returns: 0 on success, nonzero on failure

Description
~~~~~~~~~~~

Configures the L2 cache size.

NOTE: As currently specified, this request may take a long time to finish.
During this time the RTOS may be unresponsive.

Functionality
~~~~~~~~~~~~~

First, we inspect the size variable, to see if the requested L2 cache size is supportable 
on the architecture.  If not, we return failure.

Next, we inspect SYSCFG to see if the cache is already set to the correct size and 
modality.  If so, we return success immediately.

If no size change is required, we modify the modality.  If switching from WT to WB, we first
clean the cache lines. Finally, we return success.

Otherwise, we try to enter single thread mode.  If this is unsuccessful, we return failure.

Once in single thread mode, if the cache was previously enabled as writeback,
we clean the entire L2 cache.  We then configure the L2 cache as zero size, issue
an L2KILL instruction, and reconfigure the L2 cache size.

Once the L2 cache has been reconfigured, we leave single thread mode, and return success.


H2K_trap_hwconfig_partitions
----------------------------


.. cfunction:: u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)

	:param unused: Unused parameter
	:param unusedp: Unused parameter
	:param whatcache: Which cache is being repartitioned (L1I, L1D, L2)
	:param configval: Configuration for the repartitioning
	:param me: Pointer to the current thread context
	:returns: 0 on success, nonzero on failure



Description
~~~~~~~~~~~

Configures partition information.


Functionality
~~~~~~~~~~~~~

We set the kernel partition configuration according to the whatcache and
configval parameters, as appropriate for the architecture.




H2K_trap_hwconfig_prefetch
--------------------------


.. cfunction:: u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)

	:param unused: Unused parameter
	:param unusedp: Unused parameter
	:param whatcache: Which type of prefetch is being selected (I/D/SW)
	:param configval: Configuration for the prefetch
	:param me: Pointer to the current thread context
	:returns: 0 on success, nonzero on failure


Description
~~~~~~~~~~~

Configures prefetch characteristics.


Functionality
~~~~~~~~~~~~~

We set the kernel prefetch configuration according to the whatcache and
configval parameters, as appropriate for the architecture.





