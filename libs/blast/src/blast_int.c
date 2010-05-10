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

//  Modem FW uses this, so use this for incoming registrations/deregs
//  SIRC0 starts at 768, SIRC1 starts at 800.

#define HW_IRQ_SIRC(sirc,idx) ((24+sirc)*32 + idx)

//  Uh, BLAST config indicates 24, but FW RTOS indicates 23 (see modem/fw/target/inc/hw_irq.h)
//  Gonna go with BLAST config.

#define SIRC0_L1_INT 24
#define SIRC1_L1_INT 25

//  SIRC0 base address = 0xab010000
//  SIRC1 base address = 0xab010400
//  l;asdkjfl;kasdfkl;asjdf simulation shows controller is actually 0xb4090000

//  See:  config/sim/blast_config.c
#define SIRC0_BASE		0xb4090000
#define SIRC1_BASE		0xb4090400
#define SIRC0_IRQ_TYPE		0xffffffff
#define SIRC1_IRQ_TYPE		0xffffffff
#define SIRC0_IRQ_POLARITY	0x00000000
#define SIRC1_IRQ_POLARITY	0x00000000

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
#define SIRC1_HANDLER_BASE 64

//  So I'm guessing let's do 32 for L1, 32 for SIRC0 and 32 for SIRC1 for now.
#define MAX_SYS_INTERRUPTS 96

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
	io_write_32(SIRC0_BASE,L2_INT_ENABLE_OFFSET,0x0);
	io_write_32(SIRC0_BASE,L2_INT_ENABLE_CLEAR_OFFSET,0xffffffff);
	io_write_32(SIRC0_BASE,L2_INT_TYPE_OFFSET,SIRC0_IRQ_TYPE);
	io_write_32(SIRC0_BASE,L2_INT_POLARITY_OFFSET,SIRC0_IRQ_POLARITY);
	io_write_32(SIRC0_BASE,L2_INT_CLEAR_OFFSET,0xffffffff);

	io_write_32(SIRC1_BASE,L2_INT_ENABLE_OFFSET,0x0);
	io_write_32(SIRC1_BASE,L2_INT_ENABLE_CLEAR_OFFSET,0xffffffff);
	io_write_32(SIRC1_BASE,L2_INT_TYPE_OFFSET,SIRC1_IRQ_TYPE);
	io_write_32(SIRC1_BASE,L2_INT_POLARITY_OFFSET,SIRC1_IRQ_POLARITY);
	io_write_32(SIRC1_BASE,L2_INT_CLEAR_OFFSET,0xffffffff);
}

//  Query the L2 interrupt controller for the particular interrupt, 
//  then call the corresponding blast_int2signal handler.

void blast_sirc_fastint(int intnum)
{
	volatile void *hw_reg=NULL; 
	int handler_base=0;
	unsigned long status;

	//  map L1 intnum to SIRC
	switch (intnum) {
		case(SIRC0_L1_INT):
			hw_reg = (void *) SIRC0_BASE;
			handler_base = SIRC0_HANDLER_BASE;
			break;
		case(SIRC1_L1_INT):
			hw_reg = (void *) SIRC1_BASE;
			handler_base = SIRC1_HANDLER_BASE;
			break;
		default:
			h2_printf("bad interrupt, no donut\n");
	}
	//  So I guess I'm basically going to do everything here that interrupt_asm.S in BLAST did

	status = io_read_32(hw_reg,L2_IRQ_STATUS_OFFSET);
	//  might do it all in one shot instead of one interrupt at a time.
	do {
		intnum = Q6_R_ct0_R(status);					//  interrupt number in the L2 controller
		io_write_32(hw_reg,L2_INT_ENABLE_CLEAR_OFFSET,1<<intnum);	//  disable int

		io_write_32(hw_reg,L2_INT_CLEAR_OFFSET,1<<intnum);	//  clear int status
		io_write_32(hw_reg,L2_INT_ENABLE_SET_OFFSET,1<<intnum);	//  enable interrupt

		blast_int2signal(handler_base + intnum);  //  might could use return value

		status = io_read_32(hw_reg,L2_IRQ_STATUS_OFFSET);	//  check status again
	} while (status);

}

int map_fw_to_h2(int intnum)
{
	if (intnum < MAX_INTERRUPTS) {
		return intnum;
	}
	else if (intnum < HW_IRQ_SIRC(1,0)) {
		return (intnum - HW_IRQ_SIRC(0,0)) + SIRC0_HANDLER_BASE;
	}
	else {
		return (intnum - HW_IRQ_SIRC(1,0)) + SIRC1_HANDLER_BASE;
	}
}

int blast_register_interrupt(int int_num, 
	h2_anysignal_t *int_signal, int signal_mask)
{
	//h2_printf("%s:  int num=%d\n",__FUNCTION__,int_num);

	if (unlikely(int_num == RESCHED_INT)) {
		return 1;
	}  

	int_num = map_fw_to_h2(int_num);
	//h2_printf("%s:  h2 int_num %d\n",__FUNCTION__,int_num);

	if (int_num >= MAX_SYS_INTERRUPTS) { 
		return 1;
	}  //  ok, seriously.

	//  L2 controller is just int_num > MAX_INTERRUPTS; int_sigsets should be fine
	//  Is this atomic thing really necessary?  Let's change this later.
	if (atomic_compare_and_set(
		(unsigned int *) &int_sigsets[int_num].signal_ptr,
		(unsigned int) 0,
		(unsigned int) int_signal)) 
	{
		int_sigsets[int_num].signal_mask = signal_mask;
		h2_anysignal_clear(int_sigsets[int_num].signal_ptr,
			int_sigsets[int_num].signal_mask);
		if (int_num < MAX_INTERRUPTS) {
			h2_register_fastint(int_num,blast_int2signal);
		}
		else if (int_num < SIRC1_HANDLER_BASE) {
			//h2_printf("%s: registered SIRC0 int_num=%d\n",__FUNCTION__,int_num-SIRC0_HANDLER_BASE);
			h2_register_fastint(SIRC0_L1_INT,blast_sirc_fastint);
			io_write_32(SIRC0_BASE,L2_INT_ENABLE_SET_OFFSET,1<<(int_num - SIRC0_HANDLER_BASE));
		}  
		else {  
			//h2_printf("%s: registered SIRC1 int_num=%d\n",__FUNCTION__,int_num-SIRC1_HANDLER_BASE);
			h2_register_fastint(SIRC1_L1_INT,blast_sirc_fastint);
			io_write_32(SIRC1_BASE,L2_INT_ENABLE_SET_OFFSET,1<<(int_num - SIRC1_HANDLER_BASE));

		}
		return 0;
	}
	return 1;
}

unsigned int blast_deregister_interrupt(int int_num)
{
	h2_anysignal_set(int_sigsets[int_num].signal_ptr,1<<SIGNAL_INT_ABORT);
	//  wouldn't need to do this if code didn't check it in the first place
	int_sigsets[int_num].signal_ptr = 0;  
	//h2_printf("%s actually called\n",__FUNCTION__);
	h2_register_fastint(int_num,blast_dummy);
	return 0;
}

