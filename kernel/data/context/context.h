/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONTEXT_H
#define H2K_CONTEXT_H

#include <c_std.h>
#include <tree.h>
#include <idtype.h>
#include <max.h>

#define H2K_CONTEXT_ALIGN 32

enum {
	H2K_STATUS_DEAD = 0,
	H2K_STATUS_RUNNING,
	H2K_STATUS_READY,
	H2K_STATUS_BLOCKED,
	H2K_STATUS_VMWAIT,
	H2K_STATUS_INTBLOCKED,
};

/*
 * 64 bytes per thread of context that must stay present seems reasonable.
 * 224-256 bytes per thread of context that can be put in DDR
 * @ 100 threads, 6400 bytes in, seems reasonable
 */

/* If we're active, on a trap we want to save off r19-r16, r29:28, r31:30, continuation, ccrssr
 * ... maybe that fits in the first 64 bytes.
 * If we go to 5 cache lines, we'd want trap context in the first cache line, context_save context in first two lines,
 * and switch in the rest.  
 * ... so ...
 * r31-r28, r19-r16, ccrssr, continuation, + 5 words from full ctx save
 * r0-r15 lc0sa0 usr_preds [~20 words]
 * rest of regs in switch
 *
 * I'm really concerned about having two separate locations.  Everywhere we have me->xxx it will be me->bulk->xxx.
 * We can keep bulk pointer in SGP1, but then we won't have KSP there.  Unless we have some register storage in
 * the first 64 bytes, we don't have a place to put the bulk pointer.  And it's another pointer load.
 * r0100 may *have* to go there, because interrupts can abort blocking.  I guess that might solve the storage problem...
 * though not in a great way.
 */

