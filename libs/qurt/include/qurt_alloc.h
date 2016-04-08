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

static inline void *qurt_malloc( unsigned int size) { return h2_malloc(size); }

static inline void *qurt_calloc(unsigned int elsize, unsigned int num) { return h2_calloc(elsize,num); }

static inline void *qurt_realloc(void *ptr,  int newsize) { return h2_realloc(ptr,newsize); }

static inline void qurt_free( void *ptr) { return h2_free(ptr); }

#endif /* QURT_ALLOC_H */

