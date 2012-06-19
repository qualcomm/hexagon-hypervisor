/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <config.h>
#include <asm_offsets.h>
#include <thread.h>
#include <fatal_handler.h>
#include <globals.h>
#include <hw.h>
#include <vm.h>
#include <max.h>
#include <intconfig.h>
#include <asid.h>
#include <translate.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef u32_t (*configptr_t)(u32_t, void *, u32_t, u32_t, u32_t, H2K_thread_context *);

static const configptr_t H2K_configtab[CONFIG_MAX] IN_SECTION(".data.config.config") = {
	H2K_trap_config_setfatal,
	H2K_trap_config_vmblock_size,
	H2K_trap_config_vmblock_init,
};

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, u32_t val4,  H2K_thread_context *me)
{
	if (configtype >= CONFIG_MAX) return 0;
	return H2K_configtab[configtype](0,ptr,val2,val3,val4,me);
}

u32_t H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me)
{
	H2K_fatal_kernel_handler = handler;
	return 0;
}

/* return vm storage size required for vm with given parameters */

u32_t H2K_trap_config_vmblock_size(u32_t unused, void *unused2, u32_t max_cpus, u32_t num_ints, u32_t unused3, H2K_thread_context *me)
{
	return VMBLOCK_SIZE(max_cpus, num_ints);
}

/* FIXME: need to validate vmblock pointer to guest space in these functions */

u32_t H2K_vmblock_valid(H2K_vmblock_t *vmblock) {
	if (vmblock != NULL && H2K_gp->vmblocks[vmblock->vmidx] == vmblock) { // ok
		return 1;
	}
	return 0;
}

