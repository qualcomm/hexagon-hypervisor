/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_INFO_H
#define H2_INFO_H 1

#include <h2_common_info.h>

/** @file h2_info.h
 @brief Get system configuration info
*/
/** @addtogroup h2 
@{ */

/**
Get info.
@param[in] type  Requested configuration parameter; one of:  build ID, boot flags, STLB configuration, SYSCFG register, REV register, subsystem base address.
@returns Paremeter value or -1 on unknown request.
@dependencies None
*/
int h2_info(info_type type);

#endif
