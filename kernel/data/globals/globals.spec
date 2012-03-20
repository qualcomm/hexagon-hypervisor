

:mod:`globals` -- Global Data Structures
========================================

.. module:: globals


.. ctype:: H2K_kg_t

	Kernel Globals kept together for ease of referencing.

	.. cmember:: u64_t ready_valids[MAX_PRIOS/64]

		This is a bitfield.  Bit 0 corresponds to priority 0, bit 1 to priority 1, 
		and so on.  If the bit *n* is set, H2K_ready[*n*] must be valid.  If bit 
		*n* is clear, H2K_ready[*n*] must be NULL.

	.. cmember:: u32_t priomask

		A mask for which thread or threads are marked as the lowest priority.  
		These threads will be interruptible.  Like :cmember:`wait_mask`, 
		Hardware thread *n* corresponds to bit *n* in the mask.  If a bit 
		is set in :cmember:`wait_mask`, it should also be set in 
		:cmember:`priomask`.

	.. cmember:: u32_t wait_mask

		A mask for which threads are in wait mode.  Hardware thread 0
		corresponds to bit 0, hardware thread 1 to bit 1, and so on.

	.. cmember:: u32_t fastint_gp

		Global value for the GP value to use during fast interurpts.

	.. cmember:: u64_t oncpu_start[MAX_HTHREADS]

		Cycle count when the currently-running thread started
		execution, for each hardware thread.

	.. cmember:: u64_t oncpu_wait[MAX_HTHREADS]

		Cycle count when the currently-waiting thread entered wait
		mode, for each hardware thread.

	.. cmember:: u64_t waitcycles[MAX_HTHREADS]

		Total accumulated cycle count spent in wait mode, for each
		hardware thread.

	.. cmember:: H2K_trace_info_t trace_info

		Information about the kernel trace infrastructure

	.. cmember:: H2K_thread_context *runlist[MAX_HTHREADS]

		This array contains pointers to the threads running on each hardware
		thread.  If a hardware thread is in wait mode, its pointer should be NULL.

	.. cmember:: s16_t runlist_prios[(MAX_HTHREADS+7)/8*8]

		This array contains the priorities of threads running on each hardware
		thread.  If a hardware thread is in wait mode, its value should be -1.

	.. cmember:: H2K_thread_context *ready[MAX_PRIOS]

		This array contains a pointer to a thread in a ring of ready threads at
		each priority.  If the ring is empty, the pointer should be NULL.

	.. cmember:: void *fastint_funcptrs[MAX_INTERRUPTS]

		Pointers to the fast interrupt handlers

	.. cmember:: H2K_thread_context *futexhash[FUTEX_HASHSIZE]

		Hash table of threads waiting on futexes.

	.. cmember:: void *inthandlers[MAX_INTERRUPTS]

		Handler functions for each interrupt

.. cvar:: H2K_kg_t H2K_kg

	Instantiation of the global data.

.. cvar:: register H2K_kg_t *H2K_gp

	We use a register to hold the address of the global data structure
	across function calls.  This needs to be set up on each entry into the
	kernel.

