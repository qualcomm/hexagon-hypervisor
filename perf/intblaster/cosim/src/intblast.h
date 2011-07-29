/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************
# Copyright (c) $Date: 2010/09/09 00:50:48 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2010/09/09 00:50:48 $
*****************************************************************/
#ifndef H2_TEST_INTBLAST_H
#define H2_TEST_INTBLAST_H 1

#include "HexagonWrapper.h"

typedef struct
{
    HexagonWrapper *iss; //used to talk to iss core
    HEX_4u_t pin; //what pin to wiggle
    HEX_4u_t rate; //what pin to wiggle
} Blaster;

// Behavior functions
void BlasterInit(Blaster *blaster, HexagonWrapper *s);
void BlasterFree(Blaster *tc);
void BlasterReset(Blaster *blaster);

// Utility functions
int  BlasterSetup(Blaster *blaster, char *);

// Callback functions
void BlasterTick(void *timer);

#endif
