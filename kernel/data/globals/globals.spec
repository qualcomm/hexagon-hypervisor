

:mod:`globals` -- Global Data Structures
========================================

.. module:: globals

.. c:type:: H2K_inthandler_t

	An interrupt handler entry containing a handler pointer and its parameter.
	The entry also has a raw 64-bit representation for atomic updates.


.. c:type:: H2K_kg_t

	Kernel Globals kept together for ease of referencing.

	.. c:member:: u64_t ready_valids[MAX_PRIOS/64]

		This is a bitfield.  Bit 0 corresponds to priority 0, bit 1 to priority 1,
		and so on.  If the bit *n* is set, H2K_ready[*n*] must be valid.  If bit
		*n* is clear, H2K_ready[*n*] must be NULL.

	.. c:member:: H2K_timeinfo_t time

		A struct which contains all of the timing related data.

	.. c:member:: u32_t priomask

		A mask for which thread or threads are marked as the lowest priority.
		These threads will be interruptible.  Like :c:member:`wait_mask`,
		Hardware thread *n* corresponds to bit *n* in the mask.  If a bit
		is set in :c:member:`wait_mask`, it should also be set in
		:c:member:`priomask`.

		When ``CLUSTER_SCHED`` is enabled:

	.. c:member:: u32_t wait_mask

		A mask for which threads are in wait mode.  Hardware thread 0
		corresponds to bit 0, hardware thread 1 to bit 1, and so on.

	.. c:member:: H2K_mem_stlb_asid_info_t* stlbptr

		A pointer to a software managed cache of translations for an address space.
		By default there is no STLB, can be added by the corresponding config trap.

	.. c:member:: u32_t syscfg_val

		Cached copy of the SYSCFG register value.

	.. c:member:: u32_t arch

		Architecture version field of the core revision, as read from the
		hardware REV register.

	.. c:member:: u32_t uarch

		Microarchitecture version field of the core revision, as read from the
		hardware REV register.

	.. c:member:: u32_t l2arr

		L2 cache array size field of the core revision.  Used to compute
		:c:member:`l2size`.

	.. c:member:: u32_t metal

		Metal (silicon revision/stepping) field of the core revision.

	.. c:member:: u32_t is_nmi_soft

		Flag set when the current NMI is a "soft" NMI (software-triggered).

	.. c:member:: u32_t *l2_int_base

		Base address of the L2 interrupt controller's interrupt registers.
		Only present when ``H2K_L2_CONTROL`` is defined.

	.. c:member:: u32_t *l2_ack_base

		Base address of the L2 interrupt controller's acknowledge registers.
		Only present when ``H2K_L2_CONTROL`` is defined.

	.. c:member:: u32_t timer_intnum

		Interrupt number used by the system timer.

	.. c:member:: u32_t *hvx_clock

		Address of the register used to enable/disable the clock for the
		HVX coprocessor.

	.. c:member:: u32_t *hvx_reset

		Address of the register used to assert/deassert reset for the
		HVX coprocessor.

	.. c:member:: u32_t *hvx_power

		Address of the register used to control power to the HVX coprocessor.

	.. c:member:: u32_t *hvx_bhs_status

		Address of the status register for the HVX coprocessor's power
		head-switch (BHS), used when powering the HVX coprocessor on or off.
		Only present when ``ARCHV >= 65``.

	.. c:member:: u32_t *hvx_bhs_cfg

		Address of the configuration register for the HVX coprocessor's
		power head-switch (BHS).  Only present when ``ARCHV >= 65``.

	.. c:member:: u32_t *hvx_bhs_cmd

		Address of the command register used to trigger updates to the HVX
		coprocessor's power head-switch (BHS).  Only present when
		``ARCHV >= 65``.

	.. c:member:: u32_t *hvx_cpmem_cfg

		Address of the configuration register controlling power state
		(e.g. VTCM sleep/retention) of the HVX coprocessor memory.  Only
		present when ``ARCHV >= 65``.

	.. c:member:: u32_t *hvx_cpmem_cmd

		Address of the command register used to trigger updates to the HVX
		coprocessor memory power state.  Only present when ``ARCHV >= 65``.

	.. c:member:: u32_t *hvx_cpmem_status

		Address of the status register reporting the HVX coprocessor
		memory power state.  Only present when ``ARCHV >= 65``.

	.. c:member:: u32_t hvx_state

		Current power state of the HVX coprocessor.

	.. c:member:: u32_t hvx_vlength

		Native HVX vector length, in bytes.

	.. c:member:: u32_t coproc_contexts

		Number of native-length coprocessor contexts available.

	.. c:member:: u32_t hmx_units

		Number of HMX coprocessor units present.

	.. c:member:: u32_t hmx_state

		Current power state of the HMX coprocessor.  Only present
		when ``ARCHV >= 68``.

	.. c:member:: u32_t *hmx_rsc_seq_busy_drv0

		Address of the resource sequencer busy-status register used when
		sequencing HMX power changes.  Only present when ``ARCHV >= 68``.

	.. c:member:: u32_t *hmx_rsc_seq_override_trigger_drv0

		Address of the resource sequencer register used to trigger a
		manual override sequence for HMX power changes.  Only present
		when ``ARCHV >= 68``.

	.. c:member:: u32_t *hmx_rsc_seq_override_trigger_start_addr_drv0

		Address of the resource sequencer register holding the start
		address of the override sequence used for HMX power changes.
		Only present when ``ARCHV >= 68``.

	.. c:member:: u32_t hmx_rsc_seq_power_on_start_addr

		Resource sequencer start address for the sequence that powers on
		the HMX coprocessor.  Only present when ``ARCHV >= 68``.

	.. c:member:: u32_t hmx_rsc_seq_power_off_start_addr

		Resource sequencer start address for the sequence that powers off
		the HMX coprocessor.  Only present when ``ARCHV >= 68``.

	.. c:member:: pa_t gpio_reg

		Physical address of the GPIO register used when configuring GPIO
		access for a guest.  Only present when ``ARCHV >= 68``.

	.. c:member:: u32_t cluster_clusters

		Number of hardware thread clusters.  Only present when
		``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t cluster_hthreads

		Number of hardware threads per cluster.  Only present when
		``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t cluster_mask[]

		Bitmask of hardware threads belonging to each cluster.  Only
		present when ``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t cluster_sched

		Flag indicating whether cluster scheduling is in effect.  Only
		present when ``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t coproc_max

		Maximum number of coprocessor threads permitted per cluster.  Only
		present when ``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t coproc_max_save

		Maximum number of coprocessor threads permitted per cluster when
		all hardware threads are enabled.  Only present when
		``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t coproc_count[CLUSTER_MAX_CLUSTERS]

		Number of coprocessors currently active in each cluster.  Only
		present when ``CLUSTER_SCHED`` is enabled.

	.. c:member:: u32_t fatal_hook_arg

		Argument to be passed to the fatal error hook, if one is set.

	.. c:member:: u32_t fatal_hook

		Address of a hook function to be called on a fatal error, if set.

	.. c:member:: u32_t fatal_hook_ssr

		Value of the System Status Register to be restored/reported when
		invoking the fatal error hook.

	.. c:member:: u32_t fatal_hook_gp

		Value of the global pointer to be restored when invoking the
		fatal error hook.

	.. c:member:: u32_t mask_for_ipi

		Mask of hardware threads which are valid targets for inter-processor
		interrupts (IPIs).

	.. c:member:: u32_t tlb_index

		Next TLB index to use when allocating a new TLB entry.

	.. c:member:: u32_t last_tlb_index

		Last TLB index available for allocation before wrapping back to
		the start of the replaceable range.

	.. c:member:: u32_t tlb_size

		Number of TLB entries available on this core.

	.. c:member:: u64_t pinned_tlb_mask

		Bitmask of TLB entries which are pinned and must not be replaced.

	.. c:member:: u32_t tlb_index_dma

		Next TLB index to use when allocating a new DMA TLB entry.  Only
		present when ``ARCHV >= 73``.

	.. c:member:: u32_t last_tlb_index_dma

		Last TLB index available for DMA TLB allocation before wrapping.
		Only present when ``ARCHV >= 73``.

	.. c:member:: u32_t tlb_size_dma

		Number of DMA TLB entries available on this core.  Only present
		when ``ARCHV >= 73``.

	.. c:member:: u32_t dma_tlb_start

		First TLB index reserved for DMA TLB entries.  Only present when
		``ARCHV >= 73``.

	.. c:member:: H2K_spinlock_t bkl

		Big Kernel Lock, used to serialize kernel execution across
		hardware threads.  Only present when ``SOFT_BKL`` is defined, otherwise hardware lock is used.

	.. c:member:: H2K_spinlock_t tmpmap_lock

		Spinlock protecting use of the temporary mapping used for
		short-lived physical address accesses.

	.. c:member:: H2K_spinlock_t asid_spinlock

		Spinlock protecting :c:member:`H2K_asid_table`.

	.. c:member:: u64_t oncpu_start[MAX_HTHREADS]

		Cycle count when the currently-running thread started
		execution, for each hardware thread.

	.. c:member:: u64_t oncpu_wait[MAX_HTHREADS]

		Cycle count when the currently-waiting thread entered wait
		mode, for each hardware thread.

	.. c:member:: u64_t waitcycles[MAX_HTHREADS]

		Total accumulated cycle count spent in wait mode, for each
		hardware thread.

	.. c:member:: u64_t prof_sample[MAX_HTHREADS]

		Most recent PC sample captured for each hardware thread.  Only
		present when ``DO_PROFILE`` is defined.

	.. c:member:: u32_t prof_sample_pending

		Bitmask of hardware threads with a pending profile sample to be
		processed.  Only present when ``DO_PROFILE`` is defined.

	.. c:member:: H2K_spinlock_t prof_sample_lock

		Spinlock protecting access to :c:member:`prof_sample` and
		:c:member:`prof_sample_pending`.  Only present when ``DO_PROFILE``
		is defined.

	.. c:member:: H2K_thread_context *runlist[MAX_HTHREADS]

		This array contains pointers to the threads running on each hardware
		thread.  If a hardware thread is in wait mode, its pointer should be NULL.

	.. c:member:: s16_t runlist_prios[(MAX_HTHREADS+7)/8*8]

		This array contains the priorities of threads running on each hardware
		thread.  If a hardware thread is in wait mode, its value should be -1.

	.. c:member:: H2K_vmblock_t *vmblocks[H2K_ID_MAX_VMS]

		Array of pointers to the vmblocks for each Virtual Machine known
		to the hypervisor.

	.. c:member:: u32_t phys_offset

		Physical offset applied when translating kernel virtual addresses.

	.. c:member:: u32_t build_id

		Build identifier of the running kernel.

	.. c:member:: info_boot_flags_type info_boot_flags

		Bitfield of boot-time capability flags (e.g. whether HVX, HMX,
		SILVER, HLX, DMA, or PC sampling are present, and whether the
		kernel is running from TCM).

	.. c:member:: info_stlb_type info_stlb

		Bitfield describing the configuration of the STLB, if one is
		present (number of sets, ways, size, and whether it is enabled).

	.. c:member:: kerror_type kernel_error

		Records the first fatal kernel error encountered, if any.

	.. c:member:: u32_t hthreads

		Number of hardware threads present on this core.

	.. c:member:: u32_t hthreads_mask

		Bitmask of hardware threads present on this core.

	.. c:member:: u32_t tcm_base

		Base address of TCM (Tightly Coupled Memory), in 4k pages.

	.. c:member:: u32_t tcm_size

		Size of TCM (Tightly Coupled Memory), in 4k pages.

	.. c:member:: u32_t l2size

		Size of the L2 cache/TCM array, computed from :c:member:`l2arr`.

	.. c:member:: u32_t l2tags

		Size of the L2 cache tag array.

	.. c:member:: u32_t vtcm_base

		Base address of VTCM (Vector Tightly Coupled Memory), in 4k pages.

	.. c:member:: u32_t vtcm_size

		Size of VTCM (Vector Tightly Coupled Memory), in 4k pages.

	.. c:member:: u32_t ecc_enable

		Flag indicating whether ECC (Error Correcting Code) protection is
		enabled.

	.. c:member:: u32_t dma_version

		Version of the user-mode DMA engine present on this core.  A value
		of 0 indicates no DMA engine is present.

	.. c:member:: u32_t core_id

		Multicore core ID of this core.

	.. c:member:: u32_t core_count

		Number of cores present in this multicore system.

	.. c:member:: u32_t multicore_shift

		Shift amount used to translate addresses for the boot VM in a
		multicore system.

	.. c:member:: u32_t tcm_offset

		Offset applied to the TCM base address in a multicore system.

	.. c:member:: u32_t noc_mbase

		Base address of the NOC (Network on Chip) master widget, in 4k
		pages.

	.. c:member:: u32_t noc_sbase

		Base address of the NOC (Network on Chip) slave widget, in 4k
		pages.

	.. c:member:: u32_t local_mem_base

		Base address of this core's local memory, in 4k pages.

	.. c:member:: u32_t local_mem_size

		Size of this core's local memory, in 4k pages.

	.. c:member:: H2K_spinlock_t logbuf_lock

		Spinlock protecting the log buffer.

	.. c:member:: char *logbuf[MAX_HTHREADS]

		Per-hardware-thread log buffer used for kernel logging.

	.. c:member:: u32_t logbuf_pos[MAX_HTHREADS]

		Current write position within each hardware thread's log buffer.

	.. c:member:: u32_t logbuf_enable

		Flag indicating whether logging to the log buffer is enabled.

	.. c:member:: u32_t log_enable

		Flag indicating whether kernel logging is enabled.

	.. c:member:: H2K_spinlock_t angel_lock

		Spinlock protecting Angel semihosting operations.

	.. c:member:: u64_t crash_tlb[MAX_TLB_ENTRIES]

		Snapshot of the TLB contents captured at the time of a crash.
		Only present when ``CRASH_DEBUG`` is defined.

	.. c:member:: u32_t crash_l2vic_enabled[MAX_INTERRUPTS/32]

		Snapshot of the L2VIC enabled-interrupt bitmask captured at the
		time of a crash.  Only present when ``CRASH_DEBUG`` is defined.

	.. c:member:: u32_t crash_l2vic_pending[MAX_INTERRUPTS/32]

		Snapshot of the L2VIC pending-interrupt bitmask captured at the
		time of a crash.  Only present when ``CRASH_DEBUG`` is defined.

	.. c:member:: H2K_thread_context crash_contexts[MAX_HTHREADS]

		Snapshot of the thread context running on each hardware thread,
		captured at the time of a crash.  Only present when
		``CRASH_DEBUG`` is defined.

	.. c:member:: void *inthandlers[MAX_INTERRUPTS]

		Handler functions for each interrupt

	.. c:member:: H2K_thread_context *futexhash[FUTEX_HASHSIZE]

		Hash table of threads waiting on futexes.

	.. c:member:: H2K_asid_entry_t asid_table[MAX_ASIDS]

		Table of ASID (Address Space IDentifier) entries used to track
		address spaces maintained by the hypervisor.

	.. c:member:: H2K_thread_context *ready[MAX_PRIOS]

		This array contains a pointer to a thread in a ring of ready threads at
		each priority.  If the ring is empty, the pointer should be NULL.

	.. c:member:: u32_t hlx_contexts

		Number of HLX coprocessor contexts available.

	.. c:member:: u32_t hlx_state

		Current power state of the HLX coprocessor (on/off).

	.. c:member:: H2K_mem_alloc_tag_t *alloc_heap

		Pointer to the base of the kernel's dynamic allocation heap.

	.. c:member:: u32_t alloc_heap_size

		Size, in bytes, of the kernel's dynamic allocation heap.

	.. c:member:: H2K_spinlock_t alloc_heap_lock

		Spinlock protecting the kernel's dynamic allocation heap.
