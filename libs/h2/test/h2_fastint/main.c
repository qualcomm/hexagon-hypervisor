/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>

#define TEST_INT 0
#define TEST_SIGMASK 0x1

char *context_space[256 * 64];

h2_anysignal_t int_sig;

void int2sig(int intnum) 
{
	h2_anysignal_set(&int_sig,TEST_SIGMASK);
}

int main()
{

	h2_init(0);
	h2_config_add_thread_storage(context_space,sizeof(context_space));
	h2_printf("Starting\n");

	
	h2_anysignal_init(&int_sig);
	h2_register_fastint(TEST_INT,int2sig);

	while (1) {
		h2_anysignal_wait(&int_sig, TEST_SIGMASK);
		h2_printf("Got Interrupt!\n");
		h2_anysignal_clear(&int_sig, TEST_SIGMASK);
	
	}

}

