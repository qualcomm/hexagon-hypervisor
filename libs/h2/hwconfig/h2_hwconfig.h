/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_HWCONFIG_H
#define H2_HWCONFIG_H 1

#include <stdlib.h>

/** @file h2_hwconfig.h
 @brief Hardware Configuration Interface
*/
/** @addtogroup h2 
@{ */

#include <h2_common_hwconfig.h>

/**
Raw interface for the hwconfig trap.  Do not use.  Instead, use an interface function such
as h2_hwconfig_partition().
@param[in] whichtrap	Which hardware configuration trap to use
@param[in] ptr		Address argument for trap
@param[in] a		First argument
@param[in] b		Second argument
@returns 0 on success, negative value on error
@dependencies None
*/

int h2_hwconfig_trap(hwconfig_type_t whichtrap, void *ptr, unsigned int a, unsigned int b);

/**
Get L2 register.
@param[in] offset  Offset from L2 register base
@returns register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_l2_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETL2REG, NULL, offset, 0);
}

/**
Set L2 register.
@param[in] offset  Offset from L2 register base
@param[in] val  Value to write
@returns previous register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_l2_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETL2REG, NULL, offset, val);
}

/**
Get CLADE register.
@param[in] offset  Offset from CLADE register base
@returns register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_clade_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETCLADEREG, NULL, offset, 0);
}

/**
Set CLADE register.
@param[in] offset  Offset from CLADE register base
@param[in] val  Value to write
@returns previous register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_clade_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETCLADEREG, NULL, offset, val);
}

/**
Configure the L2 cache size.
@param[in] sizeval	Size to configure the L2 cache
@param[in] use_wb	1 enables write-back mode in the L2, 0 disables write-back mode
@returns 0 on success, negative value on error
@dependencies May return failure if more than one thread is currently running
*/

static inline int h2_hwconfig_l2cache_size(unsigned int sizeval, unsigned int use_wb)
{
	return h2_hwconfig_trap(HWCONFIG_L2CACHE, NULL, sizeval, use_wb);
}

/**
Configure partitioning.
@param[in] whichcache	Select the cache to change partitioning on
@param[in] partition_cfg	What type of partitioning for the specified cache
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_partition(unsigned int whichcache, unsigned int partition_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PARTITIONS, NULL, whichcache, partition_cfg);
}

/**
Configure prefetch.
@param[in] whichcache	Select the cache to change prefetch on
@param[in] prefetch_cfg Prefetch type
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_prefetch(unsigned int whichcache, unsigned int prefetch_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PREFETCH, NULL, whichcache, prefetch_cfg);
}

/**
Set XE2 bit.
@param[in] xe2 XE2 value
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_hmxbits(unsigned int xe2)
{
	return h2_hwconfig_trap(HWCONFIG_HMXBITS, NULL, xe2, 0);
}

/**
Set XA, XE bits.
@param[in] xa XA value
@param[in] xe XE value
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_extbits(unsigned int xa, unsigned int xe)
{
	return h2_hwconfig_trap(HWCONFIG_EXTBITS, NULL, xa, xe);
}

/**
Set vector length
@param[in] vlength Vector length log 2
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_vlength(unsigned int vlength)
{
	return h2_hwconfig_trap(HWCONFIG_VLENGTH, NULL, vlength, 0);
}

/**
Control extension power state
@param[in] state Power state (zero == off, nonzero == on)
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_extpower(unsigned int state)
{
	return h2_hwconfig_trap(HWCONFIG_EXTPOWER, NULL, state, 0);
}

/**
L2 lock address range
@param[in] Virtual Address, should be aligned to 64 bytes
@param[in] Length in bytes, should be multiple of 64 bytes
@returns 0 on success, nonzero value on error
@dependencies None
*/

static inline int h2_hwconfig_l2locka(void *addr, unsigned int len)
{
	return h2_hwconfig_trap(HWCONFIG_L2LOCKA, addr, len, 0);
}

/**
L2 unlock everything
@returns 0 on success, nonzero value on error
@dependencies None
*/

static inline int h2_hwconfig_unlock()
{
	return h2_hwconfig_trap(HWCONFIG_L2UNLOCK, NULL, 0, 0);
}

