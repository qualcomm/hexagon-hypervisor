/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_INFO_H
#define H2_INFO_H 1

#include <h2_common_info.h>
#include <h2_common_coproc.h>

/** @file h2_info.h
 @brief Get system configuration info
*/
/** @addtogroup h2 
@{ */

/**
Raw interface for the info trap.  Do not use.
@param[in] type  Requested configuration parameter; see h2_common_info.h
@param[in] unit  Unit index
@param[in] index  Entry index
@returns Parameter value or -1 on unknown request.
@dependencies None
*/
int h2_info_trap(info_type type, unsigned int unit, h2_cfg_unit_entry entry);

/**
Get info.
@param[in] type  Requested configuration parameter; see h2_common_info.h
@returns Parameter value or -1 on unknown request.
@dependencies None
*/
int h2_info(info_type type);

int h2_info_unit(unsigned int unit, h2_cfg_unit_entry entry);

#endif
