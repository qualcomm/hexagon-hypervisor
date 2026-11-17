:mod:`pmu` -- Performance Monitor Unit access
===============================================

.. module:: pmu

H2K_trap_pmuctrl
----------------

.. c:function:: u32_t H2K_trap_pmuctrl(pmuop_type configtype, u32_t val1, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: PMU operation
	:param val1: Register selector or operation value
	:param val2: Operation value
	:param val3: Operation value
	:param me: Current thread context
	:returns: 0 on success, -1 for an unsupported operation or register

Dispatches PMU register reads and writes.  ``PMUCTRL_THREADSET`` is retained
as a removed operation and returns -1.


H2K_trap_pmuctrl_setreg
-----------------------

.. c:function:: u32_t H2K_trap_pmuctrl_setreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t newval, H2K_thread_context *me)

	:param whichreg: PMU or VM TLB-event register selector
	:param newval: Value to write
	:param me: Current thread context
	:returns: 0 on success, -1 for an unsupported selector

Writes PMU registers 0 through 7, event/configuration registers 8 through 12,
and, when ``COUNT_TLB_EVENTS`` is enabled, the current VM's low and high words
for execute-TLB, read/write-TLB, and STLB miss counters using selectors -2
through -7.


H2K_trap_pmuctrl_getreg
-----------------------

.. c:function:: u32_t H2K_trap_pmuctrl_getreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t unused3, H2K_thread_context *me)

	:param whichreg: PMU or VM TLB-event register selector
	:param me: Current thread context
	:returns: Selected register value, or -1 for an unsupported selector

Reads the same PMU and optional VM TLB-event registers supported by
:c:func:`H2K_trap_pmuctrl_setreg()`.
