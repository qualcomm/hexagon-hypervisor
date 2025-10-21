/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_MXACCESS_H
#define H2_MXACCESS_H 1

/** @file h2_mxaccess.h
 @brief HMX Access for V6x
*/
/** @addtogroup h2 
@{ */

#include <h2_sem.h>
#include <h2_atomic.h>
#include <h2_hwconfig.h>
#include <h2_coproc.h>
#include <hexagon_protos.h>

/**
@brief Definition of the mxaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	h2_coproc_type_t type;
	h2_coproc_subtype_t subtype;
	h2_cfg_unit_entry entry_type;
	unsigned int unit_mask;
} h2_mxaccess_state_t;

/**
Initialize the MX Access type.
@param[in] mxacc  Address of the MX Access structure
@param[in] type  Coprocessor unit type
@param[in] subtype  Coprocessor unit subtype
@param[in] entry_type  Coprocessor unit context type (CFG_HLX_CONTEXTS, etc)
@param[in] unit_mask  Bitmask of units to use
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_mxaccess_unit_init(h2_mxaccess_state_t *mxacc, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {

	if (1 == h2_coproc_init()) {  // old style
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

/**
Initialize the MX Access type.
@param[in] mxacc  Address of the MX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_mxaccess_init(h2_mxaccess_state_t *mxacc) {
	return h2_mxaccess_unit_init(mxacc, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HMX_CONTEXTS, -1);
}

/**
Get MX access.
@param[in] mxacc  Address of the MX Access structure
@returns Index of the acquired unit.  Negative index on error.
@dependencies None
*/

static inline int h2_mxaccess_acquire(h2_mxaccess_state_t *mxacc) {
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

/**
Release MX access.
@param[in] mxacc  Address of the MX Access structure
@param[in] idx  Index of unit to release
@returns 0 on success or negative value on error
@dependencies None
*/

static inline int h2_mxaccess_release(h2_mxaccess_state_t *mxacc, int idx) 
{
	int ret;

	ret = h2_coproc_set(mxacc->type, mxacc->subtype, mxacc->entry_type, mxacc->unit_mask, 0, 0);
	h2_atomic_clrbit32(&mxacc->active, idx);
	h2_sem_up(&mxacc->sem);

	return ret;
}

/** @} */

#endif

