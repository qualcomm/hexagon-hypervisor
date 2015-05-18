/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

	.section .start, "ax", @progbits
	.global start, _start
	.type start, @function
_start:
start:
	call load_autoboot
	jump 1f
	.align 0x20
	/* This should be address 0x20 */
	.word 0		// rtos_debug_0
	.word 0		// rtos_debug_1
	.word 0		// rtos_debug_2
	.word 0		// rtos_debug_3
	.word 0		// isdb_secure_flag
	.word 0		// isdb_trusted_flag
	.word 0		// isdb_debug_flag
	.word 0		// QURT_system_state
	/* ELF INFO PATCH goes here? */

1:
	{
		r0 = ##_SDA_BASE_
		r29 = ##bootstack
	}
	{
		gp = r0
		r31:30 = #0
	}
	// Become a qurt thread
	call ##qurt_thread_mainthread_become
	{
		r1 = ##QURTK_tlb_dump
		r0 = ##qurtos_mmap_table
	}
	call ##qurt_memory_init_early
	{
		r2 = ##_init
		r3 = ##_fini
	}
	// set up and call sys_get_cmdline?
	// bss/sbss clear?
	// Some sort of crazy weak hook to set up qurt stuff instead of calling from main?
	{
		call ##_start_main
		r1:0 = #0
	}
	/* Should not actually return */
1:
	jump 1b // or fatal error or something

load_autoboot:
	// Clear out caches for supersition
	ickill
	dckill
	// 1 == enable ISDB. 0x2E == icache|dcache|isdbrdy|isdbtrusted
	r1:0 = combine(#1,#0x2e)
	syscfg = r0
	isync
	isdben = r1
	isync
	// now wait for a breakpoint so t32 can attach
	nop
	nop
	brkpt
	nop
	nop
	// Copy over autoboot image
	r0 = ##0x8d400000
	r1 = ##1f
	r2 = ##((2f-1f)/8)-1
	{
		loop0(3f,r2)
		r5:4 = memd(r1++#8)
	}
3:
	{
		r5:4 = memd(r1++#8)
		memd(r0++#8) = r5:4
	}:endloop0
	memd(r0++#8) = r5:4
	// Save return address in magic memory location, autoboot will start new VM there
	r0 = ##0x8d4ffffc
	memw(r0) = r31
	// Jump to autoboot
	r0 = ##0x8d400000
	jumpr r0

	.p2align 3
1:
	.incbin "autoboot.bin"
	.p2align 3
2:
	.word 0

	.data
	.p2align 5
_bootstack_limit:
	.space 512
bootstack:
	.word 0
	.word 0
