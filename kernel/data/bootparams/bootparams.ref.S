/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <boot.h>

/*
 * Boot parameter block: r1:0 as captured at reset on the boot thread,
 * followed by reserved pad for future parameters.  The entry code in
 * boot.ref.S stores into this before anything clobbers those registers;
 * H2K_init_setup() publishes the words into the kernel globals afterwards.
 *
 * This lives in its own module rather than in boot.ref.S so that referring
 * to it does not pull boot.ref.o into a link.  boot.ref.o defines `start',
 * which collides with crt0_standalone.o in STANDALONE tests.  It is also
 * kept out of globals.ref.o, whose H2K_kg/H2K_kg_init those tests stub
 * locally, and out of H2K_kg itself, since the capture happens before
 * H2K_kg_init() zeroes the globals.
 *
 * Zero-initialized in .data rather than .bss: nothing clears .bss before
 * the capture runs.  Doubleword aligned for the memd store.
 */

DATA_START H2K_boot_params .data.core.bootparams .p2align 3
	.space (BOOT_PARAMS_WORDS * 4)

DATA_END H2K_boot_params
