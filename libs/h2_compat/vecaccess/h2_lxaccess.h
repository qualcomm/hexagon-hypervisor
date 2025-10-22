/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_LXACCESS_H
#define H2_LXACCESS_H 1

/** @file h2_lxaccess.h
 @brief HLX Access for V6x
*/
/** @addtogroup h2 
@{ */

#include <h2_sem.h>
#include <h2_atomic.h>
#include <h2_hwconfig.h>
#include <h2_coproc.h>
#include <hexagon_protos.h>

/**
@brief Definition of the lxaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	h2_coproc_type_t type;
	h2_coproc_subtype_t subtype;
	h2_cfg_unit_entry entry_type;
	unsigned int unit_mask;
} h2_lxaccess_state_t;

/**
Initialize the LX Access type.
@param[in] lxacc  Address of the LX Access structure
@param[in] type  Coprocessor unit type
@param[in] subtype  Coprocessor unit subtype
@param[in] entry_type  Coprocessor unit context type (CFG_HLX_CONTEXTS, etc)
@param[in] unit_mask  Bitmask of units to use
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_lxaccess_unit_init(h2_lxaccess_state_t *lxacc, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {
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

/**
Initialize the LX Access type.
@param[in] lxacc  Address of the LX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/
static inline int h2_lxaccess_init(h2_lxaccess_state_t *lxacc) {
	return h2_lxaccess_unit_init(lxacc, CFG_TYPE_VXU0, CFG_SUBTYPE_VXU0, CFG_HLX_CONTEXTS, -1);
}

/**
Get LX access.
@param[in] lxacc  Address of the LX Access structure
@returns Index of the acquired unit.  Negative index on error.
@dependencies None
*/
static inline int h2_lxaccess_acquire(h2_lxaccess_state_t *lxacc) {
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

/**
Release LX access.
@param[in] lxacc  Address of the LX Access structure
@param[in] idx  Index of unit to release
@returns 0 on success or negative value on error
@dependencies None
*/
static inline int h2_lxaccess_release(h2_lxaccess_state_t *lxacc, int idx) 
{
	int ret;

	ret = h2_coproc_set(lxacc->type, lxacc->subtype, lxacc->entry_type, lxacc->unit_mask, 0, 0);
	h2_atomic_clrbit32(&lxacc->active, idx);
	h2_sem_up(&lxacc->sem);

	return ret;
}

/** @} */

#endif

