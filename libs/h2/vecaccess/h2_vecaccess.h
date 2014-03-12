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

/**
@brief Definition of the vecaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	int ext;
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

enum {
	H2_VECACCESS_VLENGTH_32 = 5,
	H2_VECACCESS_VLENGTH_64,
	H2_VECACCESS_VLENGTH_128,
	H2_VECACCESS_VLENGTH_MAX = 0xffff,
	H2_VECACCESS_VLENGTH_END
};

enum {
	H2_VECACCESS_EXT_SILVER = 0,
	H2_VECACCESS_EXT_HVX = 4,
	H2_VECACCESS_EXT_END
};

/**
@brief Extension and vector-length selection.
*/
typedef enum {
	H2_VECACCESS_SILVER_32 = (H2_VECACCESS_EXT_SILVER << 16) + H2_VECACCESS_VLENGTH_32,
	H2_VECACCESS_SILVER = (H2_VECACCESS_EXT_SILVER << 16) + H2_VECACCESS_VLENGTH_MAX,
	H2_VECACCESS_HVX_64 = (H2_VECACCESS_EXT_HVX << 16) + H2_VECACCESS_VLENGTH_64,
	H2_VECACCESS_HVX_128 = (H2_VECACCESS_EXT_HVX << 16) + H2_VECACCESS_VLENGTH_128,
	H2_VECACCESS_HVX = (H2_VECACCESS_EXT_HVX << 16) + H2_VECACCESS_VLENGTH_MAX
} h2_vecaccess_request_t;

/**
Initialize the Vector Access type.
@param[in] vacc    Address of the Vector Access structure
@param[in] length  Requested vector length
@returns 0 on success; negative on error
@dependencies None
*/

#define EXT_LEN_LO 0
#define EXT_LEN_HI 1

static inline int h2_vecaccess_init(h2_vecaccess_state_t *vacc, h2_vecaccess_request_t req) {

	static int ext_sizes[H2_VECACCESS_EXT_END][2] =
		{
			[H2_VECACCESS_EXT_SILVER] = {H2_VECACCESS_VLENGTH_32, H2_VECACCESS_VLENGTH_32},
			[H2_VECACCESS_EXT_HVX] = {H2_VECACCESS_VLENGTH_64, H2_VECACCESS_VLENGTH_128}
		};

	unsigned int length = (unsigned int)req & 0xffff;
	unsigned int ext = req >> 16;
	int ret;

	if (ext >= H2_VECACCESS_EXT_END) {
		return -2;      // invalid extension
	}
	if (ext_sizes[ext][EXT_LEN_LO] == 0) {
		return -3;  // invalid extension
	}
		
	if (length < ext_sizes[ext][EXT_LEN_LO] || length > ext_sizes[ext][EXT_LEN_HI]) {
		return -4;  // bad length
	}

	ret = h2_hwconfig_extbits(0, 0, length);  // set vector length
	if (ret < 0) {  // error
		return ret;
	}

	vacc->active = 0;
	vacc->length = length;
	vacc->ext = ext;
	h2_sem_init_val(&vacc->sem, H2_VECACCESS_NUM_CONTEXTS / (1 << (length - ext_sizes[ext][EXT_LEN_LO])));
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
	ret.length = 0;

	h2_sem_down(&vacc->sem); 
	do {
		old_active = vacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&vacc->active,old_active,new_active) != old_active);
	/* TURN ON VECTOR */
	res = h2_hwconfig_extbits(vacc->ext + idx, 1, 0);
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
	ret = h2_hwconfig_extbits(0, 0, 0);
	h2_atomic_clrbit32(&vacc->active,idx);
	h2_sem_up(&vacc->sem);

	return ret;
}

/** @} */

#endif

