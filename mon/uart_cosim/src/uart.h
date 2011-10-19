/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _UART_H_
#define _UART_H_

#include <queue>
#include "HexagonTypes.h"
#include "HexagonWrapper.h"

#define TX_FIFO_DEPTH 256
#define RX_FIFO_DEPTH 256

class UART {
 public:
  // FUNCTIONS
  UART(HexagonWrapper *pSimHandle, unsigned int base_address, int intnum, FILE *txfp_arg, FILE *rxfp_arg);
  ~UART();   
//  bool bCanStep();
  void poll(void);
  void write(int addr, int lengthInBytes, HEX_1u_t *data);
  void read(int addr, int lengthInBytes, HEX_1u_t *data);
//  void DirectMemWrite(int addr, int lengthInBytes, char* data, int tnum = -1,
//                      bool isLockedAccess = false, bool *bSuccess = NULL);
//  void DirectMemRead(int addr, int lengthInBytes, char* data,
//                     int tnum = -1, bool isLockedAccess = false);

  // ENUMS
  enum CLK_REGIMES {UART_CLK};
  
  // Defining the RW Regs 
  #define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR)
  #define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR)
  #define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) \
    REG_NAME = (ADDRESS>>2),
  enum UART_RW_REGS {
    #include "regs.def"
    NUMBER_RW_REGS};
  #undef DEF_RW_REG
  #undef DEF_R_REG
  #undef DEF_REG_FIELD
  
  /* Defining the R Regs */
  #define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR)
  #define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR)
  #define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) \
    REG_NAME,  
  enum UART_R_REGS {
    #include "regs.def"
    NUMBER_R_REGS};
  #undef DEF_RW_REG
  #undef DEF_R_REG
  #undef DEF_REG_FIELD

  // Define the reg fields
  #define DEF_RW_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_R_REG(ADDRESS, REG_NAME, R, W, CLK_REGIME, RESET_STATE, WIDTH, REG_STRING, REG_DESCR) 
  #define DEF_REG_FIELD(REG_NAME, FIELD_NAME, START_POS, LENGTH, FIELD_STRING, FIELD_DESCR) \
    REG_NAME##FIELD_NAME,
  enum UART_REG_FIELDS {
    #include "regs.def"
    NUMBER_REG_FIELDS};
  #undef DEF_REG_FIELD
  #undef DEF_RW_REG
  #undef DEF_R_REG

  // VARIABLES
  // Base address for the registers
  unsigned int base_address_rw_regs;

  // RW Registers for this UART 
  unsigned int RW_REGS[NUMBER_RW_REGS];

  // Non contiguous, inconsistent S#@%
  unsigned int R_REGS[NUMBER_R_REGS];  
  unsigned int R_REG_ADDRS[NUMBER_R_REGS];

private:
  HexagonWrapper *issHandler;
  int get_r_reg_num(unsigned int addr); 
  void transmit();
  void transmit_interrupt();
  void receive();
  void receive_interrupt();
  void decode_command();
  unsigned int reg_field_shifts[NUMBER_REG_FIELDS];
  unsigned int reg_field_masks[NUMBER_REG_FIELDS];
  
  queue <unsigned char> tx_fifo;
  unsigned int tx_fifo_depth;
  int tx_fifo_data_present;
  //  int tx_sr_unsent_bits;
  //  unsigned int tx_sr;
  signed char get_tx_parity(unsigned int data);

  FILE *txfp;
  FILE *rxfp;
  struct termios rxtermios;
  unsigned char tx_send_data_packed;
  int tx_npacked;

  queue <unsigned char> rx_fifo;
  unsigned int rx_fifo_depth;
  unsigned int rx_state;
  unsigned int rxcount;

  enum RX_STATE {RX_SLEEP, RX_START, RX_BIT_CAPTURE, RX_PARITY};
  
  int interrupt_number;
   
};

// Other defines
#define GET_REG_FIELD_VAL(REG, FIELD)                                   \
  ( (RW_REGS[REG] >> (reg_field_shifts[REG##FIELD])) & reg_field_masks[REG##FIELD])

#define SET_RW_REG_FIELD_VAL(REG, FIELD, VAL)      \
  RW_REGS[REG] = RW_REGS[REG] & (~( (reg_field_masks[REG##FIELD]) << (reg_field_shifts[REG##FIELD])) ); \
  RW_REGS[REG] = RW_REGS[REG] | (VAL << (reg_field_shifts[REG##FIELD]))

#define SET_R_REG_FIELD_VAL(REG, FIELD, VAL)      \
  R_REGS[REG] = R_REGS[REG] & (~( (reg_field_masks[REG##FIELD]) << (reg_field_shifts[REG##FIELD])) ); \
  R_REGS[REG] = R_REGS[REG] | (VAL << (reg_field_shifts[REG##FIELD]))

#define RX_GET_BIT(BYTE, BIT)                    \
  BIT = ( (BYTE & 0x80) >>7);                    \
  BYTE = (BYTE << 1)

void poll_callback(void *handle);
HEXAPI_TransactionStatus access_callback(void *handle, HEX_PA_t address, HEX_4u_t numBytes, HEX_1u_t* dataptr, HEX_4u_t reqID, HEXAPI_BusAccessType bat, HEX_4u_t tnum, HEXAPI_BusBurstType bt);

#define state_character 0
#define state_space 1
#define state_end 2
  int str2argvargc(char my_string[], int *nsoas_int, char ***soas_int);

#endif 
