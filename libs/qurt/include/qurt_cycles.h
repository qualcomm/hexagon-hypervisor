/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_CYCLES_H
#define QURT_CYCLES_H 1
/**
  @file qurt_cond.h
  @brief  Prototypes of Kernel pcycle API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

	
	 

static inline void qurt_profile_reset_idle_pcycles (void) {}

static inline unsigned long long int qurt_profile_get_thread_pcycles(void)
{
	return h2_get_pcycles();
}

static inline unsigned long long int qurt_profile_get_thread_tcycles(void) 
{ 
        return qurt_profile_get_thread_pcycles()/3; 
}
	

static inline unsigned long long int qurt_get_core_pcycles(void)
{
	return h2_get_core_pcycles();
}

static inline void qurt_profile_get_idle_pcycles (unsigned long long *pcycles)
{
	int i;
	for (i = 0; i < 4; i++) {
		pcycles[i] = h2_waitcycles(i);
	}
}

static inline void qurt_profile_get_threadid_pcycles (int thread_id, unsigned long long  *pcycles)
{
	*pcycles = 0;
}

static inline void qurt_profile_reset_threadid_pcycles (int thread_id)
{
	/* DO NOTHING */
}

static inline void qurt_profile_enable (int enable) {}

#endif

