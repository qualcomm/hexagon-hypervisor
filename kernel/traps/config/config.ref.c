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
#include <h2_common_defs.h>
#include <bzero.h>
#ifdef DO_EXT_SWITCH
#include <hvx.h>
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

typedef u32_t (*configptr_t)(u32_t, u32_t, u32_t, u32_t, u32_t, H2K_thread_context *);

static const configptr_t H2K_configtab[CONFIG_MAX] IN_SECTION(".data.config.config") = {
	H2K_trap_config_vmblock_init,
	H2K_trap_config_stlb_alloc,
	H2K_trap_config_fatal_hook,
};

u32_t H2K_trap_config(config_type_t configtype, u32_t val1, u32_t val2, u32_t val3, u32_t val4,  H2K_thread_context *me)
{
	if (configtype >= CONFIG_MAX) return 0;
	return H2K_configtab[configtype](0, val1, val2, val3, val4, me);
}

/*
 * EJP: FIXME: Not sure fatal hook is what we want any more... 
 */
u32_t H2K_trap_config_fatal_hook(u32_t unused, u32_t funcaddr, u32_t arg, u32_t val3, u32_t val4, H2K_thread_context *me)
{
	u32_t gpval;
	asm volatile (" %0 = gp ": "=r"(gpval));
	H2K_gp->fatal_hook = funcaddr;
	H2K_gp->fatal_hook_arg = arg;
	H2K_gp->fatal_hook_ssr = ((u32_t)me->ssr_asid) << 8;
	H2K_gp->fatal_hook_gp = gpval;
	return 0;
}

static u32_t H2K_config_vmblock_init_cpus_ints(H2K_vmblock_t *vmblock, u32_t vm, u32_t unused, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	u32_t alloc_size;
	bitmask_t **masks;
	bitmask_t *mask;
	H2K_thread_context *contexts;
	u32_t ptrtmp;
	int i;
#ifdef HAVE_EXTENSIONS
	u32_t use_ext;
#ifdef DO_EXT_SWITCH
	H2K_ext_context *ext_contexts;
#endif
#endif

#ifdef HAVE_EXTENSIONS
	use_ext = arg1 & CONFIG_USE_EXT;
	arg1 &= ~CONFIG_USE_EXT;
#endif
	if ((arg1 > H2K_ID_MAX_CPUS) || (arg2 > MAX_VM_INTS)) return 0; /* bad args */
#ifdef DO_EXT_SWITCH
	alloc_size = VMBLOCK_SIZE(arg1, arg2, use_ext);
#else
	alloc_size = VMBLOCK_SIZE(arg1,arg2);
#endif
	if ((vmblock = H2K_mem_alloc(alloc_size)) == NULL) return 0;
	H2K_bzero(vmblock,alloc_size);
	ptrtmp = (u32_t)vmblock;
	BKL_LOCK();
	for (i = 1; i < H2K_ID_MAX_VMS; i++) {
		if (H2K_gp->vmblocks[i] == NULL) {
			H2K_gp->vmblocks[i] = vmblock;
			vmblock->vmidx = i;
			break;
		}
	}
	BKL_UNLOCK();
	if (i == H2K_ID_MAX_VMS) {
		H2K_mem_alloc_free(vmblock);
		return 0;  // out of vm IDs
	}
	vmblock->guestmap.raw = 0;	/* EJP: probably a bad default to have */
	vmblock->max_cpus = arg1;
	vmblock->num_ints = arg2;
	vmblock->trapmask = ~0;
	vmblock->tlbidxmask = ~0;
	if (me != NULL) {
		vmblock->parent = me->id;
	} else {
		/* ID of boot VM parent stays 0:0 */
	}
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
	if (use_ext && H2K_gp->info_boot_flags.boot_have_hvx) {
		vmblock->use_ext = 1;
#ifdef DO_EXT_SWITCH
		vmblock->ext_contexts = ext_contexts = (H2K_ext_context *)ptrtmp;
		for (i = 0; i < vmblock->max_cpus; i++) {
			H2K_ext_context_clear(&ext_contexts[i]);
		}
		ptrtmp += vmblock->max_cpus * sizeof(H2K_ext_context);

		if (H2K_gp->info_boot_flags.boot_ext_ok) { // V2X currently off; can do ext switch
			vmblock->do_ext = 1;
			H2K_hvx_poweron();
		}
#endif
	}
#endif

	vmblock->intinfo = (H2K_vm_int_opinfo_t *)ptrtmp; 
	ptrtmp += INTINFO_SPACE(vmblock->num_ints);
	H2K_vm_int_intinfo_init(vmblock,vmblock->num_ints);

	vmblock->percpu_mask = NULL;
	vmblock->pending = NULL;
	vmblock->enable = NULL;

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
		for (i = 0; i < vmblock->max_cpus; i++, mask += MASK_WORDS_PERCPU(vmblock->num_ints)) {
			masks[i] = mask;
		}
	}
	/* allocate hw ints and set invalid */
	vmblock->int_v2p = (physint_t *)ptrtmp;
	ptrtmp += PHYSINT_SPACE(vmblock->num_ints + PERCPU_INTERRUPTS); // shared + per-cpu
	return vmblock->vmidx;
}

