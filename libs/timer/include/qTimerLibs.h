/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QTIMER_LIBS_H
#define _QTIMER_LIBS_H

#include "qTimerDefines.h"
#include "qTimerHw.h"
#include <qube.h>
#include "qtimer.h"

/*---------------------------------------------------------------------------*/
/*  Timer internal errors                                                    */
/*---------------------------------------------------------------------------*/
#define ETIMER_TOOCLOSE                 0xffffffff

/*---------------------------------------------------------------------------*/
/*  Timer callback data type                                                 */
/*---------------------------------------------------------------------------*/
//typedef unsigned int                    qtimer_cb_data_type;

/*---------------------------------------------------------------------------*/
/*  Timer callback data type                                                 */
/*---------------------------------------------------------------------------*/
typedef struct qtimer_client_struct    *qtimer_client_ptr;

/*---------------------------------------------------------------------------*/
/*  Timer callback function type                                             */
/*---------------------------------------------------------------------------*/
typedef void (*qtimer_cb_type) (qtimer_client_ptr timer);

/*---------------------------------------------------------------------------*/
/*  Timer list structure type                                                */
/*  Values in this structure are for private use by "timer.c" only.          */
/*---------------------------------------------------------------------------*/
typedef struct qtimers_type_struct 
{

  /* A sorted linked list of all the active timer nodes */
  qtimer_client_ptr                active_list;

  /* match value set to the HW */
  qtimetick_word_t                 match_value;

  /* count value of HW when match value was set */
  qtimetick_word_t                 count_value;

  /* uptime ticks (wall clk) when match value was set */
  qtimetick_type                   set_expiry;

  /* rollover ticks */
  qtimetick_type                   zero;
}
qtimers_type;

typedef struct qtimers_group_type_struct 
{
  /* A sorted linked list of all the active timer nodes */
  qtimer_client_ptr                list;
}
qtimers_group_type;

/*-----------------------------------------------------------------------------
  Timer structure type
    Values in this structure are for private use by "timer.c" only.
-----------------------------------------------------------------------------*/

typedef struct qtimer_client_struct
{
  /* Magic number to validate client pointer */
  unsigned int                   magic;

  /* Timer type */
  unsigned int                   type;

  /* APC (Asynchronous Procedure Call) function to call when timer expires */
  qtimer_cb_type                 func;

  /* Data for APC function, when timer expires */
  qtimer_callback_t              cb_data;

  /* Reload offset when timer expires (0 = no reload at expiry) */
  /* SLEEP_TIMER */
  qtimer_duration_t               reload;

  /* TCX0 clock tick count when timer was started */
  qtimetick_type                 start;

  /* TCX0 clock tick count timer expires at or remaining ticks until expiry */
  qtimetick_type                 expiry;

  /* Pointer to the next timer in the list (list != NULL) */
  qtimer_client_ptr              next;

  /* Group no. of the timer. The criterion used to disable or enable the set
   * of timers. */
  unsigned int                   group;
}
qtimer_client;

/**
 * Initializes the Timer Subsytem
 * Note: This function is executed from server side.
 *
 * SIDE EFFECTS: May install an ISR for the 32-bit TCXO clock match register
 *
 * @param     None
 * @return    None
 */
void qtimer_lib_init ( void );

/**
 * Return absolute current time since the hardware timer was enabled
 * Note: This function is executed from server side.
 *
 * @param     None
 * @return    Absolute current time
 */
qtimetick_type qtimer_lib_current_time (qtimetick_word_t *hw_count);

/**
 * Defines and initializes a timer.
 *
 * When the timer expires:
 * If sigs is non-null, those signals are set to the task given by task;
 * Timers may also be staticly declared via the TIMER_DEF() macro
 * Note: This function is executed from server side.
 *
 * @param timer   Timer structure to be initialized
 * @param func    Function to be executed on timer expiry
 * @param data    Data to be send on timer expiry
 * @return        None
 */
void qtimer_lib_def ( qtimer_client_ptr timer,  qtimer_cb_type func, qtimer_callback_t *cb);

/**
 * Sets an active timer to expire after a given period of time.
 * Optionally, specifies the timer to repeatly expire with a given period.
 *
 * @param timer    Timer to set
 * @param time     timer duration
 * @param expiry   expiry time in micro-seconds
 * @param reload   Period in micro-seconds between repeated expiries (0 = not periodic)
 * @return         EOK
 */
int qtimer_lib_new ( qtimer_client_ptr timer, qtimer_duration_t time,  qtimer_time_t expiry, qtimer_duration_t reload, unsigned int group);

/**
 * Restart an active timer. x
 * "Duration" value can be different from orignal value.
 *
 * @param timer    Timer 
 * @param duration duration or expiry (duration from startup)
 */
int qtimer_lib_restart_active_timer ( qtimer_client_ptr timer, qtimer_duration_t duration);

#if 0
/**
 * Get the payload pointer from the timer node
 *
 * @param timer      Timer to get
 * @param payload    Pointer to the payload
 * @return           EOK or error code
 */
inline int qtimer_lib_get_payload ( qtimer_client_ptr timer, qtimer_cb_data_type* data );
#endif /* 0 */

/**
 * Stops an active timer
 *
 * @param timer    Timer to stop
 * @return         EOK or error code
 */
int qtimer_lib_cancel ( qtimer_client_ptr timer );

/**
 * This function Processes the expired timer
 * Note: May insert messages in message queue, which can cause task switches.
 * 
 * @param timer   Expiring timer to be processed
 * @return        None
 */
void qtimer_lib_expired ( qtimer_client_ptr timer );

/**
 * This function locks mutex and processes those timers which have expired.
 * Note: May insert messages in message queue, which can cause task switches.
 *
 * @param    None
 * @return   None
 */
void qtimer_lib_isr ( void );

/**
 * Send the signal the client registered to receive upon timer expiry.
 * Note: This function is executed from qtimer server.
 *
 * @param token     Sleeping client thread ID.
 * @return          EOK if send is successful, error code otherwise
 */
void qtimer_lib_process_signal_callback (qtimer_client_ptr timer);

/**
 * Send the signal the client registered to receive upon timer expiry. It does little more than qtimer_lib_process_signal_expiry () as the timer needs to be freed.
 * Note: This function is executed from qtimer server.
 *
 * @param token     Sleeping client thread ID.
 * @return          EOK if send is successful, error code otherwise
 */
void qtimer_lib_process_sleep_callback (qtimer_client_ptr timer);

/**
 * Send the message the client registered to receive upon expiry.
 * Note: This function is executed from qtimer server pd.
 *
 * @param token     Sleeping client thread ID.
 * @return          EOK if send is successful, error code otherwise
 */
void qtimer_lib_process_qmsgq_callback (qtimer_client_ptr timer);

/**
 * Enable group timers, make them active
 *
 * @param group    group ID
 * @return         EOK, EINVAL_ARG if group ID is invalid
 */
int qtimer_lib_group_enable (unsigned int group);

/**
 * Disables group timers. Pull them from active timers
 *
 * @param group    group ID
 * @return         EOK, EINVAL_ARG if group ID is invalid
 */
int qtimer_lib_group_disable (unsigned int group);

/**
 * Process the group timers if the are alreday expired.
 *
 * @param group    group ID
 * @return         none
 */
void qtimer_lib_process_group_expiry (unsigned int group);

void blast_timer_IST_init (void);
int qtimer_lib_stop_active_timer ( qtimer_client_ptr timer );

#endif /* _QTIMER_LIBS_H */
