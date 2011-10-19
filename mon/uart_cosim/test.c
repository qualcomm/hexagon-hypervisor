/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>

volatile unsigned int *uart_base = (void *)0xAB000000;

void debug() {
	printf("UART: SR=0x%x MISR=0x%x ISR=0x%x\n",uart_base[2],uart_base[4],uart_base[5]);
}

void uputc(char x)
{
	uart_base[3] = x;
	//debug();
}

int getkey_int(int intno)
{
	int key = uart_base[3];
	uputc(key);
	return 1;
}

int main() {
	h2_init();
	h2_register_fastint(5,getkey_int);
	uart_base[4] = 0x05; /* TX EN, RX EN */
	uart_base[5] = 0x10; /* RXLEV */
	uart_base[8] = 0x0; /* RFWR ... rx watermark 0, interrupt on any byte */
	while (1) {
		uputc('>');
	}
}

