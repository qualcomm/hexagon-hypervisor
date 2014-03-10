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

#define H2_VECACCESS_NUM_CONTEXTS 4  // when using shortest vlength
#define H2_VECACCESS_VLENGTH_LEAST H2_VECACCESS_VLENGTH64
#define H2_VECACCESS_START 4

/**
@brief Definition of the vecaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	int length;
} h2_vecaccess_state_t;

/**
@brief vecaccess return value.
*/
typedef union {
	struct {
		int idx;
		int length;
	};
	unsigned long long int raw;
} h2_vecaccess_ret_t;

/**
@brief Vector-length selection.
*/
typedef enum {
	H2_VECACCESS_VLENGTH_MOST,
	H2_VECACCESS_VLENGTH1,
	H2_VECACCESS_VLENGTH2,
	H2_VECACCESS_VLENGTH3,
	H2_VECACCESS_VLENGTH16,
	H2_VECACCESS_VLENGTH32,
	H2_VECACCESS_VLENGTH64,
	H2_VECACCESS_VLENGTH128,
	H2_VECACCESS_VLENGTH_MAX
} h2_vecaccess_vlength_t;

/**
Initialize the Vector Access type.
@param[in] vacc    Address of the Vector Access structure
@param[in] length  Requested vector length
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_vecaccess_init(h2_vecaccess_state_t *vacc, h2_vecaccess_vlength_t length) {

	if (length == H2_VECACCESS_VLENGTH_MOST) length = H2_VECACCESS_VLENGTH_MAX - 1;
	if (length < H2_VECACCESS_VLENGTH_LEAST || length >= H2_VECACCESS_VLENGTH_MAX) return -1;  // error

	vacc->active = 0;
	vacc->length = length;
	h2_sem_init_val(&vacc->sem, H2_VECACCESS_NUM_CONTEXTS / (1 << (length - H2_VECACCESS_VLENGTH_LEAST)));
	return 0;
}

/**
Get mmvector access.
@param[in] vacc		Address of the Vector Access structure
@returns Index and length of the acquired context.  Negative index on error.
@dependencies None
*/

static inline h2_vecaccess_ret_t h2_vecaccess_acquire(h2_vecaccess_state_t *vacc) {

	int idx, res;
	unsigned int old_active;
	unsigned int new_active;
	h2_vecaccess_ret_t ret;

	ret.idx = -1;
	ret.length = H2_VECACCESS_VLENGTH_MAX;

	h2_sem_down(&vacc->sem); 
	do {
		old_active = vacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&vacc->active,old_active,new_active) != old_active);
	/* TURN ON VECTOR */
	res = h2_hwconfig_extbits(H2_VECACCESS_START + idx, 1);
	if (res == 0) {
		ret.idx = idx;
		ret.length = vacc->length;
	}
	return ret;
}

/**
Release mmvector access.
@param[in] vacc  Address of the Vector Access structure
@param[in] idx   Index of the context
@returns 0 on success or negative value on error
@dependencies None
*/

static inline int h2_vecaccess_release(h2_vecaccess_state_t *vacc, int idx) 
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