static u32_t H2K_config_vmblock_init_pmap_type(H2K_vmblock_t *vmblock, u32_t vm, u32_t unused, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_offset_t offset;
	/* EJP: FIXME: simplify? */
	if (arg2 == H2K_ASID_TRANS_TYPE_OFFSET) {
		/* arg1 has offset_pages[20]:xwru[4]:cccc[4]:size[4].  For negative offset wrap around by
			 using (0xffffffff - offset) */

		offset.raw = arg1;

		/* Make sure offset is aligned to a valid page size, and size field matches */
		if (offset.size < SIZE_4K
				|| offset.size > SIZE_16M
				|| offset.pages % (0x1 << (2 * offset.size)) != 0) {
			return 0;
		}

		vmblock->guestmap.ptb = offset.raw;
		vmblock->guestmap.fields.type = arg2;
		vmblock->guestmap.fields.vmid = vmblock->parent.vmidx;
		vmblock->fence_lo = 0;
		vmblock->fence_hi = -1; // uninitialized, denies all mem  EJP: FIXME: is that true? Is it signed?
		return vmblock->vmidx;
	}

	if (arg2 >= H2K_ASID_TRANS_TYPE_XXX_LAST) return 0; // bad type

	if (arg1 == 0) {
		/* use ptb from current thread as pmap by default */
		vmblock->guestmap = H2K_gp->asid_table[me->ssr_asid];
	} else {
		/* EJP: FIXME: this seems broken */
		vmblock->guestmap.fields.type = arg2;
		vmblock->guestmap.fields.vmid = vmblock->parent.vmidx;
		vmblock->guestmap.ptb = arg1;
	}
	return vmblock->vmidx;
}

static u32_t H2K_config_vmblock_init_set_fences(H2K_vmblock_t *vmblock, u32_t vm, u32_t unused, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_offset_t offset;
	/* Ensure that fences are at given page boundaries */
	offset = vmblock->phys_offset;
	/* EJP: FIXME: should probably be in page numbers anyway to allow >32 bit pa */
	arg1 >>= PAGE_BITS;
	arg2 >>= PAGE_BITS;
	if (((arg1|arg2) & ((1<<(2*offset.size))-1)) != 0) return 0;
	vmblock->fence_lo = arg1;
	vmblock->fence_hi = arg2;
	return vmblock->vmidx;
}

static u32_t H2K_config_vmblock_init_prio_trapmask(H2K_vmblock_t *vmblock, u32_t vm, u32_t unused, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	if (arg1 > MAX_PRIO) return 0; /* bad arg */
	vmblock->bestprio = (u8_t)arg1;
	vmblock->trapmask = (u32_t)arg2;
	if (vmblock->bestprio > 200) vmblock->tlbidxmask = 0x0f; /* EJP: KLUDGE */
	return vmblock->vmidx;
}

static u32_t H2K_config_vmblock_init_map_phys_intr(H2K_vmblock_t *vmblock, u32_t vm, u32_t unused, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_physint_config_t config_int;
	physint_t physint;
	u32_t virt_int;
	H2K_id_t id;
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
				 || (virt_int == H2K_TIME_GUESTINT)
				 || (physint == H2K_gp->timer_intnum)
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
}

typedef u32_t (*H2K_config_vmblock_init_helper_t)(
	H2K_vmblock_t *vmblock,
	u32_t vm,
	u32_t unused,
	u32_t arg1,
	u32_t arg2,
	H2K_thread_context *me);

static const H2K_config_vmblock_init_helper_t H2K_config_vmblock_init_helpers[NUM_OPS] = {
	[SET_PMAP_TYPE] = H2K_config_vmblock_init_pmap_type,
	[SET_FENCES] = H2K_config_vmblock_init_set_fences,
	[SET_PRIO_TRAPMASK] = H2K_config_vmblock_init_prio_trapmask,
	[SET_CPUS_INTS] = H2K_config_vmblock_init_cpus_ints,
	[MAP_PHYS_INTR] = H2K_config_vmblock_init_map_phys_intr,
};

/* Should a parent be a parent vm or a parent context? */

/* initialize vm description */
u32_t H2K_trap_config_vmblock_init(u32_t unused, u32_t vm, u32_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me)
{
	H2K_vmblock_t *vmblock = NULL;

	/* Get vmblock initialized by previous SET_CPUS_INTS */
	if (op >= NUM_OPS) return 0;
	if (op != SET_CPUS_INTS) {
		if (vm >= H2K_ID_MAX_VMS) return 0;
		else if ((vmblock = H2K_gp->vmblocks[vm]) == NULL) return 0; /* NO SUCH VM */
		if (vmblock->num_cpus > 0) return 0; // fail: VM RUNNING
		/* FIXME: H2K_init_setup_bootvm() calls with me == NULL */
		if ((me != NULL) && (me->id.vmidx != vmblock->parent.vmidx)) return 0; // fail: call not from parent VM
	}
	return H2K_config_vmblock_init_helpers[op](vmblock,vm,0,arg1,arg2,me);
}

u32_t H2K_trap_config_stlb_alloc(u32_t unused, u32_t sets, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me) {

	return H2K_mem_stlb_alloc();
}
