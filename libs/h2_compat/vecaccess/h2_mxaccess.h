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
int h2_mxaccess_unit_init(h2_mxaccess_state_t *mxacc, h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask);

/**
Initialize the MX Access type.
@param[in] mxacc  Address of the MX Access structure
@returns 0 on success; -1 on error
@dependencies None
*/
int h2_mxaccess_init(h2_mxaccess_state_t *mxacc);

/**
Get MX access.
@param[in] mxacc  Address of the MX Access structure
@returns Index of the acquired unit.  Negative index on error.
@dependencies None
*/
int h2_mxaccess_acquire(h2_mxaccess_state_t *mxacc);

/**
Release MX access.
@param[in] mxacc  Address of the MX Access structure
@param[in] idx  Index of unit to release
@returns 0 on success or negative value on error
@dependencies None
*/
int h2_mxaccess_release(h2_mxaccess_state_t *mxacc, int idx);

/** @} */

#endif
