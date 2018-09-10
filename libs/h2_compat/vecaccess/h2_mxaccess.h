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
} h2_mxaccess_state_t;

/**
Initialize the MX Access type.
@param[in] mxaccacc  Address of the MX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/

static inline int h2_mxaccess_init(h2_mxaccess_state_t *mxacc) {

	int ret;

	h2_sem_init_val(&mxacc->sem, 1);
	mxacc->active = 0;
	return 0;
}

/**
Get mx access.
@param[in] mxacc  Address of the MX Access structure
@returns Index of the acquired unit (currently always 0).  Negative index on error.
@dependencies None
*/

static inline int h2_mxaccess_acquire(h2_mxaccess_state_t *mxacc) {

	h2_sem_down(&mxacc->sem); 
	return h2_hwconfig_hmxbits(1);
}

/**
Release mx access.
@param[in] mxacc  Address of the MX Access structure
@param[in] unit  Unit to release. Currently ignored
@returns 0 on success or negative value on error
@dependencies None
*/

static inline int h2_mxaccess_release(h2_mxaccess_state_t *mxacc, int unit) 
{
	int ret;

	ret = h2_hwconfig_hmxbits(0);
	h2_sem_up(&mxacc->sem);

	return ret;
}

/** @} */

#endif

