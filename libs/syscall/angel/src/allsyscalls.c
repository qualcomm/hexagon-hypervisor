/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <angel.h>
#include <string.h>
#include <stdlib.h>
#include <syscall_defs.h>
#include <h2_thread.h>

//#define DEBUG_SYSTEM_CALLS_ERROR_PRINTS

#ifdef DEBUG_SYSTEM_CALLS_ERROR_PRINTS
#include <stdio.h>
#endif

#define ALIGN32_DOWN(X) ((X) & -32)
#define ALIGN32_UP(X) (((X) + 31) & -32)

long __boot_net_phys_offset__;

unsigned int angel(unsigned int r0, void *r1, unsigned int r2) {
	return __angel(r0, ANGEL_OFFSET_PTR(r1), r2);
}

__attribute__((noinline))
void break_on_system_call_error(unsigned int r0, const void *r1, unsigned int r2, sys_call_ret_t ret_val) {
#ifdef DEBUG_SYSTEM_CALLS_ERROR_PRINTS
	printf("System call complete  r0: 0x%08x, r1: 0x%p, r2: 0x%08x, ret: %d, err: %d\n", r0, r1, r2, ret_val.ret_value, ret_val.err_value);
#endif
}

sys_call_ret_t angel_with_err(unsigned int r0, void *r1, unsigned int r2) {
	sys_call_ret_t ret_val;
	ret_val.raw_value = __angel64r(r0, ANGEL_OFFSET_PTR(r1), r2);

	if (ret_val.ret_value == -1)
		break_on_system_call_error(r0, r1, r2, ret_val);

	return ret_val;
}

sys_call_ret_t angel_const_with_err(unsigned int r0, const void *r1, unsigned int r2) {
	sys_call_ret_t ret_val;
	ret_val.raw_value = __angel64r(r0, ANGEL_OFFSET_PTR(r1), r2);

	if (ret_val.ret_value == -1)
		break_on_system_call_error(r0, r1, r2, ret_val);

	return ret_val;
}
