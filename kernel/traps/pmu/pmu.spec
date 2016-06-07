
:mod:`pmu` -- Performance Monitor Unit Management
=================================================

.. module:: pmu


This module contains routines to manage the Performance Monitor Unit.

H2K_trap_pmuctrl
------------------

.. cfunction:: u32_t H2K_trap_pmuctrl(u32_t configtype, u32_t val1, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: Configuration operation to do
	:param val1: Operation-dependent value
	:param val2: Operation-dependent value
	:param val3: Operation-dependent value
	:param me: Pointer to the current thread context
	:returns: The return code for the configuration option.


Description
~~~~~~~~~~~

Trap handler for PMU configuration.

Functionality
~~~~~~~~~~~~~

The configtype parameter determines which configuration routine is called.
If an invalid configtype parameter is given, we return imediately.

We look up which configuration routine in H2K_pmuctrltab, and jump to the 
appropriate routine.


REMOVED: H2K_trap_pmuctrl_threadset
-----------------------------------

.. cfunction:: u32_t H2K_trap_pmuctrl_threadset(u32_t unused, u32_t vdest, u32_t turnon, u32_t unused3, H2K_thread_context *me)

	:param unused: Unused paramater
	:param vdest: thread ID to change PMU configuration for
	:param turnon: 0 to disable PMU monitoring, 1 to enable
	:param unused3: Unused parameter
	:param me: Pointer to the current thread context
	:returns: zero on success, nonzero otherwise

Description
~~~~~~~~~~~

Enables or disables performance monitoring for a software thread

Functionality
~~~~~~~~~~~~~

Acquire the BKL.

If the thread is dead or invalid, return nonzero.

Set the pmu_on field in dest according to the parameter.  This must be done
atomically, as pmu_on is in the atomic word.  

If dest is RUNNING mode, we set or clear the PMUCFG register according to 
the parameter.

We then release the BKL and return.


H2K_trap_pmuctrl_setreg
-------------------------

.. cfunction:: u32_t H2K_trap_pmuconcig_setreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t newval, H2K_thread_context *me)

	:param unused: Unused paramater
	:param unused2: Unused parameter
	:param whichreg: 0-3 :PMUCNT0-3; -1: PMUEVTCFG; -2: TLBMISSX_LO; -3: TLBMISSX_HI; -4: TLBMISSRW_LO; -5: TLBMISSRW_HI; -6: STLBMISS_LO; -7: STLBMISS_HI.
	:param newval: new value for the register
	:param me: Pointer to the current thread context
	:returns: zero on success, nonzero otherwise

Description
~~~~~~~~~~~

Sets a PMU register

Functionality
~~~~~~~~~~~~~

Write the PMU register

H2K_trap_pmuctrl_getreg
-------------------------

.. cfunction:: u32_t H2K_trap_pmuconcig_getreg(u32_t unused, u32_t unused2, u32_t whichreg, u32_t unused3, H2K_thread_context *me)

	:param unused: Unused paramater
	:param unused2: Unused parameter
	:param whichreg: 0-3 :PMUCNT0-3; -1: PMUEVTCFG; -2: TLBMISSX_LO; -3: TLBMISSX_HI; -4: TLBMISSRW_LO; -5: TLBMISSRW_HI; -6: STLBMISS_LO; -7: STLBMISS_HI.
	:param unused3: Unused parameter
	:param me: Pointer to the current thread context
	:returns: zero on success, nonzero otherwise

Description
~~~~~~~~~~~

Reads a PMU register

Functionality
~~~~~~~~~~~~~

Read the PMU register




Testing
-------

Samples
~~~~~~~

* Input: various registers
* Input: setting self / other threads

Important Cases
~~~~~~~~~~~~~~~

* Invalid register

Harness
~~~~~~~

Standalone environment, but linked with h2kernel.

We try several calls to H2K_trap_pmuctrl, and check that the registers are read/written
correctly, or that the PMU enable field is managed correctly in the destination thread.








