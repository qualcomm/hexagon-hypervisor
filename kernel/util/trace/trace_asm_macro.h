/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TRACE_ASM_MACRO_H
#define TRACE_ASM_MACRO_H 1

#ifndef ASM
#error Must be only used from assembly 
#endif

#if 0
/* EJP: non-TRACE instruction */
/*
 * ASM_INLINE_TRACE macro
 * Takes a few arguments here...
 * MSG is an arbitrary word.  It gets clobbered.
 * HTNUM_PCYCLELO is HTNUM in the high half, PCYCLELO in the low half.  Also clobbered.
 * ENTRIES_BUFADDR is ENTRIES in the high half, BUFADDR in the low half.  BUFADDR is filled
 *   with the address of the trace entry.  You should clean it.  ENTRIES is a scratch val.
 * Additionally I need the register numbers of some more scratch values:
 * - PRED is a predicate scratch value
 * - OLDIDX is a single
 * - NEWIDX is a single
 * - IDXADDR is a single
 */
#define ASM_INLINE_TRACE(MSG,HTNUM_PCYCLELO,HTNUM,PCYCLELO,ENTRIES_BUFADDR,ENTRIES,BUFADDR,PRED,OLDIDX,NEWIDX,IDXADDR) \
	{ \
		PCYCLELO = insert(HTNUM,#4,#0); /* no retry */ \
		IDXADDR = add(H2K_GP,#KG_trace_info_index); \
		ENTRIES_BUFADDR = memd(H2K_GP+#KG_trace_info_entries_buf); \
	}; \
1: \
	{ \
		OLDIDX = memw_locked(IDXADDR); \
		NEWIDX = add(ENTRIES,#-1);  /* Use NEWIDX as temp */ \
		HTNUM = MSG; \
	}; \
	{ \
		PRED = cmp.eq(NEWIDX,OLDIDX); /* Done with NEWIDX as temp */ \
		if (!PRED.new) NEWIDX = add(OLDIDX,#1); \
		if (PRED.new) NEWIDX = #0; \
		BUFADDR = addasl(BUFADDR,OLDIDX,#3); \
	}; \
	{ \
		memw_locked(IDXADDR,PRED) = NEWIDX; \
		EXTRAINSNS0 ; \
	}; \
	{ \
		if (!PRED) jump 1b; \
		if (PRED) memd(BUFADDR) = HTNUM_PCYCLELO; \
		EXTRAINSNS1 ; \
	}

#else

/*
 * ASM_INLINE_TRACE macro
 * Takes a few arguments here...
 * MSG is an arbitrary word.  It gets clobbered.
 * HTNUM_PCYCLELO is HTNUM in the high half, PCYCLELO in the low half.  Also clobbered.
 * ENTRIES_BUFADDR is ENTRIES in the high half, BUFADDR in the low half.  BUFADDR is filled
 *   with the address of the trace entry.  You should clean it.  ENTRIES is a scratch val.
 * Additionally I need the register numbers of some more scratch values:
 * - PRED is a predicate scratch value
 * - OLDIDX is a single
 * - NEWIDX is a single
 * - IDXADDR is a single
 * - EXTRAINSNS0 is optional, 3 extra instructions to add to a packet with a memw_locked
 * - EXTRAINSNS1 is optional, 2 extra instructions to add to a packet with a jump and store
 */

#define ASM_INLINE_TRACE(MSG,HTNUM_PCYCLELO,HTNUM,PCYCLELO,ENTRIES_BUFADDR,ENTRIES,BUFADDR,PRED,OLDIDX,NEWIDX,IDXADDR,EXTRAINSNS0,EXTRAINSNS1) \
	trace(MSG) 	// oh yeah

#endif

#endif
