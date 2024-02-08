/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_HWCONFIG_H
#define H2K_HWCONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <h2_common_hwconfig.h>
#include <max.h>

u32_t H2K_trap_hwconfig(hwconfig_type_t configtype, void *ptr, u32_t val2, u32_t val3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_getl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_setl2reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_getcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_setcladereg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_l2cache(u32_t unused, void *unusedp, u32_t size, u32_t use_wb, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_partitions(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_prefetch(u32_t unused, void *unusedp, u32_t whatcache, u32_t configval, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_extbits(u32_t unused, void *unusedp, u32_t xa, u32_t xe, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_vlength(u32_t unused, void *unusedp, u32_t vlength, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_extpower(u32_t unused, void *unusedp, u32_t state, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_l2locka(u32_t unused, void *addr, u32_t len, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_l2unlock(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_hwintop(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_hwthreads_mask(u32_t unused, void *unusedp, u32_t mask, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_hwthreads_num(u32_t unused, void *unusedp, u32_t num, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_ecc(u32_t unused, void *unusedp, u32_t ecc_enable, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_hmxbits(u32_t unused, void *unusedp, u32_t xe2, u32_t xa2, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_hlxbits(u32_t unused, void *unusedp, u32_t xa3, u32_t xe3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_getdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_setdmacfg(u32_t unused, void *unusedp, u32_t index, u32_t data, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_l2gclean(u32_t unused, void *unusedp, u32_t inv, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_getstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_setstrideprefetcherreg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_set_hmx_power_on_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_set_hmx_power_off_start_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_gpio_toggle(u32_t unused, void *unusedp, u32_t on, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_set_gpio_addr(u32_t unused, void *unusedp, u32_t addr, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_l2cp(u32_t unused, void *unusedp, u32_t configval, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_geteccreg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_getvwctrl(u32_t unused, void *unusedp, u32_t unused2, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_setvwctrl(u32_t unused, void *unusedp, u32_t val, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_get_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t unused3, H2K_thread_context *me)IN_SECTION(".text.config.hwconfig");
u32_t H2K_trap_hwconfig_set_dpm_voltlimitmgmt_reg(u32_t unused, void *unusedp, u32_t offset, u32_t val, H2K_thread_context *me)IN_SECTION(".text.config.hwconfig"); 

#endif

