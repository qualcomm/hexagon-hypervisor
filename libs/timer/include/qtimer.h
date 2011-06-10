/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************************/
/* FILE: qtimer.h                                                            */
/*                                                                           */
/* SERVICES: qtimer API                                                      */
/*                                                                           */
/* DESCRIPTION: Prototypes of qtimer API                                     */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*               Q D S P 6   T I M E R  S U B S Y S T E M                    */
/*                                                                           */
/* GENERAL DESCRIPTION                                                       */
/* Implements timer type functionality. This header files will be part of    */
/* the qTimerClient.c code.                                                  */
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

#ifndef _QTIMER_H
#define _QTIMER_H
#include "blast_anysignal.h"
#include "qTimerDefines.h"
#include "qmsgq.h"
#ifdef __cplusplus
extern "C" {
#endif
#define QDSP6_QTIMER_BASE 0x28880000

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

/**
 * qtimer_t types                                                           
 */
typedef unsigned int                        qtimer_t;

/**
 * qtimer_duration_t types                                                 
 */
typedef unsigned long long                  qtimer_duration_t;

/**
 * qtimer_time_t types                                                 
 */
typedef unsigned long long                  qtimer_time_t;

/**
 * qtimer_type types                                                       
 */
typedef enum
{
  QTIMER_ONESHOT = 0,
  QTIMER_PERIODIC
} qtimer_type_t;

/**
 * qtimer_attr_t types                                                      
 */
typedef struct 
{
    /**
     * Magic number to verify qmsgq_attr_t pointer 
     */
    unsigned int        magic;
    
    /**
     * This attribute specifies the duration of new timer. 
     */ 
    qtimer_duration_t   duration; 

    /**
     * This attribute specifies the absolute expiry of new timer. 
     */ 
    qtimer_time_t   expiry; 

    /**
     * This attribute specifies the remaining time of an active timer. 
     */ 
    qtimer_duration_t   remaining; 

    /**
     * This attribute specifies the timer type, only QTIMER_ONESHOT and
     * QTIMER_PERIODIC are currently supported 
     */
    qtimer_type_t       type;

    /**
     * Group no of the timer, the criterion used to disable or enable the set
     * of timers.
     */
    unsigned int        group;
}
qtimer_attr_t;

/**
 * qtimer_callback_t types                                                      
 */
typedef struct qtimer_callback {
    int type;
    union {
        struct {
            /**
             * The signal pointer the client is waiting on.
             */
            blast_anysignal_t *signal;
            /**
             * The signal mask the client is waiting on.
             */
            uint32_t mask;
        };
        /**
         * The MSGQ the client is waiting on
         */
        qmsgq_callback_t *qmsgq;
    };
}
qtimer_callback_t;

/**
 * qtimer_cmd_create_t types                                                      
 */
typedef struct qtimer_cmd_create {
    /**
     * Timer ID
     */
    qtimer_t          timer;
    /**
     * Timer type
     */
    qtimer_type_t     type;
    /**
     * Duration of the timer
     */
    qtimer_duration_t  duration;
    /**
     * Expiry of the timer
     */
    qtimer_time_t  expiry;
    /**
     * Callback type and the relevent information needed for the callback
     * from Server to Client to indicate the timer expirty
     */
    qtimer_callback_t cb;
    /**
     * Group ID of the timer
     */
    unsigned int      group;
    /**
     * Signal the client is waiting on for acknowledgement
     */
    blast_anysignal_t *ack;
    /**
     * Location to indicate SUCCESS or FAILURE by the timer server
     */
    int               result;
}
qtimer_cmd_create_t;

/**
 * qtimer_cmd_free_t types                                                      
 */
typedef struct qtimer_cmd_free {
    /**
     * Timer ID
     */
    qtimer_t          timer;
    /**
     * Location to indicate SUCCESS or FAILURE by the timer server
     */
    int               result;
    /**
     * Signal the client is waiting on for acknowledgement
     */
    blast_anysignal_t *ack;
}
qtimer_cmd_free_t;

/**
 * qtimer_cmd_getattr_t types                                                      
 */
typedef struct qtimer_cmd_getattr {
    /**
     * Timer ID
     */
    qtimer_t          timer;
    /**
     * Pointer to the attribute structure timer server has to update
     */
    qtimer_attr_t     *attr;
    /**
     * Signal the client is waiting on for acknowledgement
     */
    blast_anysignal_t *ack;
    /**
     * Location to indicate SUCCESS or FAILURE by the timer server
     */
    int               result;
}
qtimer_cmd_getattr_t;

/**
 * qtimer_cmd_getticks_t types                                                      
 */
typedef struct qtimer_cmd_getticks {
    /**
     * Location timer server has to update current ticks 
     */
    qtimetick_type     ticks;
    /**
     * Signal the client is waiting on for acknowledgement
     */
    blast_anysignal_t  *ack;
    /**
     * Location to indicate SUCCESS or FAILURE by the timer server
     */
    int                result;
}
qtimer_cmd_getticks_t;

/**
 * qtimer_cmd_group_t types                                                      
 */
typedef struct qtimer_cmd_group {
    /**
     * group ID to process the group enable / disable command
     */
    unsigned int       group;
    /**
     * Location to indicate SUCCESS or FAILURE by the timer server
     */
    int                result;
    /**
     * Signal the client is waiting on for acknowledgement
     */
    blast_anysignal_t  *ack;
}
qtimer_cmd_group_t;

/**
 * qtimer_cmd_t types                                                      
 */
typedef union {
    unsigned long long raw;
    struct {
        /**
         * Command type to indicate the type of command that follows
         */
        int cmd_type;
        union {
            /**
             * Create timer command
             */
            qtimer_cmd_create_t   *create;
            /**
             * Delete timer command
             */
            qtimer_cmd_free_t   *free;
            /**
             * Get attribute command
             */
            qtimer_cmd_getattr_t  *attr;
            /**
             * Get current ticks command
             */
            qtimer_cmd_getticks_t *ticks;
            /**
             * Group enable / disable command
             */
            qtimer_cmd_group_t    *group;
            /**
             * Communicate we are recovering from Power Collapse
             */
            qtimer_cmd_group_t    *pc;
        };
    };
}
qtimer_cmd_t;

/*****************************************************************************/
/*                                                                           */
/*                      FUNCTION DECLARATIONS                                */
/*                                                                           */
/*****************************************************************************/
/**
 * Creates a timer with specific attributes
 *
 * @param timer  [OUT] Timer object
 * @param attr         Specifies timer duration and timer type (
 *                     one-shot or periodic)
 * @param cb           Message queue and message sent when timer expires
 * @return             EOK:       Successful create,
 * @return             EMEM:      Out of memory error,
 */
int qtimer_create (qtimer_t *timer, const qtimer_attr_t *attr,
                  const qmsgq_callback_t *cb);

/**
 * Stop a one-shot timer  
 *
 * @param timer        Timer ID. 
 * @return             EOK:        Successful stop,
 * @return             EINVALID:   Invalid timer ID or duration value
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */
int qtimer_stop (qtimer_t timer);

/**
 * Restart a one-shot timer with a duration. 
 *
 * @param timer        Timer ID. 
 * @param duration     Timer duration.
 * @return             EOK:        Successful restart,
 * @return             EINVALID:   Invalid timer ID or duration value
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */
int qtimer_restart (qtimer_t timer, qtimer_duration_t duration);

/**
 * Creates a timer with specific attributes
 *
 * @param timer  [OUT] Timer object
 * @param attr         Specifies timer duration and timer type (
 *                     one-shot or periodic)
 * @param signal       Signal sent when timer expires
 * @param mask         Signal mask to be sent when timer expires
 * @return             EOK:       Successful create,
 * @return             EMEM:      Out of memory error,
 */
int sclk_timer_create (qtimer_t *timer, const qtimer_attr_t *attr,
                  const blast_anysignal_t *signal, unsigned int mask);

/**
 * Stop a one-shot timer  
 *
 * @param timer        Timer ID. 
 * @return             EOK:        Successful stop,
 * @return             EINVALID:   Invalid timer ID or duration value
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */
int sclk_timer_stop (qtimer_t timer );

/**
 * Restart a one-shot timer with a duration. 
 *
 * @param timer        Timer ID. 
 * @param duration     Timer duration.
 * @return             EOK:        Successful restart,
 * @return             EINVALID:   Invalid timer ID or duration value
 * @return             ENOTALLOWED: Timer is not a oneshot timer.
 * @return             EMEM:        Out of memory error,
 */
int sclk_timer_restart (qtimer_t timer, qtimer_duration_t duration );

/**
 * Initialize timer attributes
 *
 * @param attr  [OUT] Timer attributes object
 */
void qtimer_attr_init(qtimer_attr_t *attr);

/**
 * Set duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qtimer_attr_setduration(qtimer_attr_t *attr, qtimer_duration_t duration);

/**
 * Set expiry
 *
 * @param attr      Timer attributes object
 * @param time      Absolute expiry time for the timer in micro-seconds
 */
void qtimer_attr_setexpiry(qtimer_attr_t *attr, qtimer_time_t time);

/**
 * Get duration
 *
 * @param attr      Timer attributes object
 * @param duration  Duration for the timer
 */
void qtimer_attr_getduration(qtimer_attr_t *attr, qtimer_duration_t *duration);

/**
 * Get remaining time
 *
 * @param attr      Timer attributes object
 * @param remain    Remaining time for the timer
 */
void qtimer_attr_getremaining(qtimer_attr_t *attr, qtimer_duration_t *remaining);

/**
 * Set timer type - one shot or periodic
 *
 * @param attr   Timer attributes object
 * @param type   Timer type
 */
void qtimer_attr_settype(qtimer_attr_t *attr, qtimer_type_t type);

/**
 * Set duration
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qtimer_attr_gettype(qtimer_attr_t *attr, qtimer_type_t *type);

/**
 * Set timer group, ranging 0 to QTIMER_MAX_GROUPS - 1
 *
 * @param attr   Timer attributes object
 * @param group  Group ID
 */
void qtimer_attr_setgroup(qtimer_attr_t *attr, unsigned int group);

/**
 * Get the group ID from the attribute
 *
 * @param attr  Timer attributes object
 * @param type  Timer type
 */
void qtimer_attr_getgroup(qtimer_attr_t *attr, unsigned int *group);

/**
 * Get attributes of a message queue
 *
 * @param msgq         Message queue object
 * @param attr  [OUT]  Message queue attributes
 * @return             EOK:       get_attr successful,
 * @return             EVAL:  Wrong parameters,
 */
int qtimer_get_attr(qtimer_t timer, qtimer_attr_t *attr);

/**
 * Deletes timer
 *
 * @param timer  Pointer to timer object
 * @return       EOK:       Successful create,
 * @return       EVAL:  Wrong timer 
 */
int qtimer_delete(qtimer_t timer);

/**
 * Sleep for specified duration.
 *  
 * @param duration  number of us to sleep for 
 * @return             EOK:       Successful create,
 * @return             EMEM:      Out of memory error,
 */

int qtimer_sleep(qtimer_duration_t duration);

/**
 * Disable the timers that belong to the specified group. Timers
 * don't get processed with a timer interrupt, once they are disabled.
 * 
 * NOTE:
 * ----
 * This API is expected to be called by Power Management SW under
 * Single Threaded Mode, wheren only one HW thread runs. Otherwise,
 * it can be delays in response from Timer Server.
 *
 * @param group        The group of timers need to be disabled
 * @return             EOK:       successful
 */
int qtimer_group_disable (unsigned int group);

/**
 * Enable the timers that belong to the specified group, which are disabled. If
 * no timers were already disabled, do nothing. Process the timers which are
 * already expired.
 *
 * @param group        The group of timers need to be enabled
 * @return             EOK:       successful
 */
int qtimer_group_enable (unsigned int group);

/**
 * Communicate the timer server that we are recovering from Power Collapse. The server
 * has to account for any missed interrupts durint power collapse etc.
 *
 */
void qtimer_recover_pc (void);

/**
 * To find out if Qtimer is initialized.
 * 0        - not initialized.
 * non-zero - initialized.
 */
int qtimer_is_init (void);

/**
 * Get current ticks. The ticks are accumulated since the RTOS is started. Each tick
 * is equal to a single sleep clock cycle, where sleep clock runs at 32 KHz.
 *
 * @return             ticks since system started
 */
unsigned long long qsystem_sclk_attr_getticks (void);

/**
 * Get tick count on HW. The HW counter is 32-bit counter and it rolls over.
 * This API is a faster way to obtian ticks, if the roll over is taken care
 * externally. For complete uptime ticks, use "qsystem_sclk_attr_getticks".
 *
 * @return             ticks since system started
 */
unsigned int blast_system_sclk_attr_gethwticks (void);

/**
 * Determines how long it takes for the next timer interrupt. Gets the number of ticks
 * that needs to be elapsed for the next timer interrupt.
 *
 * @return             no. of ticks to take for next timer interrupt
 */
unsigned int qsystem_sclk_attr_getexpiry (void);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _QTIMER_H */
