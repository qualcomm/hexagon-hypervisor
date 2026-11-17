:mod:`config` -- Runtime kernel configuration
===============================================

.. module:: config

H2K_trap_config
---------------

.. c:function:: u32_t H2K_trap_config(config_type_t configtype, u32_t val1, u32_t val2, u32_t val3, u32_t val4, H2K_thread_context *me)

	:param configtype: Configuration operation
	:param val1: Operation-specific value
	:param val2: Operation-specific value
	:param val3: Operation-specific value
	:param val4: Operation-specific value
	:param me: Current thread context
	:returns: Operation result, or zero for an invalid operation

Dispatches the configuration request through the configuration table.


H2K_trap_config_vmblock_init
----------------------------

.. c:function:: u32_t H2K_trap_config_vmblock_init(u32_t unused, u32_t vm, u32_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)

	:param vm: VM ID for an existing VM, or ignored for ``SET_CPUS_INTS``
	:param op: VM initialization operation
	:param arg1: First operation argument
	:param arg2: Second operation argument
	:param me: Calling thread, used for authorization and parent mapping
	:returns: VM ID, or zero on failure

Creates a VM block or changes an existing stopped VM.  ``SET_CPUS_INTS`` must
be called first; later operations require an existing VM with no running CPUs
and a caller from its parent VM.

Supported operations configure CPU and interrupt storage, the page-map type,
physical fences, priority and trap mask, and virtual-to-physical interrupt
mappings.  Invalid arguments, a running VM, an unauthorized caller, or an
unavailable VM ID returns zero.


H2K_trap_config_stlb_alloc
--------------------------

.. c:function:: u32_t H2K_trap_config_stlb_alloc(u32_t unused, u32_t sets, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me)

	:returns: STLB allocation result

Allocates and initializes the software TLB.  The extra arguments are unused.


H2K_trap_config_fatal_hook
--------------------------

.. c:function:: u32_t H2K_trap_config_fatal_hook(u32_t unused, u32_t funcaddr, u32_t arg, u32_t unused3, u32_t unused4, H2K_thread_context *me)

	:param funcaddr: Fatal-hook function address
	:param arg: Argument saved for the fatal hook
	:returns: Zero

Stores the fatal-hook address, argument, current ASID status, and kernel global
pointer in the global fatal-hook state.


H2K_trap_config_cluster_sched
-----------------------------

.. c:function:: u32_t H2K_trap_config_cluster_sched(u32_t unused, u32_t enable, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me)

	:param enable: Cluster-scheduling enable value
	:returns: Zero

Sets the cluster-scheduling enable flag.  Available when ``CLUSTER_SCHED`` is
enabled.


H2K_trap_config_noc
-------------------

.. c:function:: u32_t H2K_trap_config_noc(u32_t unused, u32_t master, u32_t slave, u32_t unused3, u32_t unused4, H2K_thread_context *me)

	:param master: NoC master base
	:param slave: NoC slave base
	:returns: Zero

Stores the NoC master and slave base values.


H2K_trap_config_local_mem
-------------------------

.. c:function:: u32_t H2K_trap_config_local_mem(u32_t unused, u32_t base, u32_t size, u32_t stride, u32_t unused4, H2K_thread_context *me)

	:param base: Local-memory base
	:param size: Local-memory size
	:param stride: Per-core address stride
	:returns: Zero

Stores the local-memory size and computes this core's base as
``base + stride * H2K_gp->core_id``.
