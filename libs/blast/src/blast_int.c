/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <qurt.h>
#include <max.h>
#include "atomic_ops.h"
#include <hexagon_protos.h>

/*
 * EJP: FIXME: needs rewrite
 * 
 * For V4, we have a single L2 interrupt controller.
 * H2 now allows you to register fast interrupts 0..30 for L1 interrupts 
 * and 32..512 corresponding to L2 interrupts 0..480.
 * A fast interrupt returning a non-zero value will cause the interrupt
 * to be re-enabled at the L2 interrupt controller.  Otherwise we need
 * to ack it later EJP: FIXME: need call to re-enable later. :-)
 *
 */

/*  

Legacy QURT interrupt->signal support  

From what I can gather:

Supported an L2 interrupt controller
Only one sigset per interrupt
Only one signal per sigset

*/

#define MAX_SYS_INTERRUPTS 1024

typedef union {
	struct {
		h2_anysignal_t *signal_ptr;
		int signal_mask;
	};
	u64_t raw;
} qurt_interrupt_table_entry_t;

qurt_interrupt_table_entry_t int_sigsets[MAX_SYS_INTERRUPTS] __attribute__((aligned(128)));

//  Fast interrupt handler
static int qurt_int2signal(int intnum)
{
	//  maybe need to check if it's been registered first...
	//  safe to call sigset from within fastint context
	qurt_interrupt_table_entry_t entry;
	entry.raw = int_sigsets[intnum].raw;
	h2_anysignal_set(entry.signal_ptr, int_sigsets[intnum].signal_mask);
	return 1;
}

int qurt_dummy(int intnum)
{
	return 0;
}

#define io_read_32(base,offset)		(*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset))
#define io_write_32(base,offset,data)	*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset) = data

void l2_controller_init(void) 
{
}

unsigned int qurt_interrupt_register(int int_num, qurt_anysignal_t *int_signal, int signal_mask)
{
	qurt_interrupt_table_entry_t entry;
	int_num += 32;
	if (unlikely(int_num == RESCHED_INT)) {
		return 1;
	}
	entry.signal_ptr = int_signal;
	entry.signal_mask = signal_mask;
	int_sigsets[int_num].raw = entry.raw;
	qurt_anysignal_clear(int_sigsets[int_num].signal_ptr,
		int_sigsets[int_num].signal_mask);
	h2_register_fastint(int_num,qurt_int2signal);
	return QURT_EOK;
}

unsigned int qurt_interrupt_deregister(int int_num)
{
	int_num += 32;
	h2_anysignal_set(int_sigsets[int_num].signal_ptr,SIG_INT_ABORT);
	//  May need to make this atomic or use a lock or something
	//h2_register_fastint(int_num,qurt_dummy);
	h2_deregister_fastint(int_num);
	//  wouldn't need to do this if code didn't check it in the first place
	int_sigsets[int_num].signal_ptr = 0;  
	return 0;
}

unsigned int QURT_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;
unsigned int QURTK_MAX_HTHREADS __attribute__((section(".sdata"))) = 6;

