/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <qurt.h>
#include "atomic_ops.h"
#include <hexagon_protos.h>

/*
 * EJP: FIXME: needs rewrite
 *
 * For V4, we have a single L2 interrupt controller.
 */

/*

Legacy QURT interrupt->signal support

From what I can gather:

Supported an L2 interrupt controller
Only one sigset per interrupt
Only one signal per sigset

*/

#define MAX_SYS_INTERRUPTS 256

typedef union {
	struct {
		qurt_signal_t *signal_ptr;
		int signal_mask;
	};
	unsigned long long int raw;
} qurt_interrupt_table_entry_t;

typedef union {
	struct {
		unsigned int count;
		unsigned int pcycle;
	};
	unsigned long long int raw;
} qurt_interrupt_table_info_t;

static qurt_interrupt_table_entry_t qurt_int_sigsets[MAX_SYS_INTERRUPTS] __attribute__((aligned(64)));
static qurt_interrupt_table_info_t qurt_int_info[MAX_SYS_INTERRUPTS] __attribute__((aligned(64)));

int qurt_dummy(int intnum)
{
	return 0;
}

int qurt_interrupt_raise_l1(unsigned int interrupt_num)
{
	return h2_hwconfig_hwintop(HWCONFIG_HWINTOP_RAISE,interrupt_num,0);
}

#define io_read_32(base,offset)		(*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset))
#define io_write_32(base,offset,data)	*(volatile unsigned long *) ((unsigned long) base + (unsigned long) offset) = data

void l2_controller_init(void)
{
}

unsigned int qurt_interrupt_register(int int_num, qurt_signal_t *int_signal, int signal_mask)
{
	qurt_interrupt_table_entry_t entry;
	int_num += 32;
	entry.signal_ptr = int_signal;
	entry.signal_mask = signal_mask;
	qurt_int_sigsets[int_num].raw = entry.raw;
	qurt_int_info[int_num].raw = 0ULL;
	qurt_signal_clear(qurt_int_sigsets[int_num].signal_ptr,
		qurt_int_sigsets[int_num].signal_mask);
	return QURT_EOK;
}

unsigned int qurt_interrupt_deregister(int int_num)
{
	int_num += 32;
	qurt_signal_set(qurt_int_sigsets[int_num].signal_ptr,SIG_INT_ABORT);
	qurt_int_sigsets[int_num].signal_ptr = 0;
	return 0;
}

void rtos_set_interrupt(int interrupt_num)
{
	qurt_interrupt_raise(interrupt_num);
}

int qurt_interrupt_acknowledge(int int_num)
{
	h2_hwconfig_hwintop(HWCONFIG_HWINTOP_ENABLE,int_num+32,0);
	return QURT_EOK;
}

