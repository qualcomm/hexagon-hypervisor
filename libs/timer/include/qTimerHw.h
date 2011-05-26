/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _TIMER_HW_H
#define _TIMER_HW_H

#include "qTimerPorts.h"
#include "qTimerDefines.h"
#include "stdbool.h"

/*-----------------------------------------------------------------------------
  Timer ISR function
-----------------------------------------------------------------------------*/
typedef void (*hw_th_isr_type) ( void );

/*-----------------------------------------------------------------------------
  Hardware register types
-----------------------------------------------------------------------------*/
typedef unsigned int                    hw_reg_type;

/*-----------------------------------------------------------------------------
  Hardware register value types
-----------------------------------------------------------------------------*/
typedef unsigned int                    hw_value_type;

void hw_timer_init (qtimetick_word_t match);

int hw_timer_isAlive ( void );

qtimetick_word_t hw_timer_curr_timetick ( void );

qtimetick_word_t hw_timer_prg_next_interrupt
(
  /* Smallest Timer Expiry */
  qtimetick_word_t match,

  /* Current Time */
  qtimetick_word_t count 
);
  
#endif /* _TIMER_HW_H */
