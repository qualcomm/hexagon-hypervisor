:mod:`hwconfig` -- Runtime hardware configuration
=================================================

.. module:: hwconfig

H2K_trap_hwconfig
-----------------

.. c:function:: u32_t H2K_trap_hwconfig(hwconfig_type_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me)

	:param configtype: Hardware-configuration operation
	:param ptr: Operation-specific pointer or value
	:param val2: Operation-specific value
	:param val3: Operation-specific value
	:param me: Current thread context
	:returns: Operation result, or -1 for an invalid operation

Dispatches the request through the hardware-configuration table.


H2K_trap_hwconfig_l2cache
-------------------------

.. c:function:: u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me)

	:param size: L2 size configuration
	:param use_wb: Whether to enable write-back mode
	:returns: 0 on success, -1 if single-thread mode cannot be entered

Reconfigures the L2 cache in single-thread mode.  It cleans and invalidates the
cache, applies the requested size and write-back setting, updates ECC settings,
and records the resulting L2 and TCM configuration.


H2K_trap_hwconfig_partitions
----------------------------

.. c:function:: u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)

	:param whatcache: Cache selected for partitioning
	:param configval: Partition configuration
	:returns: 0 on success, -1 for an invalid cache selector

Updates the selected cache-partition field in ``SYSCFG`` under the BKL.


H2K_trap_hwconfig_prefetch
--------------------------

.. c:function:: u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me)

	:param whatcache: Prefetch field to configure
	:param configval: New one-bit value
	:returns: 0

Updates the selected instruction or data prefetch field in the current
context's CCR.  The supported selectors cover L1 instruction/data and L2
instruction/data prefetch controls.


H2K_trap_hwconfig_extbits
-------------------------

.. c:function:: u32_t H2K_trap_hwconfig_extbits(u32_t unused, void *unusedp, u32_t xa, u32_t xe, H2K_thread_context *me)

	:param xa: Extended-register bank selector
	:param xe: Extended-execution enable
	:returns: 0

Updates the current context's XA and XE fields, subject to extension-context
management.  With cluster scheduling enabled, changing coprocessor use may
move the thread through the ready list and scheduler.


H2K_trap_hwconfig_vlength
-------------------------

.. c:function:: u32_t H2K_trap_hwconfig_vlength(u32_t unused, void *unusedp, u32_t vlength, u32_t unused3, H2K_thread_context *me)

	:param vlength: Requested vector-length encoding
	:returns: 0 when extensions are supported, otherwise -1

Updates the V2X system-configuration bit according to the requested vector
length and refreshes extension-switch state when enabled.


H2K_trap_hwconfig_extpower
--------------------------

.. c:function:: u32_t H2K_trap_hwconfig_extpower(u32_t unused, void *unusedp, u32_t state, u32_t unused3, H2K_thread_context *me)

	:param state: Nonzero to power on extensions, zero to power them off
	:returns: 0 when extensions are supported, otherwise -1

Powers the HVX, HLX, and HMX extensions on or off.


H2K_trap_hwconfig_getl2reg / H2K_trap_hwconfig_setl2reg
--------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_getl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_setl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)

	:param offset: L2 configuration-register offset
	:param val: Value for the set operation
	:returns: Register value, or an error result for an out-of-range offset

Read or write an L2 configuration register through the configured L2 register
window.  Invalid offsets record the corresponding kernel error.


H2K_trap_hwconfig_l2locka / H2K_trap_hwconfig_l2unlock
-------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_l2locka(u32_t unused, void *addr, u32_t len, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_l2unlock(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me)

	:param addr: User address range to lock
	:param len: Length of the range
	:returns: 0 on success, -1 when unsupported or unsuccessful

Lock or unlock L2 cache lines.  The lock operation validates the address with
safe memory access and locks each cache-line-sized portion of the range.


H2K_trap_hwconfig_hwintop
-------------------------

.. c:function:: u32_t H2K_trap_hwconfig_hwintop(u32_t unused, void *unusedp, u32_t op_and_int, u32_t val, H2K_thread_context *me)

	:param op_and_int: Operation in the upper 16 bits and interrupt number in the lower 16 bits
	:returns: 0 on success, 1 for an invalid operation or interrupt

Enables, disables, or raises the selected hardware interrupt.


H2K_trap_hwconfig_getcladereg / H2K_trap_hwconfig_setcladereg
--------------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_getcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_setcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)

	:param offset: CLADE register offset
	:param val: Value for the set operation

Read or write CLADE registers, translating the exposed field positions to the
hardware register layout.  Out-of-range offsets record a kernel error.


