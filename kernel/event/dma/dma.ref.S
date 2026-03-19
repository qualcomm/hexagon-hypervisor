
#if ARCHV >= 73
#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_dma .text.core.dma .p2align 2
	crswap(r0,sgp0)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* R7 = sgp0, r8 = ssr, r9 = htid */
	r19 = and(r8,#0xfe)
	p0 = cmp.eq(r19,#0x76)  // DMA TLB miss is 0x76/0x77
	if (!p0) jump H2K_error_nonxe
#ifdef COUNT_TLB_EVENTS
	// atomically increment me->vmblock->tlbmissrw
	r10 = memw(r7 + #CONTEXT_vmblock)
	r10 = add(r10, #VMBLOCK_dmatlbmiss)
	r5:4 = #1
.Lagain:
	r3:2 = memd_locked(r10)
	r3:2 = add(r3:2, r5:4)
	memd_locked(r10, p0) = r3:2
	if !p0 jump .Lagain
#endif
	r2 = badva  // expected by tlbmiss_common
	jump H2K_tlbmiss_common

FUNC_END H2K_handle_dma
#endif
