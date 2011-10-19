/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <termios.h>

using namespace std;
#include "stdlib.h"
#include "stdio.h"
#include "HexagonTypes.h"
#include "HexagonWrapper.h"
#include "uart.h"

struct termios rxtermios;

UART::UART(HexagonWrapper *pSimHandle, unsigned int base_address, int intnum, FILE *txfp_arg, FILE *rxfp_arg) {
	fprintf(stderr,"Cosim UART:  init called\n");
	fprintf(stderr,"Cosim UART:  base_address = 0x%08x\n", base_address);
	fprintf(stderr,"Cosim UART:  intnum = %d\n", intnum);

	issHandler = pSimHandle;
//	issHandler->RegisterToCanStep(360, this);
	//fprintf(stderr,"Cosim UART: about to call CycleToTime\n");
	//issHandler->CycleToTime(360, &time, &units);
	//fprintf(stderr,"Cosim UART: Called CycleToTime\n");
	//issHandler->AddTimedCallback((void *)this, time, units, poll_callback);
	issHandler->AddTimedCallback((void *)this, 1000, HEX_NANOSEC, poll_callback);

	base_address_rw_regs = base_address;
	interrupt_number = intnum;
	txfp = txfp_arg;
	rxfp = rxfp_arg;
	/*
	 * If we have a receive pty, make it raw, nonblocking.
	 */
	if (rxfp != NULL) {
		struct termios t;
		int fd = fileno(rxfp);
		tcgetattr(fd, &rxtermios);
		memcpy(&t, &rxtermios, sizeof(struct termios));
		t.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
		tcsetattr(fd, TCSANOW, &t);
		fcntl(fd, F_SETFL, FNDELAY);
	}

//	issHandler->PluginMem(this, base_address_rw_regs, base_address_rw_regs+(NUMBER_RW_REGS<<2));
	issHandler->AddBusAccessCallback(this, base_address_rw_regs, base_address_rw_regs+(NUMBER_RW_REGS<<2)-1, access_callback);

	//Defining the R Reg addresses 
#define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR)
#define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR)
#define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) \
  ADDRESS,
  unsigned int tR_REG_ADDRS[NUMBER_R_REGS] = { 
#include "regs.def"
  };
#undef DEF_RW_REG
#undef DEF_R_REG
#undef DEF_REG_FIELD

	for(int i=0;i<NUMBER_R_REGS;i++) {
	R_REG_ADDRS[i] = tR_REG_ADDRS[i];
	}

  // Define the reg field shifts
  #define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR) \
    START_POS,
	unsigned int treg_field_shifts[NUMBER_REG_FIELDS] = {
	#include "regs.def"
	};
  #undef DEF_REG_FIELD
  #undef DEF_RW_REG
  #undef DEF_R_REG

	for(int i=0;i<NUMBER_REG_FIELDS;i++) {
		reg_field_shifts[i] = treg_field_shifts[i];
	}

  // Define the reg field masks
  #define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR) \
    (1<<LENGTH)-1,
	unsigned int treg_field_masks[NUMBER_REG_FIELDS] = {
	#include "regs.def"
	};
  #undef DEF_REG_FIELD
  #undef DEF_RW_REG
  #undef DEF_R_REG

	for(int i=0;i<NUMBER_REG_FIELDS;i++) {
		reg_field_masks[i] = treg_field_masks[i];
	}

	// Initialize the registers
	bzero(RW_REGS, 4*NUMBER_RW_REGS);
	bzero(R_REGS, 4*NUMBER_R_REGS);

	// Initialize the fifos
	tx_fifo_depth = TX_FIFO_DEPTH;
	rx_fifo_depth = RX_FIFO_DEPTH;

	// Initialize the shift registers
	//  tx_sr_unsent_bits = 0;
	//  tx_sr = 0xff;
	tx_npacked = 0;
	tx_send_data_packed = 0xff;

	// Initialize the rx state machine
	rx_state = RX_SLEEP;
}

UART::~UART() {

	int rfd = fileno(rxfp);
	tcsetattr(rfd, TCSANOW, &rxtermios);
	
	fclose(txfp);
	fclose(rxfp);

}

