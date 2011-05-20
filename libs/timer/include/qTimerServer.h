/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QTIMER_SERVER_H 
#define _QTIMER_SERVER_H 

/*****************************************************************************/
/*                                                                           */
/*               Q D S P 6   T I M E R   S U B S Y S T E M                   */
/*                                                                           */
/* GENERAL DESCRIPTION                                                       */
/* Implements timer server functionality. This header files will be part of  */
/* the qTimerServer code.                                                    */
/*                                                                           */
/* FUNCTIONS                                                                 */
/*                                                                           */
/* INITIALIZATION AND SEQUENCING REQUIREMENTS                                */
/*                                                                           */

/*                                                                           */
/*****************************************************************************/

/************************ COPYRIGHT NOTICE ***********************************/

/* All data and information contained in or disclosed by this                */
/* document is confidential and proprietary information of                   */
/* QUALCOMM, Inc and all rights therein are expressly reserved.              */
/* By accepting this material the recipient agrees that this                 */
/* material and the information contained therein is held in                 */
/* confidence and in trust and will not be used, copied,                     */
/* reproduced in whole or in part, nor its contents revealed in              */
/* any manner to others without the express written permission               */
/* of QUALCOMM, Inc.                                                         */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                      EDIT HISTORY FOR MODULE                              */
/*                                                                           */
/*  This section contains comments describing changes made to the module.    */
/*  Notice that changes are listed in reverse chronological order.           */
/*                                                                           */
/*  when       who     what, where, why                                      */
/* --------    ---     ------------------------------------------------------*/
/* 07/10/06    saa     File created.                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                           INCLUDE FILES                                   */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                          DATA DECLARATIONS                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                      FUNCTION DECLARATIONS                                */
/*                                                                           */
/*****************************************************************************/
void qTimerServer ( void *arg);

/**
 * This is a timer isr thread. It first registers itself as timer interrupt 
 * handler and waits for ipc. Upon receiving ipc, it processes the expired
 * timers by calling timer_isr function and finally sends a reply back to the
 * sender before going in wait again.
 *
 * @param    None
 * @return   None
 */
void qtimer_task ( void );

#endif /* _QTIMER_SERVER_H */