/* initialize vm description */
u32_t H2K_trap_config_vmblock_init(u32_t unused, void *ptr, u32_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock;
	bitmask_t **masks;
	bitmask_t *mask;
	H2K_physint_config_t config_int;
	physint_t physint;
	u32_t virt_int;
	H2K_id_t id;
	struct _h2_thread_context *contexts;
	u32_t *p;
	u32_t i;
	H2K_offset_t offset;

	u32_t ptrtmp = (u32_t)ptr;
	/* Align Pointer */
	ptrtmp = ((ptrtmp + (H2K_VMBLOCK_ALIGN-1)) & (-H2K_VMBLOCK_ALIGN));
	vmblock = (H2K_vmblock_t *)ptrtmp;

	switch (op) {
	case SET_STORAGE:

		/* raw space, must align */
		if (ptrtmp & (H2K_VMBLOCK_ALIGN-1)) {
			ptrtmp += ((ptrtmp + (H2K_VMBLOCK_ALIGN - 1)) & (-H2K_VMBLOCK_ALIGN)) - ptrtmp;
		}
		ptrtmp = ROUND(ptrtmp);
		vmblock = (H2K_vmblock_t *)ptrtmp;
		for (i = 1; i < H2K_ID_MAX_VMS; i++) {
			BKL_LOCK();
			if (H2K_gp->vmblocks[i] == NULL) {
				H2K_gp->vmblocks[i] = vmblock;
				H2K_vmblock_clear(vmblock);
				vmblock->vmidx = i;
				BKL_UNLOCK();
				break;
			} else {
				BKL_UNLOCK();
				continue;
			}
		}

		return (u32_t)vmblock;

	case SET_PMAP_TYPE:
		if (!H2K_vmblock_valid(vmblock)) return 0;

		if (arg2 == H2K_ASID_TRANS_TYPE_OFFSET) {
			/* arg1 has offset[24]:size[8].  For negative offset wrap around by
				 adding (0xffffffff - offset) */

			offset.raw = arg1;

			/* Make sure offset is aligned to a valid page size, and size field matches */
			if ((i = Q6_R_ct0_R(offset.pages)) % 4 != 0 || offset.size != (i / 2)) return 0;

			vmblock->phys_offset = offset;
			vmblock->pmap_type = arg2;
			vmblock->fence_hi = 1; // deny all mem, in case we forget to configure fences
			return (u32_t)vmblock;
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
				if (H2K_translate(arg1, me->vmblock->pmap, me->vmblock->pmap_type, &vmblock->pmap) == -1) return 0;
			}
		}
		return (u32_t)vmblock;

	case SET_FENCES:
		if (!H2K_vmblock_valid(vmblock)) return 0;

		/* Ensure that fences are at given page boundaries */
		offset = vmblock->phys_offset;
		if ((Q6_R_ct0_R(arg1 >> PAGE_BITS) / 2 != offset.size)
				|| Q6_R_ct0_R(arg2 >> PAGE_BITS) / 2 != offset.size) return 0;

		/* Could be a problem here if we have two levels of offset remapping with different page sizes */
		if (H2K_translate(arg1, me->vmblock->pmap, me->vmblock->pmap_type, &vmblock->fence_lo) == -1) return 0;
		if (H2K_translate(arg2, me->vmblock->pmap, me->vmblock->pmap_type, &vmblock->fence_hi) == -1) return 0;

		vmblock->fence_lo >>= PAGE_BITS;
		vmblock->fence_hi >>= PAGE_BITS;
		return (u32_t)vmblock;

	case SET_PRIO_TRAPMASK:
		if (!H2K_vmblock_valid(vmblock)) return 0;
		if (arg1 > MAX_PRIO) return 0; /* bad arg */

		vmblock->bestprio = (u8_t)arg1;
		vmblock->trapmask = (u32_t)arg2;
		return (u32_t)vmblock;

	case SET_CPUS_INTS:
		if (!H2K_vmblock_valid(vmblock)) return 0;
		if ((arg1 > MAX_VM_CPUS) || (arg2 > MAX_VM_INTS)) return 0; /* bad args */

		vmblock->max_cpus = arg1;
		vmblock->num_cpus = 0;
		vmblock->num_ints = arg2;

		ptrtmp += VMBLOCK_SPACE;

		/* allocate cpu contexts and set invalid */
		vmblock->contexts = contexts = (H2K_thread_context *)ptrtmp;
		for (i = 0; i < vmblock->max_cpus; i++) {
			H2K_thread_context_clear(&contexts[i]);
			contexts[i].id.vmidx = vmblock->vmidx;
			contexts[i].id.cpuidx = i;
			contexts[i].next = vmblock->free_threads;
			contexts[i].vmblock = vmblock;
			vmblock->free_threads = &contexts[i];
		}
		ptrtmp += vmblock->max_cpus * sizeof(H2K_thread_context);
		vmblock->intinfo = (H2K_vm_int_opinfo_t *)ptrtmp; 
		ptrtmp += INTINFO_SPACE(vmblock->num_ints);
		H2K_vm_int_intinfo_init(vmblock,vmblock->num_ints);

		if (vmblock->num_ints > 0) {
			/* allocate per-cpu mask blocks and clear */
			vmblock->percpu_mask = masks = (bitmask_t **)ptrtmp;
			ptrtmp += MASKPTR_SPACE(vmblock->max_cpus, vmblock->num_ints);
			mask = (bitmask_t *)ptrtmp;
			ptrtmp += MASK_SPACE(vmblock->max_cpus, vmblock->num_ints);
			/* allocate pending, enable blocks and clear  */
			vmblock->pending = (bitmask_t *)ptrtmp;
			ptrtmp += PENDING_SPACE(vmblock->num_ints);
			vmblock->enable = (bitmask_t *)ptrtmp;
			ptrtmp += ENABLE_SPACE(vmblock->num_ints);
			/* allocate hw ints and set invalid */
			vmblock->int_v2p = (physint_t *)ptrtmp;
			ptrtmp += PHYSINT_SPACE(vmblock->num_ints);
			p = (u32_t *)ptrtmp;
			while (p != mask) {
				*p = 0;
				p--;
			}
			for (i = 0; i < vmblock->max_cpus; i++, mask += MASK_WORDS_PERCPU(vmblock->num_ints)) {
				masks[i] = mask;
			}
		} else {
			vmblock->percpu_mask = NULL;
			vmblock->pending = NULL;
			vmblock->enable = NULL;
			vmblock->int_v2p = NULL;
		}

		return (u32_t)vmblock;

	case MAP_PHYS_INTR:
		if (!H2K_vmblock_valid(vmblock)) return 0;
		config_int.raw = arg2;
		physint = config_int.physint;
		virt_int = arg1;
		id.vmidx = vmblock->vmidx;
		id.cpuidx = config_int.cpuidx;

		if ((virt_int >= vmblock->num_ints) 
				|| (physint >= MAX_INTERRUPTS)
				|| (id.cpuidx >= vmblock->max_cpus)
				|| (physint == RESCHED_INT)
				|| (physint == VM_IPI_INT)
				|| (physint == TIMER_INT)
#ifdef H2K_L2_CONTROL
				|| (physint == L2_CORE_INTERRUPT)
#endif
				|| (virt_int == 0))  return 0; /* bad args */
		vmblock->int_v2p[virt_int] = physint;
		/* FIXME: set up int mapping in vmblock for large vint# */
		H2K_register_passthru(physint, id, virt_int);
		return (u32_t)vmblock;
	}
	return 0; // bad op
}
