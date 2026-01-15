/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_mxaccess.ref.c
 * @brief HMX Access for V6x - Implementation
 */

#include "h2_mxaccess.h"

int h2_mxaccess_unit_init(h2_mxaccess_state_t *mxacc, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {
	int ret;

	if ((ret = h2_coproc_init()) < 0) return ret;

	if (1 == ret) {  // old style
		h2_sem_init_val(&mxacc->sem, h2_info(INFO_HMX_INSTANCES));
	} else {
		h2_sem_init_val(&mxacc->sem, h2_coproc_count(type, subtype, entry_type, unit_mask));
	}
	mxacc->active = 0;
	mxacc->type = type;
	mxacc->subtype = subtype;
	mxacc->entry_type = entry_type;
	mxacc->unit_mask = unit_mask;
	return 0;
}

int h2_mxaccess_init(h2_mxaccess_state_t *mxacc) {
	return h2_mxaccess_unit_init(mxacc, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, -1);
}

int h2_mxaccess_acquire(h2_mxaccess_state_t *mxacc) {
	int idx, res;
	unsigned int old_active;
	unsigned int new_active;

	h2_sem_down(&mxacc->sem); 
	do {
		old_active = mxacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&mxacc->active, old_active, new_active) != old_active);
	res = h2_coproc_set(mxacc->type, mxacc->subtype, mxacc->entry_type, mxacc->unit_mask, idx, 1);
	if (0 == res) {
		return idx;
	}
	return res;
}

int h2_mxaccess_release(h2_mxaccess_state_t *mxacc, int idx) 
{
	int ret;

	ret = h2_coproc_set(mxacc->type, mxacc->subtype, mxacc->entry_type, mxacc->unit_mask, 0, 0);
	h2_atomic_clrbit32(&mxacc->active, idx);
	h2_sem_up(&mxacc->sem);

	return ret;
}
