/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <h2.h>
#include <max.h>
#include "atomic_ops.h"
#include <q6protos.h>

//  Maybe should have these defined elsewhere.
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

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

/*  

Legacy BLAST interrupt->signal support  

From what I can gather:

Supported an L2 interrupt controller
Only one sigset per interrupt
Only one signal per sigset

*/

typedef struct {
	h2_anysignal_t *signal_ptr;
	int signal_mask;
} blast_interrupt_table_entry_t;

blast_interrupt_table_entry_t int_sigsets[MAX_SYS_INTERRUPTS];

//  Fast interrupt handler
void blast_int2signal(int intnum)
{
	//  maybe need to check if it's been registered first...
	//  safe to call sigset from within fastint context
	h2_anysignal_set(int_sigsets[intnum].signal_ptr, 
		int_sigsets[intnum].signal_mask);
	//  Interrupt automatically ack'd by H2 when we leave
}

void blast_dummy(int intnum)
{
}

#define io_read_32(base,offset)		(*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset))
#define io_write_32(base,offset,data)	*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset) = data

void l2_controller_init(void) 
{
	unsigned long i;

	for (i=0; i<SIRC_COUNT; i++) {
		io_write_32(SIRC_TO_IO_BASE(i),L2_INT_ENABLE_OFFSET,0x0);
		io_write_32(SIRC_TO_IO_BASE(i),L2_INT_ENABLE_CLEAR_OFFSET,0xffffffff);
		io_write_32(SIRC_TO_IO_BASE(i),L2_INT_TYPE_OFFSET,SIRC_IRQ_TYPE[i]);
		io_write_32(SIRC_TO_IO_BASE(i),L2_INT_POLARITY_OFFSET,SIRC_IRQ_POLARITY[i]);
		io_write_32(SIRC_TO_IO_BASE(i),L2_INT_CLEAR_OFFSET,0xffffffff);
	}
}

int map_fw_to_sirc(int intnum) 
{
	/*  Probably should do something more intelligent than this  */
	if (intnum < MAX_INTERRUPTS) {
		// This is fatal/assertion territory
		return -1;
	}
	else if (intnum < HW_IRQ_SIRC(1,0)) {
		return 0;
	}
	else if (intnum < HW_IRQ_SIRC(2,0)) {
		return 1;
	}
	else if (intnum < HW_IRQ_SIRC(3,0)) {
		return 2;
	}
	else if (intnum < HW_IRQ_SIRC(4,0)) {
		return 3;
	}
	else if (intnum < HW_IRQ_SIRC(5,0)) {
		return 4;
	}
	else {
		return 5;
	}
}

//  Query the L2 interrupt controller for the particular interrupt, 
//  then call the corresponding blast_int2signal handler.

void blast_sirc_fastint(int intnum)
{
	unsigned long status;
	unsigned int sirc = L1_TO_SIRC(intnum);
	unsigned long hw_reg=SIRC_TO_IO_BASE(sirc);

	//  So I guess I'm basically going to do everything here that interrupt_asm.S in BLAST did

	status = io_read_32(hw_reg,L2_IRQ_STATUS_OFFSET);
	//  might do it all in one shot instead of one interrupt at a time.
	do {
		intnum = Q6_R_ct0_R(status);					//  interrupt number in the L2 controller
		io_write_32(SIRC_TO_IO_BASE(sirc),L2_INT_ENABLE_CLEAR_OFFSET,1<<intnum);	//  disable int
		io_write_32(SIRC_TO_IO_BASE(sirc),L2_INT_CLEAR_OFFSET,1<<intnum);	//  clear int status
		io_write_32(SIRC_TO_IO_BASE(sirc),L2_INT_ENABLE_SET_OFFSET,1<<intnum);	//  enable interrupt

		blast_int2signal(SIRC_TO_HANDLER_BASE(sirc) + intnum);  //  might could use return value

		status = io_read_32(hw_reg,L2_IRQ_STATUS_OFFSET);	//  check status again
	} while (status);

}

int blast_register_interrupt(int int_num, 
	h2_anysignal_t *int_signal, int signal_mask)
{
        unsigned int sirc = NULL;

	if (unlikely(int_num == RESCHED_INT)) {
		return 1;
	}  

	if (int_num >= MAX_INTERRUPTS) {
		sirc = map_fw_to_sirc(int_num);
		//  bring the intnum down from outer space and into our space
		int_num = int_num - HW_IRQ_SIRC(sirc,0) + SIRC_TO_HANDLER_BASE(sirc);
	}  //  L2 interrupt registration

	if (int_num >= MAX_SYS_INTERRUPTS) { 
		return 1;
	}  //  ok, seriously.

	//  L2 controller is just int_num > MAX_INTERRUPTS; int_sigsets should be fine
	//  Is this atomic thing really necessary?  Let's change this later.
	if (atomic_compare_and_set(
		(void *) &int_sigsets[int_num].signal_ptr,
		(unsigned int) 0,
		(unsigned int) int_signal)) 
	{
		int_sigsets[int_num].signal_mask = signal_mask;
		h2_anysignal_clear(int_sigsets[int_num].signal_ptr,
			int_sigsets[int_num].signal_mask);
		if (int_num < MAX_INTERRUPTS) {
			h2_register_fastint(int_num,blast_int2signal);
		}
		else {
			h2_register_fastint(SIRC_TO_L1_INT(sirc),blast_sirc_fastint);
			//h2_printf("%s enabling:  0x%08x + 0x%08x = 1 << %d\n",__FUNCTION__,SIRC_TO_IO_BASE(sirc), L2_INT_ENABLE_SET_OFFSET, int_num - SIRC_TO_HANDLER_BASE(sirc));
			io_write_32(SIRC_TO_IO_BASE(sirc),L2_INT_ENABLE_SET_OFFSET,1<<(int_num - SIRC_TO_HANDLER_BASE(sirc)));
		}  
		return 0;
	}
	return 1;
}

unsigned int blast_deregister_interrupt(int int_num)
{
	unsigned long sirc;
	h2_anysignal_set(int_sigsets[int_num].signal_ptr,1<<SIGNAL_INT_ABORT);
	//  May need to make this atomic or use a lock or something
	if (int_num < MAX_INTERRUPTS) {
		h2_register_fastint(int_num,blast_dummy);
		//  need a trap to mask the L1 interrupt
	}
	else {
		sirc = map_fw_to_sirc(int_num);
		int_num = int_num - HW_IRQ_SIRC(sirc,0) + SIRC_TO_HANDLER_BASE(sirc);
		io_write_32(SIRC_TO_IO_BASE(sirc),L2_INT_ENABLE_CLEAR_OFFSET,0xffffffff);
	}
	//  wouldn't need to do this if code didn't check it in the first place
	int_sigsets[int_num].signal_ptr = 0;  
	return 0;
}

