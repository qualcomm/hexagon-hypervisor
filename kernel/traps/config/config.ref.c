/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>
#include <asm_offsets.h>
#include <thread.h>
#include <fatal.h>
#include <globals.h>
#include <hw.h>
#include <vm.h>
#include <max.h>

typedef u32_t (*configptr_t)(u32_t, void *, u32_t, u32_t, u32_t, H2K_thread_context *);

#define MAX_CONFIGS 4

static const configptr_t H2K_configtab[MAX_CONFIGS] IN_SECTION(".data.config.config") = {
	H2K_trap_config_add_thread_storage,
	H2K_trap_config_setfatal,
	H2K_trap_config_vmblock_size,
	H2K_trap_config_vmblock_init,
};

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, u32_t val4,  H2K_thread_context *me)
{
	if (configtype >= MAX_CONFIGS) return 0;
	return H2K_configtab[configtype](0,ptr,val2,val3,val4,me);
}

u32_t H2K_trap_config_add_thread_storage(u32_t unused, void *ptr, u32_t size, u32_t unused2, u32_t unused3, H2K_thread_context *me)
{
	u32_t ptrtmp = (u32_t)ptr;
	H2K_thread_context *thread;
	u32_t delta;
	u32_t i;
	u32_t created_threads = 0;
	if (ptrtmp & (H2K_CONTEXT_ALIGN-1)) {
		delta = ((ptrtmp + (H2K_CONTEXT_ALIGN-1)) & (-H2K_CONTEXT_ALIGN)) - ptrtmp;
		ptrtmp += delta;
		size -= delta;
	}
	for (i = 0; i+CONTEXT_SIZE <= size; i += CONTEXT_SIZE) {
		thread = (H2K_thread_context *)(ptrtmp+i);
		H2K_thread_context_clear(thread);
		BKL_LOCK();
		thread->next = H2K_gp->free_threads;
		H2K_gp->free_threads = thread;
		BKL_UNLOCK();
		created_threads++;
	}
	return created_threads;
}

u32_t H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me)
{
	H2K_fatal_kernel_handler = handler;
	return 0;
}

#define ALIGNMENT sizeof(u32_t)
#define ALIGN(expr) ((((expr) + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT)

#define BITS_PER_WORD 32
#define BYTES_PER_WORD (sizeof(u32_t) / sizeof(u8_t))
#define PHYS_PER_WORD (sizeof(u32_t) / sizeof(physint_t))

#define VMBLOCK_SPACE ALIGN(sizeof(H2K_vmblock_t))

// pending
#define PENDING_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define PENDING_SPACE(ints) ALIGN(PENDING_WORDS(ints) * BYTES_PER_WORD)

// enable
#define ENABLE_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define ENABLE_SPACE(ints) ALIGN(ENABLE_WORDS(ints) * BYTES_PER_WORD)

// percpu_mask
#define MASKPTR_SPACE(cpus) ALIGN((cpus * sizeof(bitmask_t *)))
#define MASK_WORDS(cpus, ints) (cpus * (ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define MASK_SPACE(cpus, ints) ALIGN(MASK_WORDS(cpus, ints) * BYTES_PER_WORD)

// int_v2p
#define PHYSINT_WORDS(ints) ((ints * sizeof(physint_t) + PHYS_PER_WORD - 1) / PHYS_PER_WORD)
#define PHYSINT_SPACE(ints) ALIGN(PHYSINT_WORDS(ints) * BYTES_PER_WORD)

// cpu_contexts
#define CONTEXT_SPACE(cpus) ALIGN(cpus * sizeof(struct _h2_thread_context *))

/* return vm storage size required for vm with given parameters */

u32_t H2K_trap_config_vmblock_size(u32_t unused, void *unused2, u32_t max_cpus, u32_t num_ints, u32_t unused3, H2K_thread_context *me)
{

	return
		VMBLOCK_SPACE + 
		PENDING_SPACE(num_ints) +
		ENABLE_SPACE(num_ints) +
		MASKPTR_SPACE(max_cpus) +
		MASK_SPACE(max_cpus, num_ints) +
		PHYSINT_SPACE(num_ints) +
		CONTEXT_SPACE(max_cpus) +
		(H2K_VMBLOCK_ALIGN - 1);  // space to align if needed
}

/* initialize vm description */
/* FIXME: rename enable to global_mask since there is already another enable bit */

