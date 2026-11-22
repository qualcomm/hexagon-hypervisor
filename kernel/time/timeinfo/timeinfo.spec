:mod:`timeinfo` -- Timer state
================================

.. module:: timeinfo

H2K_timeinfo_t
--------------

.. c:type:: H2K_timeinfo_t

	Per-kernel timer state, aligned to 32 bytes.

	.. c:member:: ticks_t next_ticks

		Hardware tick value programmed for the next timer interrupt.

	.. c:member:: ticks_t last_ticks

		Tick value recorded at the last time update.

	.. c:member:: u64_t last_pcycles

		Processor-cycle value recorded at the last time update.

	.. c:member:: H2K_treenode_t *timeouts

		Root of the tree containing active thread timeouts.

	.. c:member:: volatile u32_t *devptr

		Pointer to the timer device registers.

The time interface uses ``nsec_t`` and ``ticks_t``, both represented as
``u64_t``.  ``H2K_TIME_FOREVER`` disables a timeout, and
``H2K_TIME_BIGBANG`` represents an expired or inactive timeout.
