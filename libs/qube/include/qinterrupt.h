/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_QINTERRUPT_H
#define H2_QINTERRUPT_H

/*****************************************************************/
/* FILE: qinterrupt.h                                            */
/*                                                               */
/* SERVICES: qinterrupt API                                      */
/*                                                               */
/* DESCRIPTION: Prototypes of qinterrupt API                     */
/*****************************************************************/

/************************ COPYRIGHT NOTICE ***********************/

/* All data and information contained in or disclosed by this    */
/* document is confidential and proprietary information of       */
/* QUALCOMM, Inc and all rights therein are expressly reserved.  */
/* By accepting this material the recipient agrees that this     */
/* material and the information contained therein is held in     */
/* confidence and in trust and will not be used, copied,         */
/* reproduced in whole or in part, nor its contents revealed in  */
/* any manner to others without the express written permission   */
/* of QUALCOMM, Inc.                                             */
/*****************************************************************/

/* l1_int_num shall NOT be zero: NO L2 interrupt controller connects to L1 interrupt 0 */
#define L2_INTERRUPT_NUM(l1_int_num, l2_int_num) (l1_int_num*32+l2_int_num)

/**
 * Register for an interrupt.  Register number 2-9(inclusive) are 
 * reserved for rtos usage. 
 *
 * @param int_num  Interrupt number
 * @param cb       Callback to be used when interrupt occurs
 * @return         EOK if successful
 * @return         EVAL if int_num is not valid interrupt or
 *                  cb is NULL or callback function is NULL
 * @return         EMEM if out of memory allocating internal data
 *                  structure 
 */
//int qinterrupt_register(int int_num, qinterrupt_callback_t *cb);
int qinterrupt_register(int int_num);

/**
 * Deregister for a interrupt.
 *
 * @param int_num  Interrupt number
 * @return         EOK if successful
 * @return         EVAL if int_num is not valid interrupt number less
 *                  than 31 or reserved by rtos 
 * @return         EINVALID if not a registered interrupt
 */
int qinterrupt_deregister(int int_num);

/**
 * Receive an interrupt.
 *
 * @param int_num  Interrupt number
 * @return         EOK if successful
 * @return         EVAL if int_num is not valid interrupt number less
 *                  than 31 or reserved by rtos 
 * @return         ENO_INTERRUPTS if the calling thread is not registering
 *                  for any interrupts 
 */

int qinterrupt_receive (unsigned int *int_num);

#endif
