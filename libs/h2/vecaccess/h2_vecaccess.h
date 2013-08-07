/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_VECACCESS_H
#define H2_VECACCESS_H 1

/** @file h2_vecaccess.h
 @brief Vector Access for V60
*/
/** @addtogroup h2 
@{ */

#include <h2_sem.h>
#include <h2_atomic.h>
#include <h2_hwconfig.h>
#include <hexagon_protos.h>

#define VEC_ACCESS_NUM_CONTEXTS 2
#define VEC_ACCESS_START 4

/**
@brief Definition of the vecaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
} h2_vecaccess_t;

/**
@brief Vector-length selection.
*/
typedef enum {
	H2_VECACCESS_VLENGTH64,
	H2_VECACCESS_VLENGTH32,
	H2_VECACCESS_VLENGTH_MAX
} h2_vecaccess_vlength_t;

/**
Initialize the Vector Access type.
@param[in] vacc		Address of the Vector Access structure
@returns None
@dependencies None
*/

static inline void h2_vecaccess_init(h2_vecaccess_t *vacc) {
	vacc->active = 0; h2_sem_init_val(&vacc->sem,VEC_ACCESS_NUM_CONTEXTS);
}

/**
Get mmvector access.
@param[in] vacc		Address of the Vector Access structure
@returns Index of the acquired context or negative value on error
@dependencies None
*/

static inline int h2_vecaccess_acquire(h2_vecaccess_t *vacc) {

	int idx, ret;
	unsigned int old_active;
	unsigned int new_active;

	h2_sem_down(&vacc->sem); 
	do {
		old_active = vacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&vacc->active,old_active,new_active) != old_active);
	/* TURN ON VECTOR */
	ret = h2_hwconfig_extbits(VEC_ACCESS_START + (idx << 1), 1);
	return (ret < 0 ? ret : idx);
}

/**
Set vector length.  Call after acquiring vector.
@param[in] index  Previously-acquired index
@param[in] length  Vector length to set
@returns Index of the context or negative value on error
@dependencies None
*/

static inline int h2_vecaccess_vlength(int idx, h2_vecaccess_vlength_t length) {

	int ret;

	if (length >= H2_VECACCESS_VLENGTH_MAX) return -1;
	ret = h2_hwconfig_extbits(VEC_ACCESS_START + (idx << 1) + length, 1);
	return (ret < 0 ? ret : idx);
}

/**
Release mmvector access
@param[in] vacc		Address of the Vector Access structure
@param[in] idx		Index of the context
@returns 0 on success or negative value on error
@dependencies None
*/

static inline int h2_vecaccess_release(h2_vecaccess_t *vacc, int idx) 
{
	int ret;

	/* TURN OFF VECTORS */
	ret = h2_hwconfig_extbits(0,0);
	h2_atomic_clrbit32(&vacc->active,idx);
	h2_sem_up(&vacc->sem);

	return ret;
}

/** @} */

#endif

