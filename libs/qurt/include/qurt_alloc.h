/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_ALLOC_H
#define QURT_ALLOC_H

/**
  @file qurt_alloc.h 
  @brief Prototypes of Kernel memory allocation API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

void *qurt_malloc( unsigned int size);

void *qurt_calloc(unsigned int elsize, unsigned int num);

void *qurt_realloc(void *ptr,  int newsize);

void qurt_free( void *ptr);

#endif /* QURT_ALLOC_H */