typedef struct _h2_thread_context
{
	/* Kernel Variables */
	/* Make ring compatible */
	struct _h2_thread_context *next;
	struct _h2_thread_context *prev;
	// #8
	/* Other info */
	union {
		struct {
			u8_t tid;			// STID
			u8_t hthread;			// could be < 8 bits
			u8_t prio;
			u8_t status;			// could be < 8 bits, combined with vmstatus?
		};
		u32_t status_prio_hthread_tid;
	};
	// #12
	union {				// must be updated with LL/SC?
		u32_t atomic_status_word;
		struct {
			u8_t vmstatus;
			u8_t base_prio;	// Does it need to be atomic?
			u8_t tlbidxmask;	// mask tlbidx
			u8_t unused;
					// island mode?
		};
	};
	// #16
	union {
		u64_t vmblock_id;
		struct {
			H2K_id_t id;				// lower bits unused? Maybe union with vmstatus? extra futex pa bits?
			struct H2K_vmblock_struct *vmblock;	// could look up from GP + high bits of id. Not used so much. --> futex_ptr
		};
	};
	// 24
	struct {
		u32_t trapmask;		// Alread in VMblock?  Maybe move it?  All threads in a VM have same trap mask?
		u32_t elr;	// could be in zeroed area. --> bulk storage ptr
	};
	// 32
	union {
		struct {
			u64_t rightleft;
			u64_t timeout;
		};
		H2K_treenode_t tree;
	};
	// 48
	struct {
		union {	// maybe change to 16 bits or even 8 bits?  Saves 1-1.5 words
			u32_t cpuint_enabled_pending;
			struct {
				u16_t cpuint_pending;
				u16_t cpuint_enabled;
			};
		};
		void *gevb;		// isn't necessarily the same for all CPUs in a guest.  Also, we want it quickly.  Can move to DDR, don't zero though
	};
	u64_t totalcycles;		// can move to ddr, don't zero though
	// 64
	u64_t pktcount;
	union {	// need to keep futex ptr in TCM if split
		struct {
			u32_t futex_ptr_lo;		// Probably not needed if interrupted; only on trap; could be unioned below?
			// needs to be pa_t, but is word aligned.  For 36 bits pa, can be 34 bits... 
			union {
				u32_t futex_ptr_hi;		// only low 2 bits
				void *continuation;		// only high 30 bits
			};
		};
		struct {
			u64_t futex_ptr:34;		// only low 34 bits
			u64_t continuation_bits:30;
		};
		u64_t futex_ptr_64;
	};
	union {
		u64_t ccrssr;
		struct {
			union {
				u32_t ssr;
				struct {
					u8_t ssr_cause;
#if ARCHV <= 3
					u8_t ssr_asid:5;
					u8_t ssr_guest:1; /* Fake guest bit using a bit of ASID */
					u8_t ssr_asid_byte_unused:2;
#else
					u8_t ssr_asid:7;
					u8_t ssr_asid_byte_unused:1;
#endif
					u8_t ssr_um:1;
					u8_t ssr_ex:1;
					u8_t ssr_ie:1;
#if ARCHV <= 3
					u8_t ssr_tnum:3;
					u8_t ssr_hfi:1;
					u8_t ssr_hfd:1;
					u8_t ssr_sfd:1;
					u8_t ssr_hi_rsvd:7;
#else
					u8_t ssr_guest:1;
					u8_t ssr_badva_v0:1;
					u8_t ssr_badva_v1:1;
					u8_t ssr_badva_bvs:1;
					u8_t ssr_badva_ce:1;
					u8_t ssr_badva_pe:1;
#ifdef HAVE_EXTENSIONS
					u8_t ssr_hi_rsvd:2;
					u8_t ssr_xa:3;
#else
					u8_t ssr_hi_rsvd:5;
#endif
					u8_t ssr_ss:1;
#ifdef HAVE_EXTENSIONS
					u8_t ssr_xe:1;
#else
					u8_t ssr_unused:1;
#endif

#endif
				};
			};
			u32_t ccr;
		};
	};
	u64_t r3130;
	union {
		u64_t r2928;
		struct {
			u32_t r28;
			u32_t r29;
		};
	};
	// 96
	u64_t r1918;	// OK FOR DCZEROA
	u64_t r1716;
	union {
		u64_t usrp30;
		struct {
			u32_t p30;
			u32_t usr;
		};
	};
	union {
		u64_t r0100;	/* used for return value */
		struct {
			u32_t r00;
			u32_t r01;
		};
	};
	/* Context required for interrupts... everything else */
	/* Note: SR really needed for any context switch.  */
	/* Note: Fast Interrupt contexts don't need these (can't be interrupted) */
	// 128
	u64_t r1514;	// OK FOR DCZEROA
	u64_t r1312;
	u64_t r1110;
	u64_t r0908;
	// 160
	u64_t r0706;	// OK FOR DCZEROA
	u64_t r0504;
	union {
		u64_t r0302;
		struct {
			u32_t r02;
			u32_t r03;
		};
	};
	u64_t lc0sa0;
	// 192
	union {
		u64_t gpugp;
		struct {
			u32_t ugp;
			u32_t gp;
		};
	};
	u64_t lc1sa1;	// OK FOR DCZEROA
	u64_t m1m0;
	// 224
	u64_t r2726;	// OK for dczeroa
	u64_t r2524;
	u64_t r2322;
	u64_t r2120;
	// 256
	union {
		u64_t gssr_gelr;	// OK to move to cleared area?
		struct {
			u32_t gelr;
			u32_t gssr;
		};
	};
	union {
		u64_t gbadva_gosp;	// OK to move to cleared area?
		struct {
			u32_t gosp;
			u32_t gbadva;
		};
	};
	u64_t cs1cs0;	// V4 regs
	union {
		u64_t framekey_framelimit;
		struct {
			u32_t framelimit;
			u32_t framekey;
		};
	};
	// 288
} __attribute__((aligned(H2K_CONTEXT_ALIGN))) H2K_thread_context;

/* Big enough for 128-byte contexts. FIXME: size this space dynamically */
typedef struct {
	u32_t vregs[32][32];
	u32_t qregs[32];
} __attribute__((aligned(H2K_CONTEXT_ALIGN))) H2K_ext_context;

typedef struct {
	H2K_thread_context context;
	u64_t stack120;
	u64_t stack112;
	u64_t stack104;
	u64_t stack096;
	u64_t stack088;
	u64_t stack080;
	u64_t stack072;
	u64_t stack064;
	u64_t stack056;
	u64_t stack048;
	u64_t stack040;
	u64_t stack032;
	u64_t stack024;
	u64_t stack016;
	u64_t stack008;
	u64_t stack000;
} H2K_fastint_context;

#endif
