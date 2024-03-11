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

/*HVX*/
/* both start at ssr:xa == 0 */
#define H2_VECACCESS_EXT_SILVER 0
#define H2_VECACCESS_EXT_HVX 0

enum {
	H2_VECACCESS_VLENGTH_MIN,
	H2_VECACCESS_VLENGTH_32 = 5,
	H2_VECACCESS_VLENGTH_64,
	H2_VECACCESS_VLENGTH_128
};

/**
@brief Extension and vector-length selection.
*/
typedef enum {
	H2_VECACCESS_SILVER,
	H2_VECACCESS_SILVER_MAX,
	H2_VECACCESS_HVX_64,
	H2_VECACCESS_HVX_128,
	H2_VECACCESS_HVX_MAX
} h2_vecaccess_request_t;

/*end*/

/*HLX*/
#define H2_ELTACCESS_EXT_HLX 0

enum {
	H2_ELTACCESS_LENGTH_MIN = 11 // TODO: reconfirm value 2048 (val = log2(x), 2048=11)
};
/**
@brief Extension and vector-length selection.
*/
typedef enum {
	H2_ELTACCESS_HLX_MAX
} h2_eltaccess_request_t;

/*end*/

/**
@brief Definition of the vecaccess type.  Please do not use directly. /Used for HVX and HLX
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	int ext;
	int length;
} h2_vecaccess_state_t; /*TODO: Do we need separate structs for HLX?*/

/**
@brief vecaccess return value. /Used for HVX and HLX
*/
typedef union {
	struct {
		int idx;
		int length;
	};
	unsigned long long int raw;
} h2_vecaccess_ret_t;

/**
Initialize the Vector Access type.
@param[in] vacc  Address of the Vector Access structure
@param[in] req  Requested extension and vector length
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_vecaccess_init(h2_vecaccess_state_t *vacc, unsigned int req) {

	int ret;
	unsigned long native_vlength = h2_info(INFO_HVX_VLENGTH);

	/* Block be default if init fails */
	h2_sem_init_val(&vacc->sem, 0);

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
	vacc->active = 0;
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
	res = h2_hwconfig_extbits(vacc->ext + idx, 1);
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
	ret = h2_hwconfig_extbits(0, 0);
	h2_atomic_clrbit32(&vacc->active,idx);
	h2_sem_up(&vacc->sem);

	return ret;
}

/** @} */

//#define ARCHV  85

#ifdef HAVE_HLX
/**
Initialize the Vector Access type.
@param[in] vacc  Address of the Vector Access structure
@param[in] req  Requested extension and vector length
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_eltaccess_init(h2_vecaccess_state_t *vacc, unsigned int req) {

	unsigned long native_length = h2_info(INFO_HLX_LENGTH); //TODO: Check h2_info(INFO_HLX_LENGTH) response

	/* Block be default if init fails */
	h2_sem_init_val(&vacc->sem, 0); /*semaphore init*/

	vacc->ext = H2_ELTACCESS_EXT_HLX; //sets elt extension type
	vacc->length = H2_ELTACCESS_LENGTH_MIN; // sets elt len
	h2_sem_init_val(&vacc->sem, h2_info(INFO_HLX_CONTEXTS) / (2048 / native_length)); //TODO: What is the value for hlx

	vacc->active = 0;
	return 0;
}

/**
Get mmvector access.
@param[in] vacc		Address of the Vector Access structure
@returns Index and length of the acquired context.  Negative index on error.
@dependencies None
*/

static inline h2_vecaccess_ret_t h2_eltaccess_acquire(h2_vecaccess_state_t *vacc) {

	int idx, res;
	unsigned int old_active;
	unsigned int new_active;
	h2_vecaccess_ret_t ret;

	ret.idx = -1;
	ret.length = 0;

	h2_sem_down(&vacc->sem); //blocking sem
	do {
		old_active = vacc->active;
		idx = Q6_R_ct1_R(old_active);
		new_active = old_active | (1<<idx);
	} while (h2_atomic_compare_swap32(&vacc->active,old_active,new_active) != old_active); //get next available HLX Context
	/* TURN ON VECTOR */
	res = h2_hwconfig_set_hlxbits(vacc->ext + idx, 1);//xa3/xe3
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

static inline int h2_eltaccess_release(h2_vecaccess_state_t *vacc, int idx) 
{
	int ret;

	/* TURN OFF VECTORS */
	ret = h2_hwconfig_set_hlxbits(0, 0);//xa3/xe3
	h2_atomic_clrbit32(&vacc->active,idx);//release hlx context
	h2_sem_up(&vacc->sem); //release sem

	return ret;
}

# endif

#endif

