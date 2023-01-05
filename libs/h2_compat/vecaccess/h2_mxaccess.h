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
#include <hexagon_protos.h>

/**
@brief Definition of the mxaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
} h2_mxaccess_state_t;

/**
Initialize the MX Access type.
@param[in] mxacc  Address of the MX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_mxaccess_init(h2_mxaccess_state_t *mxacc) {

	h2_sem_init_val(&mxacc->sem, h2_info(INFO_HMX_INSTANCES));
	return 0;
}

/**
Get MX access.
@param[in] mxacc  Address of the MX Access structure
@returns Index of the acquired unit.  Negative index on error.
@dependencies None
*/

static inline int h2_mxaccess_acquire(h2_mxaccess_state_t *mxacc) {
	int idx;
	unsigned int old_active;
	unsigned int new_active;

	h2_sem_down(&mxacc->sem); 
	do {
		old_active = mxacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&mxacc->active, old_active, new_active) != old_active);
	return h2_hwconfig_set_hmxbits(1, idx);
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

	ret = h2_hwconfig_set_hmxbits(0, 0);
	h2_atomic_clrbit32(&mxacc->active, idx);
	h2_sem_up(&mxacc->sem);

	return ret;
}

/** @} */

#endif

