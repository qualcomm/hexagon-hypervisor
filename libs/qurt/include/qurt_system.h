/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_NON_SYSTEM_H
#define QURT_NON_SYSTEM_H 

/**
  @file  qurt_system.h 
  @brief Contains misc system calls

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include "qurt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

  /**@ingroup func_qurt_system_jump
    Performs a jump from the current physical address to the address specified.
    This function is useful for moving execution from the current image to another image.
   
    @param addr  Address where execution will resume.

    @return
    None.

    @dependencies
    None.
   
   */ 
void qurt_system_jump (qurt_addr_t addr);

#endif /* QURT_SYSTEM_H */
