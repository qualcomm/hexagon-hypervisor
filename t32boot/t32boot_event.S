/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_trap_constants.h>

	.text

	.global bootvm_vectors
	.p2align 12
bootvm_vectors:
	jump bootvm__Reset
	jump bootvm__MachineCheck
	jump bootvm__Error
	jump bootvm__Rsvd
	jump bootvm__Rsvd
	jump bootvm__Trap0
	jump bootvm__Rsvd
	jump bootvm__Interrupt

bootvm__Reset:
bootvm__MachineCheck:
bootvm__Rsvd:
bootvm__Rsvd:
bootvm__Rsvd:
bootvm__Error:
bootvm__Trap0:
1:
	jump 1b

/*
 * If you're looking at this, you might be thinking: WTF? How does that work? 
 * The answer is: when we get the interrupt we are probably in wait-for-interrupt,
 * 
 * And so that function call will expect callee-saved regs to be clobbered.
 * When the interrupt comes it will clobber those but it's OK by the time we 
 * get back to returning from wait-for-interrupt.
 * 
 * Needless to say, that's a pretty lame way to do things, but I guess it gets
 * the job done until we have a better environment.
 */
bootvm__Interrupt:
	call handle_child_int
	trap1(#H2_VMTRAP_RETURN)
