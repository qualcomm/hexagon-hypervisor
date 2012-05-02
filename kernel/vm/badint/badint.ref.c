/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <vmint.h>
#include <badint.h>

s32_t H2K_vm_badint_func(H2K_vmblock_t *vmblock, H2K_thread_context *dest, u32_t intno, H2K_vm_int_opinfo_t *info)
{
	return -1;
}

const H2K_vm_int_ops_t H2K_vm_badint_ops = {
	.nop = H2K_vm_badint_func,
	.enable = H2K_vm_badint_func,
	.disable = H2K_vm_badint_func,
	.localen = H2K_vm_badint_func,
	.localdis = H2K_vm_badint_func,
	.setaffinity = H2K_vm_badint_func,
	.get = H2K_vm_badint_func,
	.peek = H2K_vm_badint_func,
	.status = H2K_vm_badint_func,
	.post = H2K_vm_badint_func,
	.clear = H2K_vm_badint_func,
};