u32_t H2K_trap_config_vmblock_init(u32_t unused, void *ptr, u32_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{

	H2K_vmblock_t *vmblock;
	bitmask_t *pends;
	bitmask_t *enables;
	bitmask_t **masks;
	bitmask_t *mask;
	physint_t *physints;
	struct _h2_thread_context **contexts;

	u32_t i, j;

	u32_t ptrtmp = (u32_t)ptr;
	vmblock = (H2K_vmblock_t *)ptrtmp; /* for all but SET_STORAGE_IDENT_PMAP, assume already aligned */

	switch (op) {
	case SET_STORAGE_IDENT:
		/* raw space, must align */
		if (ptrtmp & (H2K_VMBLOCK_ALIGN-1)) {
			ptrtmp += ((ptrtmp + (H2K_VMBLOCK_ALIGN - 1)) & (-H2K_VMBLOCK_ALIGN)) - ptrtmp;
		}
		vmblock = (H2K_vmblock_t *)ALIGN(ptrtmp);

		if (arg1 > MAX_VM_ID) return 0; /* bad arg */
		vmblock->ident = (u8_t)arg1;
		return (u32_t)vmblock;

	case SET_PMAP_TYPE:
		if (!arg1) { 								/* use ptb from current thread as pmap by default */
			vmblock->pmap = H2K_mem_asid_table[me->ssr_asid].ptb;
		}
		else {
			vmblock->pmap = (u32_t)arg1;
		}

		if (arg2 >= H2K_ASID_TRANS_TYPE_XXX_LAST) return 0; // bad type
		vmblock->pmap_type = arg2;
		return (u32_t)vmblock;

	case SET_PRIO_TRAPMASK:
		if (arg1 > MAX_PRIO) return 0; /* bad arg */
		vmblock->bestprio = (u8_t)arg1;
		vmblock->trapmask = (u32_t)arg2;
		return (u32_t)vmblock;

	case SET_CPUS_INTS:
		if (arg1 > MAX_VM_CPUS || arg2 > MAX_VM_INTS) return 0; /* bad args */
		ptrtmp += VMBLOCK_SPACE;

		vmblock->max_cpus = (u8_t)arg1;
		vmblock->num_cpus = 0;
		vmblock->num_ints = (u16_t)arg2;

		/* allocate pending, enable blocks and clear  */
		vmblock->pending = pends = (bitmask_t *)ptrtmp;
		ptrtmp += PENDING_SPACE(vmblock->num_ints);
		vmblock->enable = enables = (bitmask_t *)ptrtmp;
		ptrtmp += ENABLE_SPACE(vmblock->num_ints);

		for (i = 0; i < ENABLE_WORDS(vmblock->num_ints); i++) {
			pends[i] = 0;
			enables[i] = 0;
		}

		/* allocate per-cpu mask blocks and clear */
		vmblock->percpu_mask = masks = (bitmask_t **)ptrtmp;
		ptrtmp += MASKPTR_SPACE(vmblock->max_cpus);
		mask = (bitmask_t *)ptrtmp;
		ptrtmp += MASK_SPACE(vmblock->max_cpus, vmblock->num_ints);

		for (i = 0; i < vmblock->max_cpus; i++, mask += MASK_WORDS(vmblock->max_cpus, vmblock->num_ints)) {
			masks[i] = mask;
			for (j = 0; j < MASK_WORDS(vmblock->max_cpus, vmblock->num_ints); j++) {
				mask[j] = 0;
			}
		}

		/* allocate hw ints and set invalid */
		vmblock->int_v2p = physints = (physint_t *)ptrtmp;
		ptrtmp += PHYSINT_SPACE(vmblock->num_ints);

		for (i = 0; i < vmblock->num_ints; i++) {
			physints[i] = H2K_VMBLOCK_V2P_INVALID;
		}

		/* allocate cpu contexts and set invalid */
		vmblock->cpu_contexts = contexts = (struct _h2_thread_context **)ptrtmp;
		for (i = 0; i < vmblock->max_cpus; i++) {
			contexts[i] = 0;
		}
		return (u32_t)vmblock;

	case MAP_PHYS_INTR:
		if (((u16_t)arg1 >= vmblock->num_ints) || ((physint_t)arg2) > MAX_INTERRUPTS) return 0; /* bad args
 */
		vmblock->int_v2p[arg1] = (physint_t)arg2;
		return (u32_t)vmblock;
	}
	return 0; // bad op
}
