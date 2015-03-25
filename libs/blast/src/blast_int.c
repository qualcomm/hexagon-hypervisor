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

#if 0
#define SIRC_COUNT	6

//  Modem FW uses this, so use this for incoming registrations/deregs
//  SIRC0 starts at 768, SIRC1 starts at 800.
#define HW_IRQ_SIRC(sirc,idx) ((24+sirc)*32 + idx)

#define SIRC0_L1_INT 24
#define SIRC_TO_L1_INT(SIRC)	(SIRC0_L1_INT + SIRC)
#define L1_TO_SIRC(intnum)	(intnum - SIRC0_L1_INT)

// a;klasdfj;klasgjkl;jasdkl;fj they moved the SIRC AGAIN
#define SIRC0_BASE		0xb4490000
#define SIRC_BASE_OFFSET	0x400
#define SIRC_TO_IO_BASE(SIRC)	(SIRC0_BASE + SIRC*SIRC_BASE_OFFSET)

//  Can probably be discarded after init, unless we do some crazy runtime re-initialization
unsigned long SIRC_IRQ_TYPE[SIRC_COUNT] =	{ 0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff };
unsigned long SIRC_IRQ_POLARITY[SIRC_COUNT] =	{ 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000 };

#define L2_INT_ENABLE_OFFSET		0x00
#define L2_INT_ENABLE_CLEAR_OFFSET	0x04
#define L2_INT_ENABLE_SET_OFFSET	0x08 
#define L2_INT_TYPE_OFFSET		0x0c
#define L2_INT_POLARITY_OFFSET		0x10
#define L2_IRQ_STATUS_OFFSET		0x14
#define L2_INT_CLEAR_OFFSET		0x18
#define L2_SOFT_INT_OFFSET		0x1c
#define L2_IRQ_PENDING_OFFSET		0x20

#define SIRC0_HANDLER_BASE 32
#define SIRC_TO_HANDLER_BASE(SIRC)	(SIRC0_HANDLER_BASE + SIRC*32)

#define MAX_SYS_INTERRUPTS (MAX_INTERRUPTS + 32 * SIRC_COUNT)
#endif

/*  

Legacy QURT interrupt->signal support  

From what I can gather:

Supported an L2 interrupt controller
Only one sigset per interrupt
Only one signal per sigset

*/

#define MAX_SYS_INTERRUPTS 512

typedef struct {
	h2_anysignal_t *signal_ptr;
	int signal_mask;
} qurt_interrupt_table_entry_t;

qurt_interrupt_table_entry_t int_sigsets[MAX_SYS_INTERRUPTS];

//  Fast interrupt handler
int qurt_int2signal(int intnum)
{
	//  maybe need to check if it's been registered first...
	//  safe to call sigset from within fastint context
	h2_anysignal_set(int_sigsets[intnum].signal_ptr, 
	int_sigsets[intnum].signal_mask);
	//  Interrupt automatically ack'd by H2 when we leave
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

int qurt_register_interrupt(int int_num, 
	h2_anysignal_t *int_signal, int signal_mask)
{
	int_num += 32;
	if (unlikely(int_num == RESCHED_INT)) {
		return 1;
	}  

	int_sigsets[int_num].signal_ptr = int_signal;
	int_sigsets[int_num].signal_mask = signal_mask;
	h2_anysignal_clear(int_sigsets[int_num].signal_ptr,
		int_sigsets[int_num].signal_mask);
	h2_register_fastint(int_num,qurt_int2signal);
	return 0;
}

unsigned int qurt_deregister_interrupt(int int_num)
{
	int_num += 32;
	h2_anysignal_set(int_sigsets[int_num].signal_ptr,1<<SIGNAL_INT_ABORT);
	//  May need to make this atomic or use a lock or something
	//h2_register_fastint(int_num,qurt_dummy);
	h2_deregister_fastint(int_num);
	//  wouldn't need to do this if code didn't check it in the first place
	int_sigsets[int_num].signal_ptr = 0;  
	return 0;
}

unsigned int qurt_interrupt_status(int int_num, int *status)
{
	assert(0);
	return EOK;
}

unsigned int qurt_interrupt_clear(int int_num)
{
	assert(0);
	return EOK;
}
