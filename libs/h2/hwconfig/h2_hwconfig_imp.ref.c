/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_hwconfig_imp.ref.c
 * 
 * @brief Hardware Configuration Interface - Implementation
 */

#include "h2_hwconfig.h"

int h2_hwconfig_l2_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETL2REG, NULL, offset, 0);
}

int h2_hwconfig_l2_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETL2REG, NULL, offset, val);
}

int h2_hwconfig_clade_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETCLADEREG, NULL, offset, 0);
}

int h2_hwconfig_clade_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETCLADEREG, NULL, offset, val);
}

int h2_hwconfig_l2cache_size(unsigned int sizeval, unsigned int use_wb)
{
	return h2_hwconfig_trap(HWCONFIG_L2CACHE, NULL, sizeval, use_wb);
}

int h2_hwconfig_partition(unsigned int whichcache, unsigned int partition_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PARTITIONS, NULL, whichcache, partition_cfg);
}

int h2_hwconfig_prefetch(unsigned int whichcache, unsigned int prefetch_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_PREFETCH, NULL, whichcache, prefetch_cfg);
}

int h2_hwconfig_set_coprocbits(unsigned int coproc)
{
	return h2_hwconfig_trap(HWCONFIG_COPROCBITS, NULL, coproc, 0);
}

int h2_hwconfig_set_hmxbits(unsigned int xe2,  unsigned int xa2)
{
	return h2_hwconfig_trap(HWCONFIG_HMXBITS, NULL, xe2, xa2);
}

int h2_hwconfig_hmxbits(unsigned int xe2)
{
	return h2_hwconfig_set_hmxbits(xe2, 0);
}

int h2_hwconfig_set_hlxbits(unsigned int xa3,  unsigned int xe3)
{
	return h2_hwconfig_trap(HWCONFIG_HLXBITS, NULL, xa3, xe3);
}

int h2_hwconfig_extbits(unsigned int xa, unsigned int xe)
{
	return h2_hwconfig_trap(HWCONFIG_EXTBITS, NULL, xa, xe);
}

int h2_hwconfig_vlength(unsigned int vlength)
{
	return h2_hwconfig_trap(HWCONFIG_VLENGTH, NULL, vlength, 0);
}

int h2_hwconfig_extpower(unsigned int state)
{
	return h2_hwconfig_trap(HWCONFIG_EXTPOWER, NULL, state, 0);
}

int h2_hwconfig_l2locka(void *addr, unsigned int len)
{
	return h2_hwconfig_trap(HWCONFIG_L2LOCKA, addr, len, 0);
}

int h2_hwconfig_unlock()
{
	return h2_hwconfig_trap(HWCONFIG_L2UNLOCK, NULL, 0, 0);
}

int h2_hwconfig_hwintop(unsigned int op, unsigned int intno, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_HWINTOP, NULL, (op << 16) | intno, val);
}

int h2_hwconfig_hwthreads_mask(unsigned int mask) {

	return h2_hwconfig_trap(HWCONFIG_HWTHREADS_MASK, NULL, mask, 0);
}

int h2_hwconfig_hwthreads_num(unsigned int num) {

	return h2_hwconfig_trap(HWCONFIG_HWTHREADS_NUM, NULL, num, 0);
}

int h2_hwconfig_ecc(unsigned int ecc_enable) {

	return h2_hwconfig_trap(HWCONFIG_ECC, NULL, ecc_enable, 0);
}

int h2_hwconfig_dma_getcfg(unsigned int index) {

	return h2_hwconfig_trap(HWCONFIG_GETDMACFG, NULL, index, 0);
}

int h2_hwconfig_dma_setcfg(unsigned int index, unsigned int data) {

	return h2_hwconfig_trap(HWCONFIG_SETDMACFG, NULL, index, data);
}

int h2_hwconfig_l2gclean(unsigned int inv) {

	return h2_hwconfig_trap(HWCONFIG_L2GCLEAN, NULL, inv, 0);
}

int h2_hwconfig_strideprefetcher_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETSTRIDEPREFETCHERREG, NULL, offset, 0);
}

int h2_hwconfig_strideprefetcher_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SETSTRIDEPREFETCHERREG, NULL, offset, val);
}

int h2_hwconfig_hmxpower_on_set_addr(unsigned int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETHMXPOWERONSTARTADDR, NULL, addr, 0);
}

int h2_hwconfig_hmxpower_off_set_addr(unsigned int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETHMXPOWEROFFSTARTADDR, NULL, addr, 0);
}

int h2_hwconfig_gpio_toggle(unsigned int val) {
	return h2_hwconfig_trap(HWCONFIG_GPIOTOGGLE, NULL, val, 0);
}

int h2_hwconfig_set_gpio_addr(unsigned long long int addr) {
	return h2_hwconfig_trap(HWCONFIG_SETGPIOADDR, NULL, addr >> 32, addr & 0xffffffff);
}

int h2_hwconfig_l2cp(unsigned int l2cp_cfg)
{
	return h2_hwconfig_trap(HWCONFIG_L2CP, NULL, l2cp_cfg, 0);
}

int h2_hwconfig_ecc_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GETECCREG, NULL, offset, 0);
}

int h2_hwconfig_vwctrl_get(void) {

	return h2_hwconfig_trap(HWCONFIG_GETVWCTRL, NULL, 0, 0);
}

int h2_hwconfig_vwctrl_set(unsigned int val) {

	return h2_hwconfig_trap(HWCONFIG_SETVWCTRL, NULL, val, 0);
}

int h2_hwconfig_dpm_voltlmtmgmt_get_reg(unsigned int offset)
{
	return h2_hwconfig_trap(HWCONFIG_GET_DPM_VOLTLMTMGMT_REG, NULL, offset, 0);
}

int h2_hwconfig_dpm_voltlmtmgmt_set_reg(unsigned int offset, unsigned int val)
{
	return h2_hwconfig_trap(HWCONFIG_SET_DPM_VOLTLMTMGMT_REG, NULL, offset, val);
}
