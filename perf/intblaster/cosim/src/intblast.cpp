/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************
# Copyright (c) $Date: 2010/09/17 21:09:59 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2010/09/17 21:09:59 $
*****************************************************************/
#include <iostream>
#include <stdio.h>
#include "intblast.h"

#define SKIPTICKS 1000
#define ONTICKS 1000*100

void BlasterReset(Blaster *t)
{
}

void BlasterInit(Blaster *t, HexagonWrapper *pSimHandle)
{
	BlasterReset(t);
	t->iss = pSimHandle;
}

void BlasterFree(Blaster *t)
{
	free(t);
}

void BlasterTick(void *vblaster)
{
	Blaster *me = (Blaster *)vblaster;
	static int skipticks = SKIPTICKS;
	static int onticks = ONTICKS;
	if (skipticks) {
		skipticks--;
		return;
	}
	if (!onticks) {
		me->iss->SetInterrupt(me->pin+1,INT_PIN_ASSERT);
		return;
	}
	me->iss->SetInterrupt(me->pin,INT_PIN_ASSERT);
	onticks--;
}

int BlasterSetup(Blaster *me, char *args)
{
	unsigned int rate = 1000,pin = 4;
	printf("BlasterSetup: args=<%s>\n",args);
	rate = strtoul(args,&args,0);
	printf("BlasterSetup: rate=%u (ns)\n",rate);
	if (args) pin = strtoul(args,NULL,0);
	printf("BlasterSetup: pin=%u \n",pin);
	me->rate= rate;
	me->pin = pin;
	return 0;
};

extern "C" {
void INTERFACE *RegisterCosimArgs(char **name, HexagonWrapper *simPtr, char *args)
{
        Blaster *me = (Blaster *)calloc(1, sizeof(Blaster));
        BlasterInit(me, simPtr);
	BlasterSetup(me,args);

	me->iss->AddTimedCallback((void *)me, me->rate, HEX_NANOSEC, BlasterTick);
        return (void *)me;
}

char INTERFACE *GetCosimVersion()
{
	return (char *)HEXAGON_WRAPPER_VERSION;
}

void INTERFACE UnRegisterCosim(void *handle)
{
	BlasterFree((Blaster *)handle);
}

} /* EXTERN C */
