/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_LIFO_H
#define QURT_LIFO_H
/**
  @file qurt_lifo.h

  @brief
  Provide lock free LastInFirstOut algorithm, which can be used in a
  variety of situations for allocation/free fixed size buffer    
  This implementation will touch the first word of your FREED buffer. Even
	though it doesn't matter how you use it when it is allocated, you might want
	to be a bit careful not to put your MAGIC number as the first field.
	Because it will not hold the magic value for "freed"

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

	

void * qurt_lifo_pop(void *freelist);

 

void qurt_lifo_push(void *freelist, void *buf);

void qurt_lifo_remove(void *freelist, void *buf);
#endif /* QURT_LIFO_H */