int UART::get_r_reg_num(unsigned int addr) {
	for(int i=0;i<NUMBER_R_REGS;i++) {
		if(R_REG_ADDRS[i] == addr) {
			return i;
		}
	}
	return -1;
}

signed char UART::get_tx_parity(unsigned int data) {
	signed char parity = -1;
	unsigned char ones4[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
	unsigned char nones;
	
	unsigned char parity_type = GET_REG_FIELD_VAL(MR2, PARITY_MODE);
	
	// No parity
	if(parity_type == 0) {
		return -1;
	}
	
	nones = ones4[(data & 0xf)] + ones4[ (data >> 4) & 0xf];
	
	// Odd parity
	if(parity_type == 1) {
		if(nones & 0x1) {
			parity = 0;
		} else {
			parity = 1;
		}
	}
	
	// Even parity
	if(parity_type == 2) {
		if(nones & 0x1) {
			parity = 1;
		} else {
			parity = 0;
		}
	}
	
	// Space pariry
	if(parity_type == 3) {
		parity = 0;
	}
	
	return parity;
}

void UART::transmit_interrupt() {
	int txlev_intr_enable = GET_REG_FIELD_VAL(IMR, TXLEV);
	unsigned int tfwr;
	
	if(txlev_intr_enable == 0) {
		return;
	}

	tfwr = GET_REG_FIELD_VAL(TFWR, TFW);
	if(tx_fifo.size() <= tfwr) {
		SET_R_REG_FIELD_VAL(ISR, TXLEV, 1);
		R_REGS[MISR] = RW_REGS[IMR] & R_REGS[ISR] & 0x1bf;
		if (R_REGS[MISR] != 0)
//			issHandler->GenerateInterrupt(interrupt_number);
			issHandler->SetInterrupt(interrupt_number, INT_PIN_ASSERT);
	}
}

void UART::transmit() {

	unsigned char tx_sr;

//	fprintf(stderr,"Cosim UART:  transmit called\n");
//	fprintf(stderr,"Cosim UART:  transmit tx_fifo.size() = %d\n",tx_fifo.size());

	if(tx_fifo.empty() == false) {
		tx_sr = tx_fifo.front();
		tx_fifo.pop();
	} else {
		return;
	}
  	

	fwrite((const void *)&tx_sr, sizeof(unsigned char), 1, txfp);
	fsync(fileno(txfp));

	if(GET_REG_FIELD_VAL(CR, UART_TX_DISABLE)){
		SET_RW_REG_FIELD_VAL(CR, UART_TX_EN, 0);
		SET_RW_REG_FIELD_VAL(CR, UART_TX_DISABLE, 0);    
	}

	if(tx_fifo.size() < tx_fifo_depth) {
		SET_R_REG_FIELD_VAL(SR, TXRDY, 1);
	} else {
		SET_R_REG_FIELD_VAL(SR, TXRDY, 0);
	}
}

void UART::receive_interrupt() {
	int rxlev_intr_enable = GET_REG_FIELD_VAL(IMR, RXLEV);
	unsigned int rfwr;

	if(rxlev_intr_enable == 0) {
		return;
	}

	rfwr = GET_REG_FIELD_VAL(RFWR, RFW);
	if(rx_fifo.size() > rfwr) {
		SET_R_REG_FIELD_VAL(ISR, RXLEV, 1);
		R_REGS[MISR] = RW_REGS[IMR] & R_REGS[ISR] & 0x1bf;
		if (R_REGS[MISR] != 0)
//			issHandler->GenerateInterrupt(interrupt_number);
			issHandler->SetInterrupt(interrupt_number, INT_PIN_ASSERT);
	}
}

void UART::receive() {
	unsigned char rx_sr;
	int trx_sr;
	int fret;

//	fprintf(stderr,"Cosim UART:  receive called\n");

	// Check if the receiver needs to be disabled
	if(GET_REG_FIELD_VAL(CR, UART_RX_DISABLE)) {
		SET_RW_REG_FIELD_VAL(CR, UART_RX_EN, 0);
		SET_RW_REG_FIELD_VAL(CR, UART_RX_DISABLE, 0);
		return;
	}

	fret = fread(&trx_sr, sizeof(unsigned char), 1, rxfp);

	if(fret) {
		rx_sr = (unsigned char) trx_sr;
		rx_fifo.push(rx_sr);
	}

	// Set the appropriate control signals
	if(rx_fifo.size() == rx_fifo_depth) {
		SET_R_REG_FIELD_VAL(SR, RXFULL, 1);
	} else {
		SET_R_REG_FIELD_VAL(SR, RXFULL, 0);
	}

	if(rx_fifo.size() > 0) {
		SET_R_REG_FIELD_VAL(SR, RXRDY, 1);
	} else {
		SET_R_REG_FIELD_VAL(SR, RXRDY, 0);
	}

}

//bool UART::bCanStep() {
void UART::poll(void) {

	if(GET_REG_FIELD_VAL(CR, UART_TX_EN)) {
		transmit();
	}  
	if(GET_REG_FIELD_VAL(CR, UART_TX_EN)) {
		transmit_interrupt();
	}
	if(GET_REG_FIELD_VAL(CR, UART_RX_EN)) {
		receive();
	}
	if(GET_REG_FIELD_VAL(CR, UART_RX_EN)) {
		receive_interrupt();
	}
}

void poll_callback(void *handle) {
	UART *puart = (UART *) handle;
	puart->poll();
}

//void UART::DirectMemWrite(int addr, int lengthInBytes, char* data, 
//                          int tnum, bool isLockedAccess, bool *bSuccess) {
void UART::write(int addr, int lengthInBytes, HEX_1u_t *data) {
	unsigned int intAddr = (addr - base_address_rw_regs) >> 2;
	unsigned int tval, writeVal=1;
	unsigned int tfwr;
	unsigned char tx_fifo_data;

//	fprintf(stderr,"Cosim UART:  DirectMemWrite called\n");

	memcpy(&tval, data, 4);  
//	fprintf(stderr,"Cosim UART:  @0x%08x = 0x%08x\n", addr,tval);

	if(writeVal) {
		RW_REGS[intAddr] = tval;
	}

	// Check writing into the TX FIFO
	if (intAddr == TF) {
//	    fprintf(stderr,"Cosim UART:  wrote to TX fifo\n");
//	    fprintf(stderr,"Cosim UART:  tx_fifo.size=%d\n",tx_fifo.size());
//	    fprintf(stderr,"Cosim UART:  tx_fifo_depth=%d\n",tx_fifo_depth);
	}

	if( (intAddr == TF) && (tx_fifo.size() <= tx_fifo_depth) ) {
		tx_fifo_data = tval & 0xff;
		tx_fifo.push(tx_fifo_data);
		tx_fifo_data_present = 1;
		tfwr = GET_REG_FIELD_VAL(TFWR, TFW);
		if(tx_fifo.size() > tfwr) {
			SET_R_REG_FIELD_VAL(ISR, TXLEV, 0);
			R_REGS[MISR] = RW_REGS[IMR] & R_REGS[ISR] & 0x1bf;
			if (R_REGS[MISR] == 0)
				issHandler->ClearInterrupt(interrupt_number);
		}
	}

	/*
	 * If the interrupt mask changed, check against current
	 * source values to see if we need to clear the input to
	 * the CPU.
	 */
	if (intAddr == IMR) {
		R_REGS[MISR] = RW_REGS[IMR] & R_REGS[ISR] & 0x1bf;
		if (R_REGS[MISR] == 0)
			issHandler->ClearInterrupt(interrupt_number);
		else
			issHandler->SetInterrupt(interrupt_number, INT_PIN_ASSERT);
	}

	// Check if any UART command needs to be executed
	if(intAddr == CR) {
		decode_command();
	}
}

void UART::decode_command() {
	unsigned char cmd;

	cmd = GET_REG_FIELD_VAL(CR, CHANNEL_COMMAND);
	switch(cmd) {
		case 0: 
			break;
		case 1:
			// reset receiver
			rx_state = RX_SLEEP;
			while(!(rx_fifo.empty())) {
				rx_fifo.pop();
			}
			break;

		case 2:
			// reset transmitter
			while(!(tx_fifo.empty())) {
				tx_fifo.pop();
			}
			//    tx_sr_unsent_bits = 0;
			break;

		default:
			// Not handling other commands right now
			break;
	}
}

//void UART::DirectMemRead(int addr, int lengthInBytes, char* data,
//                         int threadno, bool isLockedAccess) {
void UART::read(int addr, int lengthInBytes, HEX_1u_t *data) {
	unsigned int tval;
	int r_reg_pos;
	unsigned int intAddr = addr - base_address_rw_regs;
	unsigned int rfwr;

	r_reg_pos = get_r_reg_num(intAddr);

	if(r_reg_pos >= 0) {
		tval = R_REGS[r_reg_pos];

		if(r_reg_pos == RF) {
		    if (!(rx_fifo.empty())) {
			tval = rx_fifo.front();
			rx_fifo.pop();
			rfwr = GET_REG_FIELD_VAL(RFWR, RFW);
			if(rx_fifo.size() <= rfwr) {
			    SET_R_REG_FIELD_VAL(ISR, RXLEV, 0);
			    R_REGS[MISR] = RW_REGS[IMR] & R_REGS[ISR] & 0x1bf;
			    if (R_REGS[MISR] == 0)
				issHandler->ClearInterrupt(interrupt_number);
			}
		    	if (rx_fifo.empty())
			    SET_R_REG_FIELD_VAL(SR, RXRDY, 0);
		    } else {
			tval = 0;
		    }
		}

	} else {
		tval = RW_REGS[intAddr];
	}
  
	memcpy(data, &tval, 4);
}

HEXAPI_TransactionStatus access_callback(void *handle, HEX_PA_t address, HEX_4u_t numBytes, HEX_1u_t* dataptr, HEX_4u_t reqID, HEXAPI_BusAccessType bat, HEX_4u_t tnum, HEXAPI_BusBurstType bt) {
	UART *puart = (UART *) handle;
	if (bat == HEX_DEBUG_READ) bat = HEX_DATA_READ;
	if (bat == HEX_DEBUG_WRITE) bat = HEX_DATA_WRITE;
	if (bat == HEX_DATA_READ) {
		fprintf(stderr,"UART: read from 0x%016llx\n",address);
		puart->read(address, numBytes, dataptr);
	} else {
		fprintf(stderr,"UART: write to 0x%016llx\n",address);
		puart->write(address, numBytes, dataptr);
	}
	return TRANSACTION_SUCCESS;
}

int str2argvargc(char my_string[],
                       int *nsoas_int,
                       char ***soas_int) {
  
	int i, j, k, nsoas;
	int state = state_space;
	int next_state;
	int argument = 0;
	int length = 0, max_length = 0;
	char **soas;

	if(my_string == NULL) {
		*nsoas_int = 0;
		*soas_int = NULL;
		return argument;
	}

	i=-1;
	do {
		i++;
		if(my_string[i] == ' ') {
			next_state = state_space;
		} else if(my_string[i] == '\0') {
			next_state = state_end;
		} else {
			next_state = state_character;
			length++;
		}
    
		if((state == state_space) && (next_state == state_character)) {
			argument++;
		}

		if(((state == state_character) && (next_state == state_space)) ||
			((state == state_character) && (next_state == state_end))) {

			if(length > max_length) {
				max_length = length;
			}
			length = 0;
		}
    
		state = next_state;

	} while(my_string[i]!='\0');

	/* Create a string with argv format */
	nsoas = argument;
	soas = (char **)calloc(nsoas, sizeof(char *));
	for(i=0;i<nsoas;i++) {
		soas[i] = (char *)calloc(max_length+1, sizeof(char));
	}
  

	i=-1;
	j=0;
	k=0;
	state = state_space;
	do {
		i++;
		if(my_string[i] == ' ') {
			next_state = state_space;
		} else if(my_string[i] == '\0') {
			next_state = state_end;
		} else {
			next_state = state_character;
		}

		if(next_state == state_character) {
			soas[j][k] = my_string[i];
			k++;
		}
    
		if(((state == state_character) && (next_state == state_space)) ||
			((state == state_character) && (next_state == state_end))) {
			soas[j][k]='\0';
			j++;
			k=0;
		}
    
		state = next_state;

	} while(my_string[i]!='\0');
	
	*nsoas_int = nsoas;
	*soas_int = soas;

	return argument;
}

extern "C" {
#include "common.h"

FILE* allocate_pty (char *mode) {
	int i;
	char pty_name[256];
	FILE *fp;

	FILE *argfile;

	for(i=1;i<256;i++) {
		sprintf(pty_name, "/dev/ptys%d", i);
	
		fp = fopen (pty_name, mode);
		if(fp != NULL) {
			argfile = fopen("cosim_tty.txt","w");
			fprintf(argfile,"/dev/ttys%d\n",i);
			fclose(argfile);
			fprintf(stderr,"Cosim UART:  opened ptys%0d\n",i);
			return fp;
		}
	}
	return NULL ;
}

void INTERFACE *RegisterCosimArgs(string &name, HexagonWrapper *simPtr, char *args) {
	int nsoas, i;
	char **soas = NULL;
	unsigned int base_address = 0x40000000;
	int interrupt_number = 1;
	FILE *txfp = NULL;
	FILE *rxfp = NULL;
    

	name = "UART";

	str2argvargc(args, &nsoas, &soas);
	i = 0;
	while(i<nsoas) {
		if(!strcmp(soas[i],"--help")) {
			fprintf(stderr, "Whoa! We are here!\n");
			exit(0);
		} else if(!strcmp(soas[i],"--baseaddress")) {
			i++;
			if(i<nsoas) {
				base_address = strtoul((soas[i]),NULL,16);
			} else {
				printf("Base address required!!!\n");
				exit(0);
			}
		} else if(!strcmp(soas[i],"--interrupt_number")) {
			i++;
			if(i<nsoas) {
				interrupt_number = atoi(soas[i]);
			} else {
				interrupt_number = 1;
			}
		} else if(!strcmp(soas[i],"--txofile")) {
		        i++;
		        if(i<nsoas) {
				if(!strcmp(soas[i],"terminal")) {
					txfp = allocate_pty("ab");
					if(txfp == NULL) {
						printf("Unable to allocate pty!!!\n");
						exit(0);
					} else {
						printf("Opening terminal for transmit!\n");
					}
				} else {
					txfp = fopen(soas[i], "wb");
					if(txfp == NULL) {
						printf("Unable to open tx file for writing!\n");
						exit(0);
					}
				}
			} else {
				printf("txofile requires a file argument!!!\n");
				exit(0);
			}
		} else if(!strcmp(soas[i],"--rxifile")) {
			i++;
			if(i<nsoas) {
				if(!strcmp(soas[i],"terminal")) {
					rxfp = allocate_pty("rb");
					if(rxfp == NULL) {
						printf("Unable to allocate pty!!!\n");
						exit(0);
					} else {
						printf("Opening terminal for receive!\n");
					}
				} else {
					rxfp = fopen(soas[i],"rb");
					if(rxfp == NULL) {
						printf("Unable to open rx file for reading!\n");
						exit(0);
					} 
				}
			} else {
				printf("rxifile requires a file argument!!!\n");
				exit(0);
			}
	
		} else if (!strcmp(soas[i],"--pty")) {
			//i++;
			//rxfp = fopen(soas[i],"rb+");
			rxfp = allocate_pty("rb+");
			if (!rxfp) {
				fprintf(stderr,"ERROR:  unable to allocate pty\n");
				exit(0);
			}
			txfp = rxfp;
		} else {
			printf("Unknown option %s\n",
			soas[i]);
			exit(0);
		}
		i++;
	}
		
	return new UART(simPtr, base_address, interrupt_number, txfp, rxfp);
}

void INTERFACE UnRegisterCosim(void *me) {
	delete (UART *) me;
}

char * INTERFACE GetCosimVersion(void) {
	return (char *) HEXAGON_WRAPPER_VERSION;
}
  
}