H2K_kg_init
-----------

.. c:function:: void H2K_kg_init(u32_t phys_offset, u32_t multicore_shift, u32_t devpage_offset, u32_t last_tlb_index, u32_t tlb_size, u32_t core_id, u32_t core_count, u32_t tcm_offset)

	:param phys_offset: Physical address offset
	:param multicore_shift: Shift used for multicore address regions
	:param devpage_offset: Device-page offset
	:param last_tlb_index: Last regular TLB index
	:param tlb_size: Number of regular TLB entries
	:param core_id: Core identifier
	:param core_count: Number of cores
	:param tcm_offset: TCM address offset

Description
~~~~~~~~~~~

Initializes the global kernel data and discovers the processor and memory
configuration needed by the rest of the kernel.


H2K_cluster_config
------------------

.. c:function:: void H2K_cluster_config(void)

Description
~~~~~~~~~~~

Finalizes cluster-scheduler configuration when ``CLUSTER_SCHED`` is enabled.

Functionality
~~~~~~~~~~~~~

Builds masks for the hardware threads that are actually started and disables
cluster scheduling when a cluster cannot support the configured coprocessor
limit.


.. c:var:: H2K_kg_t H2K_kg

	Instantiation of the global data.

.. c:var:: register H2K_kg_t *H2K_gp

	We use a register to hold the address of the global data structure
	across function calls.  This needs to be set up on each entry into the
	kernel.

