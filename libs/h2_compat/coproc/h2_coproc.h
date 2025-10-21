/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COPROC_H
#define H2_COPROC_H 1

/** @file h2_coproc.h
		@brief Get/set coprocessor config
*/
/** @addtogroup h2 
@{ */

#include <h2_hwconfig.h>
#include <hexagon_protos.h>
#include <h2_info.h>
#include <h2_common_coproc.h>

/**
Count coprocessor (context) instances
@param[in] type  Coprocessor type
@param[in] subtype  Coprocessor subtype
@param[in] entry_type  Coprocessor attribute (e.g. CFG_HVX_CONTEXTS)
@param[in] unit_mask  Bitmask of coprocessor units to be considered
@returns Number of instances on success; -1 on errpr
@dependencies None
*/
int h2_coproc_count(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask);

/**
Count coprocessor (context) instances
@param[in] type  Coprocessor type
@param[in] subtype  Coprocessor subtype
@param[in] entry_type  Coprocessor attribute (e.g. CFG_HVX_CONTEXTS)
@param[in] unit_mask  Bitmask of coprocessor units to be considered
@param[in] num  Ordinal number of instance to use (0 .. # present)
@param[in] enable  Enable (1) or disable (0) flag
@returns Number of instances on success; -1 on errpr
@dependencies None
*/
int h2_coproc_set(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask, unsigned int num, unsigned int enable);

/**
Initialize coprocessor data
@returns 0 on success; 1 to indicate absence of multi-unit configuration; -1 on error
@dependencies None
*/
int h2_coproc_init();

/** @} */

#endif