H2K_trap_hwconfig_hwthreads_num / H2K_trap_hwconfig_hwthreads_mask
------------------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_hwthreads_num(u32_t unused, void *unusedp, u32_t num, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_hwthreads_mask(u32_t unused, void *unusedp, u32_t mask, u32_t unused3, H2K_thread_context *me)

	:param num: Requested number of hardware threads
	:param mask: Requested hardware-thread mask
	:returns: Resulting thread count or mask

Starts the requested hardware threads, updates the global hardware-thread
count and mask, and refreshes cluster configuration when enabled.  The number
form selects the first available threads required to satisfy ``num``; the mask
form selects specific threads and always retains thread zero.


H2K_trap_hwconfig_ecc
---------------------

.. c:function:: u32_t H2K_trap_hwconfig_ecc(u32_t unused, void *unusedp, u32_t ecc_enable, u32_t unused3, H2K_thread_context *me)

	:param ecc_enable: ECC register-enable mask
	:returns: 0 on success, -1 for an out-of-range mask

Updates ECC configuration by reconfiguring the L2 cache while preserving its
current size and write-back mode.


H2K_trap_hwconfig_hmxbits / H2K_trap_hwconfig_hlxbits
------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_hmxbits(u32_t unused, void *unusedp, u32_t xe2, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_hlxbits(u32_t unused, void *unusedp, u32_t xa3, u32_t xe3, H2K_thread_context *me)

	:param xe2: HMX execution-enable value
	:param xa3: HLX register-bank selector
	:param xe3: HLX execution-enable value
	:returns: 0 when the extension exists, otherwise -1

Updates HMX or HLX execution state.  When cluster scheduling is active,
transitions that change coprocessor use reschedule the current thread so
cluster accounting remains consistent.


H2K_trap_hwconfig_coproc_bits
-----------------------------

.. c:function:: u32_t H2K_trap_hwconfig_coproc_bits(u32_t unused, void *unusedp, u32_t coproc, u32_t unused3, H2K_thread_context *me)

	:param coproc: Coprocessor-enable value
	:returns: 0

Updates the coprocessor bit in the current context's CCR.


H2K_trap_hwconfig_getdmacfg / H2K_trap_hwconfig_setdmacfg
----------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_getdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_setdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t data, H2K_thread_context *me)

	:param index: DMA configuration index
	:param data: Value for the set operation
	:returns: DMA register value or 0; -1 when DMA is unavailable

Read or write DMA configuration when supported by the architecture and DMA
hardware.


H2K_trap_hwconfig_l2gclean
--------------------------

.. c:function:: u32_t H2K_trap_hwconfig_l2gclean(u32_t unused, void *unusedp, u32_t inv, u32_t unused3, H2K_thread_context *me)

	:param inv: Nonzero to clean and invalidate, zero to clean only
	:returns: 0 when supported, otherwise -1

Synchronizes and globally cleans the L2 cache, optionally invalidating it too.


H2K_trap_hwconfig_getstrideprefetcherreg / H2K_trap_hwconfig_setstrideprefetcherreg
-------------------------------------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_getstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_setstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)

	:param offset: Stride-prefetcher register offset
	:param val: Value for the set operation

Read or write stride-prefetcher registers when the hardware reports that the
feature is present and the offset is within the per-thread range.


H2K_trap_hwconfig_set_hmx_power_on_start_addr / H2K_trap_hwconfig_set_hmx_power_off_start_addr
-----------------------------------------------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_set_hmx_power_on_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_set_hmx_power_off_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me)

	:param addr: HMX power-sequence start address
	:returns: 0 when supported, otherwise -1

Stores the HMX power-on or power-off resource-sequence start address.


H2K_trap_hwconfig_gpio_toggle
-----------------------------

.. c:function:: u32_t H2K_trap_hwconfig_gpio_toggle(u32_t unused, void *unusedp, u32_t on, u32_t unused3, H2K_thread_context *me)

	:param on: Nonzero to turn GPIO output on, zero to turn it off
	:returns: 0 when supported and configured, otherwise -1

Toggles the configured GPIO output.


H2K_trap_hwconfig_set_gpio_addr
-------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_set_gpio_addr(u32_t unused, void *unusedp, u32_t addr_hi, u32_t addr_lo, H2K_thread_context *me)

	:param addr_hi: High physical-address word
	:param addr_lo: Low physical-address word
	:returns: TLB allocation result, or -1 when unsupported

Stores the 64-bit GPIO physical address and allocates a global, monitor-only
TLB mapping for the GPIO page.


H2K_trap_hwconfig_l2cp
----------------------

.. c:function:: u32_t H2K_trap_hwconfig_l2cp(u32_t unused, void *unusedp, u32_t configval, u32_t unused3, H2K_thread_context *me)

	:param configval: L2 partition-by-thread mode
	:returns: 0

Updates the L2 partition mode in the current context's CCR.


H2K_trap_hwconfig_geteccreg
---------------------------

.. c:function:: u32_t H2K_trap_hwconfig_geteccreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)

	:param offset: ECC register offset
	:returns: ECC register value, or -1 for an invalid offset

Reads an ECC configuration register and records an error for an out-of-range
offset.


H2K_trap_hwconfig_getvwctrl / H2K_trap_hwconfig_setvwctrl
----------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_getvwctrl(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_setvwctrl(u32_t unused, void *unusedp, u32_t val, u32_t unused3, H2K_thread_context *me)

	:param val: VWCTRL value for the set operation
	:returns: Register value or 0; -1 when unsupported

Read or write VWCTRL on supported V73+ processor revisions.  The set operation
also updates the current thread's saved VWCTRL value.


H2K_trap_hwconfig_get_dpm_voltlimitmgmt_reg / H2K_trap_hwconfig_set_dpm_voltlimitmgmt_reg
---------------------------------------------------------------------------------------------

.. c:function:: u32_t H2K_trap_hwconfig_get_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)

.. c:function:: u32_t H2K_trap_hwconfig_set_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)

	:param offset: DPM voltage-limit-management register offset
	:param val: Value for the set operation
	:returns: Register value, or -1 when unsupported or out of range

Read or write DPM voltage-limit-management registers when supported by the
processor and present in the core-configuration table.
