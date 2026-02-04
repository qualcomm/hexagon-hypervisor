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
#include <h2_coproc.h>
#include <hexagon_protos.h>

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
@brief Definition of the vecaccess type.  Please do not use directly.
*/
typedef struct {
	h2_sem_t sem;
	atomic_u32_t active;
	int ext;
	int length;
	h2_coproc_type_t type;
	h2_coproc_subtype_t subtype;
	h2_cfg_unit_entry entry_type;
	unsigned int unit_mask;
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
@brief Extension and vector-length selection.
*/
typedef enum {
	H2_VECACCESS_SILVER,
	H2_VECACCESS_SILVER_MAX,
	H2_VECACCESS_HVX_64,
	H2_VECACCESS_HVX_128,
	H2_VECACCESS_HVX_MAX
} h2_vecaccess_request_t;

/**
Initialize the Vector Access type
@param[in] vacc  Address of the Vector Access structure
@param[in] req  Requested extension and vector length
@param[in] type  Coprocessor unit type
@param[in] subtype  Coprocessor unit subtype
@param[in] entry_type  Coprocessor unit context type (CFG_HVX_CONTEXTS, etc)
@param[in] unit_mask  Bitmask of units to use
@returns 0 on success; -1 on error
@dependencies None
*/
int h2_vecaccess_unit_init(h2_vecaccess_state_t *vacc, unsigned int req, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask);

/**
Initialize the Vector Access type (legacy)
@param[in] vacc  Address of the Vector Access structure
@param[in] req  Requested extension and vector length
@returns 0 on success; -1 on error
@dependencies None
*/
int h2_vecaccess_init(h2_vecaccess_state_t *vacc, unsigned int req);

/**
Get mmvector access.
@param[in] vacc		Address of the Vector Access structure
@returns Index and length of the acquired context.  Negative index on error.
@dependencies None
*/
h2_vecaccess_ret_t h2_vecaccess_acquire(h2_vecaccess_state_t *vacc);

/**
Release mmvector access.
@param[in] vacc  Address of the Vector Access structure
@param[in] idx   Index of the context
@returns 0 on success or negative value on error
@dependencies None
*/
int h2_vecaccess_release(h2_vecaccess_state_t *vacc, int idx);

/** @} */

#endif
