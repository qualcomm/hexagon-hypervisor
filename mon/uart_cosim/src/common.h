/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef CYGPC 
  #define INTERFACE __declspec (dllexport) 
#else
  #define INTERFACE 
#endif
#endif
