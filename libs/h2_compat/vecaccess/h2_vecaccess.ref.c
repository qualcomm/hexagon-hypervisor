/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_vecaccess.ref.c
 * @brief Vector Access for V60 - Implementation
 */

#include "h2_vecaccess.h"

int h2_vecaccess_unit_init(h2_vecaccess_state_t *vacc, unsigned int req, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {
	int ret;

	/* Block by default if init fails */
	h2_sem_init_val(&vacc->sem, 0);

	if ((ret = h2_coproc_init()) < 0) return ret;

	if (1 == ret) {  // old style

		unsigned long native_vlength = h2_info(INFO_HVX_VLENGTH);

		switch(req) {
		case H2_VECACCESS_SILVER:
		case H2_VECACCESS_SILVER_MAX:
			if ((ret = h2_hwconfig_vlength(H2_VECACCESS_VLENGTH_MIN)) <0) return ret;
			vacc->ext = H2_VECACCESS_EXT_SILVER;
			vacc->length = H2_VECACCESS_VLENGTH_128;
			h2_sem_init_val(&vacc->sem, h2_info(INFO_COPROC_CONTEXTS));
			break;

		case H2_VECACCESS_HVX_64:
			if ((ret = h2_hwconfig_vlength(H2_VECACCESS_VLENGTH_64)) <0) return ret;
			vacc->ext = H2_VECACCESS_EXT_HVX;
			vacc->length = H2_VECACCESS_VLENGTH_64;
			h2_sem_init_val(&vacc->sem, h2_info(INFO_COPROC_CONTEXTS));
			break;

		case H2_VECACCESS_HVX_128:
		case H2_VECACCESS_HVX_MAX:
			if ((ret = h2_hwconfig_vlength(H2_VECACCESS_VLENGTH_128)) <0) return ret;
			vacc->ext = H2_VECACCESS_EXT_HVX;
			vacc->length = H2_VECACCESS_VLENGTH_128;
			h2_sem_init_val(&vacc->sem, h2_info(INFO_COPROC_CONTEXTS) / (128 / native_vlength));
			break;

		default:
			return -1;
		}
	} else {
		switch(req) {  // still old style
		case H2_VECACCESS_SILVER:
		case H2_VECACCESS_SILVER_MAX:
			if ((ret = h2_hwconfig_vlength(H2_VECACCESS_VLENGTH_MIN)) <0) return ret;
			vacc->ext = H2_VECACCESS_EXT_SILVER;
			vacc->length = H2_VECACCESS_VLENGTH_128;
			h2_sem_init_val(&vacc->sem, h2_info(INFO_COPROC_CONTEXTS));
			break;

		case H2_VECACCESS_HVX_128:
		case H2_VECACCESS_HVX_MAX:
			if ((ret = h2_hwconfig_vlength(H2_VECACCESS_VLENGTH_128)) <0) return ret;
			vacc->ext = H2_VECACCESS_EXT_HVX;
			vacc->length = H2_VECACCESS_VLENGTH_128;
			h2_sem_init_val(&vacc->sem, h2_coproc_count(type, subtype, entry_type, unit_mask));
			break;

		default:
			return -1;
		}
	}

	vacc->active = 0;
	vacc->type = type;
	vacc->subtype = subtype;
	vacc->entry_type = entry_type;
	vacc->unit_mask = unit_mask;

	return 0;
}

int h2_vecaccess_init(h2_vecaccess_state_t *vacc, unsigned int req) {
	return h2_vecaccess_unit_init(vacc, req, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HVX_CONTEXTS, -1);
}

h2_vecaccess_ret_t h2_vecaccess_acquire(h2_vecaccess_state_t *vacc) {
	int idx, res;
	unsigned int old_active;
	unsigned int new_active;
	h2_vecaccess_ret_t ret;

	ret.idx = -1;  // error
	ret.length = 0;

	h2_sem_down(&vacc->sem); 
	do {
		old_active = vacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&vacc->active,old_active,new_active) != old_active);

	/* TURN ON VECTOR */
	res = h2_coproc_set(vacc->type, vacc->subtype, vacc->entry_type, vacc->unit_mask, idx, 1);

	if (res == 0) {
		ret.idx = idx;
		ret.length = vacc->length;
	}
	return ret;
}

int h2_vecaccess_release(h2_vecaccess_state_t *vacc, int idx) {
	int ret;

	/* TURN OFF VECTORS */
	ret = h2_hwconfig_extbits(0, 0);
	ret = h2_coproc_set(vacc->type, vacc->subtype, vacc->entry_type, vacc->unit_mask, 0, 0);
	h2_atomic_clrbit32(&vacc->active,idx);
	h2_sem_up(&vacc->sem);

	return ret;
}
