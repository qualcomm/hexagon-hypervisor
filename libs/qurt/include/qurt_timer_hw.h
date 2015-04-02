/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_TIMER_HW_H
#define QURT_TIMER_HW_H

#include "qurt_timer_defines.h"
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

void hw_timer_init (qurt_timetick_word_t match);

int hw_timer_isAlive ( void );

qurt_timetick_word_t hw_timer_curr_timetick ( void );

qurt_timetick_word_t hw_timer_prg_next_interrupt
(
  /* Smallest Timer Expiry */
  qurt_timetick_word_t match,

  /* Current Time */
  qurt_timetick_word_t count, 

  /* order of values */
  unsigned int order
);
  
#endif /* QTIMER_HW_H */
