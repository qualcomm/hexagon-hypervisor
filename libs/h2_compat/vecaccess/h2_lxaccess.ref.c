/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_lxaccess.ref.c
 * @brief HLX Access for V6x - Implementation
 */

#include "h2_lxaccess.h"

int h2_lxaccess_unit_init(h2_lxaccess_state_t *lxacc, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {
	int ret;
	
	if ((ret = h2_coproc_init()) < 0) return ret;

	if (1 == ret) {  // old style
		h2_sem_init_val(&lxacc->sem, h2_info(INFO_HLX_CONTEXTS));
	} else {
		h2_sem_init_val(&lxacc->sem, h2_coproc_count(type, subtype, entry_type, unit_mask));
	}

	lxacc->active = 0;
	lxacc->type = type;
	lxacc->subtype = subtype;
	lxacc->entry_type = entry_type;
	lxacc->unit_mask = unit_mask;

	return 0;
}

int h2_lxaccess_init(h2_lxaccess_state_t *lxacc) {
	return h2_lxaccess_unit_init(lxacc, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HLX_CONTEXTS, -1);
}

int h2_lxaccess_acquire(h2_lxaccess_state_t *lxacc) {
	int idx, res;
	unsigned int old_active;
	unsigned int new_active;

	h2_sem_down(&lxacc->sem); 
	do {
		old_active = lxacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&lxacc->active, old_active, new_active) != old_active);
	res = h2_coproc_set(lxacc->type, lxacc->subtype, lxacc->entry_type, lxacc->unit_mask, idx, 1);
	if (0 == res) {
		return idx;
	}
	return res;
}

int h2_lxaccess_release(h2_lxaccess_state_t *lxacc, int idx) 
{
	int ret;

	ret = h2_coproc_set(lxacc->type, lxacc->subtype, lxacc->entry_type, lxacc->unit_mask, 0, 0);
	h2_atomic_clrbit32(&lxacc->active, idx);
	h2_sem_up(&lxacc->sem);

	return ret;
}
