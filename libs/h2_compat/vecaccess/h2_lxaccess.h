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
#include <hexagon_protos.h>

/**
@brief Definition of the lxaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
} h2_lxaccess_state_t;

/**
Initialize the LX Access type.
@param[in] lxacc  Address of the LX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_lxaccess_init(h2_lxaccess_state_t *lxacc) {

	h2_sem_init_val(&lxacc->sem, h2_info(INFO_HLX_CONTEXTS));
	lxacc->active = 0;
	return 0;
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
	res = h2_hwconfig_set_hlxbits(1, idx);
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

	ret = h2_hwconfig_set_hlxbits(0, 0);
	h2_atomic_clrbit32(&lxacc->active, idx);
	h2_sem_up(&lxacc->sem);

	return ret;
}

/** @} */

#endif