/**
Control interrupt hardware directly
@returns 0 on success, nonzero value on error
@dependencies None
*/

static inline int h2_hwconfig_hwintop(unsigned int op, unsigned int intno, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_HWINTOP, NULL, (op << 16) | intno, val);
}

/**
Start hardware threads with mask (-1 to start all)
@param[in] mask Mask of thread numbers to start
@returns mask of threads started on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_hwthreads_mask(unsigned int mask) {

	return h2_hwconfig_trap(HWCONFIG_HWTHREADS_MASK, NULL, mask, 0);
}

/**
Start number of hardware threads
@param[in] num Number of threads to start
@returns number of threads started on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_hwthreads_num(unsigned int num) {

	return h2_hwconfig_trap(HWCONFIG_HWTHREADS_NUM, NULL, num, 0);
}

/**
Enable/Disable memory ECC
@param[in] ecc_enable 1 == enable, 0 == disable
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_ecc(unsigned int ecc_enable) {

	return h2_hwconfig_trap(HWCONFIG_ECC, NULL, ecc_enable, 0);
}

/**
Read DMA configuration register
@param[in] index  Index of register to read
@returns Register value on success, negative value on error
@dependencies none
*/
static inline int h2_hwconfig_dma_getcfg(unsigned int index) {

	return h2_hwconfig_trap(HWCONFIG_GETDMACFG, NULL, index, 0);
}

/**
Write DMA configuration register
@param[in] index  Index of register to write
@param[in] data  Data value to write
@returns 0 on success, negative value on error
@dependencies none
*/
static inline int h2_hwconfig_dma_setcfg(unsigned int index, unsigned int data) {

	return h2_hwconfig_trap(HWCONFIG_SETDMACFG, NULL, index, data);
}

/**
Clean L2 cache
@param[in] inv  Invalidate (zero == false, nonzero == true)
@returns 0 on success, negative value on error
@dependencies none
*/
static inline int h2_hwconfig_l2gclean(unsigned int inv) {

	return h2_hwconfig_trap(HWCONFIG_L2GCLEAN, NULL, inv, 0);
}

/**
Get stride prefetcher register.
@param[in] offset  Offset from stride prefetcher register base
@returns register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_strideprefetcher_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETSTRIDEPREFETCHERREG, NULL, offset, 0);
}

/**
Set stride prefetcher register.
@param[in] offset  Offset from stride prefetcher register base
@param[in] val  Value to write
@returns previous register value or 0 on failure -- check kerror
@dependencies Returns failure if offset out of range
*/

static inline int h2_hwconfig_strideprefetcher_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETSTRIDEPREFETCHERREG, NULL, offset, val);
}

/**
Set HMX RSC sequence power-on start address
@param[in] addr  Address
@returns 0 on success, negative value on error
@dependencies none
*/

static inline int h2_hwconfig_hmxpower_on_set_addr(unsigned int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETHMXPOWERONSTARTADDR, NULL, addr, 0);
}

/**
Set HMX RSC sequence power-off start address
@param[in] addr  Address
@returns 0 on success, negative value on error
@dependencies none
*/

static inline int h2_hwconfig_hmxpower_off_set_addr(unsigned int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETHMXPOWEROFFSTARTADDR, NULL, addr, 0);
}

/**
Toggle power-measurement GPIO
@param[in] val  Value (1 == on, 0 == off)
@returns 0 on success, negative value on error
@dependencies none
*/

static inline int h2_hwconfig_gpio_toggle(unsigned int val) {
	return h2_hwconfig_trap(HWCONFIG_GPIOTOGGLE, NULL, val, 0);
}

/**
Set power-measurement GPIO physical address
@param[in] addr  Address
@returns 0 on success, negative value on error
@dependencies none
*/

static inline int h2_hwconfig_set_gpio_addr(unsigned int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETGPIOADDR, NULL, addr, 0);
}

/**
Configure per-thread cache partition use
@param[in] l2cp_cfg Partition selection
@returns 0 on success, negative value on error
@dependencies None
*/

static inline int h2_hwconfig_l2cp(unsigned int l2cp_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_L2CP, NULL, l2cp_cfg, 0);
}

#endif

/** @} */
