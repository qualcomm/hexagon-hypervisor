:mod:`vmblock` -- virtual machine state
=======================================

.. module:: vmblock

This module defines the state shared by the virtual CPUs in a virtual machine.

Each VM has its own :c:type:`H2K_vmblock_t`, registered in the kernel's
``H2K_gp->vmblocks`` table at the index given by :c:member:`vmidx`.  A thread's
VM ID selects its VM block, and the VM block supplies the translation context
used for that thread's guest addresses.  This gives each VM an independent
translation configuration; it does not by itself guarantee that VMs cannot
map the same physical memory.

The translation path uses the VM's :c:member:`guestmap` to translate guest
physical addresses through a parent mapping when one exists.  For offset
mappings, :c:member:`phys_offset`, :c:member:`fence_lo`, and
:c:member:`fence_hi` constrain the physical pages that the VM may access.

H2K_vmblock_t
-------------

Description
~~~~~~~~~~~

.. c:type:: H2K_vmblock_t

	The state and resources associated with one virtual machine.  The structure is
	32-byte aligned.

	.. c:member:: u32_t vmidx

		Index of this virtual machine.

	.. c:member:: H2K_id_t parent

		ID of the virtual machine's parent.

	.. c:member:: s32_t status

		Virtual machine status, such as its halt or reboot status.

	.. c:member:: u32_t trapmask

		Mask of traps that the virtual machine is permitted to use.

	.. c:member:: H2K_spinlock_t lock

		Lock protecting virtual-machine state.

	.. c:member:: H2K_ext_context *ext_contexts

		Extended contexts for the virtual CPUs.  Present when ``DO_EXT_SWITCH``
		is enabled.

	.. c:member:: H2K_asid_entry_t guestmap

		Address-space translation information for the guest.

	.. c:member:: u8_t tlbidxmask

		Mask used when selecting guest TLB indices.

	.. c:member:: H2K_offset_t phys_offset

		Offset translation descriptor for the guest physical-memory map.

	.. c:member:: u32_t pmap

		Raw representation of the physical-memory map.

	.. c:member:: s32_t fence_lo

		Lowest permitted physical page for offset translation.

	.. c:member:: s32_t fence_hi

		Highest permitted physical page for offset translation.

	.. c:member:: long_bitmask_t waiting_cpus

		Mask of virtual CPUs waiting for an event.

	.. c:member:: u32_t num_ints

		Number of shared virtual interrupts.

	.. c:member:: bitmask_t *pending

		Pending virtual-interrupt bitmask.

	.. c:member:: bitmask_t *enable

		Global virtual-interrupt enable bitmask.

	.. c:member:: bitmask_t **percpu_mask

		Per-CPU virtual-interrupt masks.  A zero bit disables the interrupt for
		the corresponding CPU.

	.. c:member:: physint_t *int_v2p

		Mapping from virtual interrupts to physical interrupts.

	.. c:member:: H2K_vm_int_opinfo_struct *intinfo

		Operation information associated with each virtual interrupt.

	.. c:member:: u32_t max_cpus

		Maximum number of virtual CPUs allocated to this virtual machine.

	.. c:member:: u32_t num_cpus

		Current number of virtual CPUs in this virtual machine.

	.. c:member:: u32_t bestprio

		Best priority permitted for threads in this virtual machine.

	.. c:member:: H2K_thread_context *intpool

		Thread context associated with the interrupt pool.

	.. c:member:: unsigned int intpool_anypending

		Indicates whether the interrupt pool has any pending interrupts.

	.. c:member:: u32_t intpool_pending[MAX_INTERRUPTS/32]

		Pending interrupt-pool bits.

	.. c:member:: H2K_thread_context *free_threads

		Head of the virtual machine's list of free thread contexts.

	.. c:member:: H2K_thread_context *contexts

		Storage for the virtual machine's thread contexts.

	.. c:member:: u32_t flags

		Raw virtual-machine flags.

	.. c:member:: u32_t use_ext

		Indicates that the guest requests extended-context switching.  Present
		when ``HAVE_EXTENSIONS`` is enabled.

	.. c:member:: u32_t do_ext

		Indicates that extended-context switching is active.  Present when both
		``HAVE_EXTENSIONS`` and ``DO_EXT_SWITCH`` are enabled.

	.. c:member:: u32_t flags_unused

		Unused bits in :c:member:`flags`.

	.. c:member:: u64_t tlbmissx

		Count of execute TLB misses.  Present when ``COUNT_TLB_EVENTS`` is enabled.
		The value can also be accessed through ``tlbmissx_lo`` and ``tlbmissx_hi``.

	.. c:member:: u64_t tlbmissrw

		Count of read/write TLB misses.  Present when ``COUNT_TLB_EVENTS`` is
		enabled.  The value can also be accessed through ``tlbmissrw_lo`` and
		``tlbmissrw_hi``.

	.. c:member:: u64_t stlbmiss

		Count of software-TLB misses.  Present when ``COUNT_TLB_EVENTS`` is
		enabled.  The value can also be accessed through ``stlbmiss_lo`` and
		``stlbmiss_hi``.

	.. c:member:: u64_t dmatlbmiss

		Count of DMA TLB misses.  Present when ``COUNT_TLB_EVENTS`` is enabled.
		The value can also be accessed through ``dmatlbmiss_lo`` and
		``dmatlbmiss_hi``.
