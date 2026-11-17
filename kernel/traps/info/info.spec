:mod:`info` -- Query system configuration
==========================================

.. module:: info

H2K_trap_info
-------------

.. c:function:: u32_t H2K_trap_info(info_type op, u32_t unit, h2_cfg_unit_entry entry, H2K_thread_context *me)

	:param op: Information query
	:param unit: Unit index, used by ``INFO_UNIT_ENTRY``
	:param entry: Unit configuration entry, used by ``INFO_UNIT_ENTRY``
	:param me: Current thread context, used by ``INFO_HVX_SWITCH``
	:returns: Requested value, or -1 for an unsupported query

Returns kernel, processor, memory, timer, cache, extension, and multicore
configuration values selected by ``op``.

Queries include:

* build ID, boot flags, STLB information, ``SYSCFG``, live-lock state, and core
  revision;
* subsystem, L2 interrupt-controller, timer, L2 configuration, CLADE, and
  configuration-register bases;
* free TLB count, TLB size, kernel physical address, TCM/VTCM information,
  page size, kernel page count, timer interrupt, and kernel error;
* hardware-thread mask, L2 tag size, HVX vector length, coprocessor counts,
  HMX instances, and HLX contexts;
* core ID, core count, multicore shift, TCM offset, NoC bases, and local-memory
  base and size.

Architecture-dependent queries return zero or -1 when the relevant hardware is
not available.  ``INFO_UNIT_ENTRY`` uses ``unit`` and ``entry`` to query the
selected unit configuration.  ``INFO_HVX_SWITCH`` reports the current VM's
extension-context switching state when ``DO_EXT_SWITCH`` is enabled, otherwise
zero.
