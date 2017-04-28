
:mod:`timeinfo` -- Time Information definition
==============================================

.. module:: timeinfo

This module contains the definition of the information required to keep track of time.

H2K_timeinfo_t
--------------

Description
~~~~~~~~~~~

.. c:type:: H2K_timeinfo_t

	Time Information Block

	.. c:member:: H2K_spinlock_t lock

		spinlock for atomic accesses to the time information

	.. c:member:: H2K_treenode_t *timeouts

		Tree of all threads requesting a timeout

	.. c:member:: u64_t last_ticks

		Last update in ticks

	.. c:member:: u64_t last_pcycles

		Processor Cycle Count of the last time update

	.. c:member:: u64_t next_ticks

		Hardware counter information for the next interrupt

	.. c:member:: volatile u32_t *devptr

		Pointer to the hardware timer device

