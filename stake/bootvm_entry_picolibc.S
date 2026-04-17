/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>

/*
 * Picolibc variant of bootvm_entry.S.
 *
 * The original bootvm_entry.S jumps to hexagon_pre_main which is the
 * Dinkumware (h2 oslib) CRT entry point.  Picolibc does not provide
 * hexagon_pre_main; its CRT entry point is _start (placed in
 * .text.init.enter by crt0-noflash-hosted.o).  _start sets up the
 * stack pointer from the __stack linker-script symbol, initialises
 * TLS and BSS, runs constructors via __libc_init_array, then calls
 * main().
 */
FUNC_START __bootvm_entry_point .entry .p2align 12
	// r0 has phys_offset
	r1 = #__boot_net_phys_offset__
	memw(r1) = r0
	jump _start          // picolibc CRT entry (replaces hexagon_pre_main)

FUNC_END __bootvm_entry_point
