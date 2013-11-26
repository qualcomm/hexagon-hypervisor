/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>
#include <asm_offsets.h>
#include <thread.h>
#include <globals.h>
#include <hw.h>
#include <vm.h>
#include <max.h>
#include <intconfig.h>
#include <asid.h>
#include <translate.h>
#include <alloc.h>
#include <stlb.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef u32_t (*configptr_t)(u32_t, u32_t, vmblock_init_op_t, u32_t, u32_t, H2K_thread_context *);

static const configptr_t H2K_configtab[CONFIG_MAX] IN_SECTION(".data.config.config") = {
	H2K_trap_config_vmblock_init,
	H2K_trap_config_stlb_alloc
};

u32_t H2K_trap_config(config_type_t configtype, u32_t val1, vmblock_init_op_t val2, u32_t val3, u32_t val4,  H2K_thread_context *me)
{
	if (configtype >= CONFIG_MAX) return 0;
	return H2K_configtab[configtype](0, val1, val2, val3, val4, me);
}

/* initialize vm description */
u32_t H2K_trap_config_vmblock_init(u32_t unused, u32_t vm, vmblock_init_op_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock;
	bitmask_t **masks;
	bitmask_t *mask;
	H2K_physint_config_t config_int;
	physint_t physint;
	u32_t virt_int;
	H2K_id_t id;
	H2K_thread_context *contexts;
	u32_t *p;
	u32_t *clear_start;
	u32_t i;
	H2K_offset_t offset;
	H2K_translation_t phys_translation;
	H2K_mem_alloc_block_t block;
	u32_t ptrtmp;

#ifdef HAVE_EXTENSIONS
	u32_t use_ext;
#ifdef DO_EXT_SWITCH
	H2K_ext_context *ext_contexts;
#endif
#endif

	/* Get vmblock initialized by previous SET_CPUS_INTS */
	if (op != SET_CPUS_INTS) {
		if (vm < H2K_ID_MAX_VMS && H2K_gp->vmblocks[vm] != NULL) { // ok
			vmblock = H2K_gp->vmblocks[vm];

			if (vmblock->num_cpus > 0) return 0;  // VM is running, don't touch it.
			/* H2K_init_setup_bootvm() calls with me == NULL */
			if (me != NULL && me->id.vmidx != vmblock->parent.vmidx) return 0;  // Call is not from parent VM
		} else {
			vmblock = NULL;
		}
	}

	switch (op) {

	case SET_CPUS_INTS:	/* Allocates a new VM  */
#ifdef HAVE_EXTENSIONS
		use_ext = arg1 & CONFIG_USE_EXT;
		arg1 &= ~CONFIG_USE_EXT;
#endif

		if ((arg1 > MAX_VM_CPUS) || (arg2 > MAX_VM_INTS)) return 0; /* bad args */

#ifdef DO_EXT_SWITCH
		block = H2K_mem_alloc_get(VMBLOCK_SIZE(arg1, arg2, use_ext));
#else
		block = H2K_mem_alloc_get(VMBLOCK_SIZE(arg1, arg2));
#endif
		if (block.ptr == NULL) return 0;  // no space

		vmblock = (H2K_vmblock_t *)(void *)block.ptr;
		ptrtmp = (u32_t)vmblock;

		for (i = 1; i < H2K_ID_MAX_VMS; i++) {
			BKL_LOCK();
			if (H2K_gp->vmblocks[i] == NULL) {
				H2K_gp->vmblocks[i] = vmblock;
				H2K_vmblock_clear(vmblock);
				vmblock->vmidx = i;
				vmblock->pmap_type = -1; // uninitialized
				BKL_UNLOCK();
				break;
			} else {
				BKL_UNLOCK();
				continue;
			}
		}
		if (i == H2K_ID_MAX_VMS) return 0;  // out of vm IDs

		vmblock->max_cpus = arg1;
		vmblock->num_cpus = 0;
		vmblock->num_ints = arg2;
		if (me != NULL) {
			vmblock->parent = me->id;
		}
		/* ID of boot VM parent stays 0:0 */
		ptrtmp += VMBLOCK_SPACE;

		/* allocate cpu contexts and set invalid */
		vmblock->contexts = contexts = (H2K_thread_context *)ptrtmp;
		for (i = 0; i < vmblock->max_cpus; i++) {
			H2K_thread_context_clear(&contexts[i]);
			contexts[i].id.raw = 0;  // because H2K_thread_context_clear restores garbage
			contexts[i].id.vmidx = vmblock->vmidx;
			contexts[i].id.cpuidx = i;
			contexts[i].next = vmblock->free_threads;
			contexts[i].vmblock = vmblock;
			vmblock->free_threads = &contexts[i];
		}
		ptrtmp += vmblock->max_cpus * sizeof(H2K_thread_context);

#ifdef HAVE_EXTENSIONS
		/* maybe initialize extended contexts */
		if (use_ext) {
			vmblock->use_ext = 1;
#ifdef DO_EXT_SWITCH
			vmblock->ext_contexts = ext_contexts = (H2K_ext_context *)ptrtmp;
			for (i = 0; i < vmblock->max_cpus; i++) {
				H2K_ext_context_clear(&ext_contexts[i]);
			}
			ptrtmp += vmblock->max_cpus * sizeof(H2K_ext_context);
#endif
		}
#endif

		vmblock->intinfo = (H2K_vm_int_opinfo_t *)ptrtmp; 
		ptrtmp += INTINFO_SPACE(vmblock->num_ints);
		H2K_vm_int_intinfo_init(vmblock,vmblock->num_ints);

		vmblock->percpu_mask = NULL;
		vmblock->pending = NULL;
		vmblock->enable = NULL;

		clear_start = (u32_t *)ptrtmp;
		if (vmblock->num_ints > 0) {
			/* allocate per-cpu mask blocks and clear */
			vmblock->percpu_mask = masks = (bitmask_t **)ptrtmp;
			ptrtmp += MASKPTR_SPACE(vmblock->max_cpus, vmblock->num_ints);
			clear_start = mask = (bitmask_t *)ptrtmp;
			ptrtmp += MASK_SPACE(vmblock->max_cpus, vmblock->num_ints);
			/* allocate pending, enable blocks and clear  */
			vmblock->pending = (bitmask_t *)ptrtmp;
			ptrtmp += PENDING_SPACE(vmblock->num_ints);
			vmblock->enable = (bitmask_t *)ptrtmp;
			ptrtmp += ENABLE_SPACE(vmblock->num_ints);

			for (i = 0; i < vmblock->max_cpus; i++, mask += MASK_WORDS_PERCPU(vmblock->num_ints)) {
				masks[i] = mask;
			}
		}
		/* allocate hw ints and set invalid */
		vmblock->int_v2p = (physint_t *)ptrtmp;
		ptrtmp += PHYSINT_SPACE(vmblock->num_ints + PERCPU_INTERRUPTS); // shared + per-cpu
		p = (u32_t *)ptrtmp;
		while (p >= clear_start) {
			*p = 0;
			p--;
		}

		return vmblock->vmidx;

	case SET_PMAP_TYPE:

		if (vmblock == NULL) return 0;

		if (arg2 == H2K_ASID_TRANS_TYPE_OFFSET) {
			/* arg1 has offset_pages[20]:xwru[4]:cccc[4]:size[4].  For negative offset wrap around by
				 using (0xffffffff - offset) */

			offset.raw = arg1;

			/* Make sure offset is aligned to a valid page size, and size field matches */
			if ((i = Q6_R_ct0_R(offset.pages)) % 2 != 0 || offset.size > (i / 2)) return 0;

			vmblock->phys_offset = offset;
			vmblock->pmap_type = arg2;
			vmblock->fence_lo = 0;
			vmblock->fence_hi = -1; // uninitialized, denies all mem
			return vmblock->vmidx;
		}

		if (arg2 >= H2K_ASID_TRANS_TYPE_XXX_LAST) return 0; // bad type

		if (arg1 == 0) {
			/* use ptb from current thread as pmap by default */
			vmblock->pmap = H2K_mem_asid_table[me->ssr_asid].ptb;
			vmblock->pmap_type = H2K_mem_asid_table[me->ssr_asid].fields.transtype;
		} else {
			vmblock->pmap_type = arg2;

			if (me == NULL) { // we are setting up the boot vm
				vmblock->pmap = arg1;
			} else { // calling from a guest that might be remapped
				if ((arg1 & 0xff000000) == 0xff000000) { // FIXME: remove this once guests no longer in monitor space
					vmblock->pmap = arg1 - H2K_gp->phys_offset;
				} else {
					phys_translation =	H2K_vm_translate(arg1, me->vmblock);
					if (!phys_translation.valid) return 0;
					vmblock->pmap = phys_translation.addr;
				}
			}
		}
		return vmblock->vmidx;

	case SET_FENCES:

		if (vmblock == NULL) return 0;

		/* Ensure that fences are at given page boundaries */
		offset = vmblock->phys_offset;
		if ((Q6_R_ct0_R(arg1 >> PAGE_BITS) / 2 < offset.size)
				|| Q6_R_ct0_R(arg2 >> PAGE_BITS) / 2 < offset.size) return 0;

		if (me == NULL) { // we are setting up the boot vm, args are already phys addresses
			vmblock->fence_lo = arg1;
			vmblock->fence_hi = arg2;
		} else { // translate, since the caller might be remapped

			/* Could be a problem here if we have two levels of offset remapping with different page sizes */
			phys_translation =	H2K_vm_translate(arg1, me->vmblock);
			if (!phys_translation.valid) return 0;
			vmblock->fence_lo = phys_translation.addr;

			phys_translation =	H2K_vm_translate(arg2, me->vmblock);
			if (!phys_translation.valid) return 0;
			vmblock->fence_hi = phys_translation.addr;
		}
		vmblock->fence_lo = (u32_t)vmblock->fence_lo >> PAGE_BITS;
		vmblock->fence_hi = (u32_t)vmblock->fence_hi >> PAGE_BITS;

		return vmblock->vmidx;

	case SET_PRIO_TRAPMASK:

		if (vmblock == NULL) return 0;

		if (arg1 > MAX_PRIO) return 0; /* bad arg */

		vmblock->bestprio = (u8_t)arg1;
		vmblock->trapmask = (u32_t)arg2;
		return vmblock->vmidx;

	case MAP_PHYS_INTR:

		if (vmblock == NULL) return 0;

		config_int.raw = arg2;
		physint = config_int.physint;
		virt_int = arg1;
		id.vmidx = vmblock->vmidx;
		id.cpuidx = config_int.cpuidx;

		if ((virt_int >= (vmblock->num_ints + PERCPU_INTERRUPTS)) 
				|| (physint >= MAX_INTERRUPTS)
				|| (id.cpuidx >= vmblock->max_cpus)
				)  return 0; /* bad args */

		/* skip H2 interrupts instead of returning error */
		if (! ((physint == RESCHED_INT)
					 || (physint == VM_IPI_INT)
					 || (physint == TIMER_INT)
#ifdef H2K_L2_CONTROL
					 || (physint == L2_CORE_INTERRUPT)
#endif
					 || (virt_int == 0) // reserved for large vint#
					 )) {
			vmblock->int_v2p[virt_int] = physint;
			/* FIXME: set up int mapping in vmblock for large vint# */
			H2K_register_passthru(physint, id, virt_int);
		}
		return vmblock->vmidx;

	default:
		return 0; // bad op
	}
}

u32_t H2K_trap_config_stlb_alloc(u32_t unused, u32_t sets, vmblock_init_op_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me) {

	return H2K_mem_stlb_alloc();
}
