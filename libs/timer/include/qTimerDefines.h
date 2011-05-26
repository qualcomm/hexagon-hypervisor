/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _TIMER_DEFINES_H
#define _TIMER_DEFINES_H

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
/*---------------------------------------------------------------------------*/
/* Define null                                                               */
/*---------------------------------------------------------------------------*/
#ifndef NULL      
#define NULL                           0
#endif

//#define QTIMER_TXCO 1

/*---------------------------------------------------------------------------*/
/* Define magic                                                              */
/*---------------------------------------------------------------------------*/
#define QTIMER_MAGIC                   0xadadadad
#define QTIMER_EXPIRED                 0xedededed
#define QTIMER_INVALID                 0xdfdfdfdf
#define QTIMER_ATTR_MAGIC              0xaeaeaeae
#define QTIMER_ATTR_INVALID            QTIMER_INVALID

/*---------------------------------------------------------------------------*/
/* Default Values                                                            */
/*---------------------------------------------------------------------------*/
#define QTIMER_DEFAULT_TYPE            QTIMER_ONESHOT
#define QTIMER_DEFAULT_DURATION        1000uL
#define QTIMER_DEFAULT_EXPIRY          0uL

/*---------------------------------------------------------------------------*/
/* Supported Callback types                                                  */
/*---------------------------------------------------------------------------*/
#define QTIMER_CALLBACK_SIG_TYPE        1
#define QTIMER_CALLBACK_QMSGQ_TYPE      2

/*---------------------------------------------------------------------------*/
/* Supported timer types                                            */
/*---------------------------------------------------------------------------*/
#define QTIMER_ABSOLUTE_EXPIRY          1
#define QTIMER_RELATIVE_DURATION        2

/*****************************************************************************/
/*                                                                           */
/*                          MACRO DEFINITIONS                                */
/*                                                                           */
/*****************************************************************************/

#ifdef QTIMER_TXCO
/*---------------------------------------------------------------------------*/
/*  Nominial TCXO clock freq in hertz.                                       */
/*---------------------------------------------------------------------------*/
#define TIMETICK_NOMINAL_FREQ_HZ        19354838uL

/*---------------------------------------------------------------------------*/
/*  Nominial TCXO clock freq in hertz.                                       */
/*---------------------------------------------------------------------------*/
#define TIMETICK_NOMINAL_FREQ_KHZ       19354uL

/*---------------------------------------------------------------------------*/
/*  Nominial TCXO clock freq in hertz.                                       */
/*---------------------------------------------------------------------------*/
#define TIMETICK_NOMINAL_FREQ_MHZ       19uL

#define QTIMER_TIMETICK_FROM_MS(ms) (((unsigned long long)(ms)*TIMETICK_NOMINAL_FREQ_HZ)/1000)
  /* Conversion from milliseconds to TCXO clock ticks at the nominal frequency */

  /* Conversion from microseconds to TCXO clock ticks at the nominal frequency */

static inline unsigned int QTIMER_TIMETICK_FROM_US(unsigned long long us)
{
    unsigned int result;
    __asm__ __volatile__ (
            "{r1:0 = %1\n"
            "r4.h = #0x5AD6}\n" /* r4 = fixed point: 0.354838 * 0xffffffff */
            "{r4.l = #0xA9C5\n"
            "r1.l = #19}\n"
            "r1 = mpyui(r0, r1)\n"
            "r3:2 = mpyu(r0, r4)\n"
            "r3:2 = lsr(r3:2, #32)\n"
            "%0 = add(r1, r2)\n"
            : "=r" (result)
            : "r" (us)
            : "r0", "r1", "r2", "r3", "r4"
            );
    return result;
}

#define QTIMER_TIMETICK_TO_MS(ticks) (((unsigned long long)(ticks)*(1000uL))/TIMETICK_NOMINAL_FREQ_HZ )
  /* Conversion from TCXO clock ticks to milliseconds at the nominal frequency */

#define QTIMER_TIMETICK_TO_US(ticks) (((unsigned long long)(ticks)*(1000000))/TIMETICK_NOMINAL_FREQ_HZ)
  /* Conversion from TCXO clock ticks to microseconds at the nominal frequency */

/*---------------------------------------------------------------------------*/
/* MINIMUM Microseconds Value = 125 Microseconds                              */
/*---------------------------------------------------------------------------*/
#define QTIMER_MIN_DURATION             125

/*---------------------------------------------------------------------------*/
/* MAXIMUM Microseconds Value = 3 Minutes                                    */
/*---------------------------------------------------------------------------*/
#define QTIMER_MAX_DURATION             (3 * 60 * 1000 * 1000)

/*---------------------------------------------------------------------------*/
/* Timer Error Margin 25us                                                   */
/*---------------------------------------------------------------------------*/
#define QTIMETICK_ERROR_MARGIN          0x1f7

/*---------------------------------------------------------------------------*/
/* Timer Interrupt Margin 3 Microseconds                                     */
/*---------------------------------------------------------------------------*/
#define QTIMETICK_INT_MARGIN            0x60

#else
#if 0
  /* Conversion from microseconds to sleep clock ticks at the nominal frequency */
static inline unsigned int QTIMER_TIMETICK_FROM_US(unsigned long long us)
  {
    unsigned int result;
    __asm__ __volatile__ (
        "{r1:0 = %1\n"
        "r4.h = #0x863}\n" /* r4 = fixed point: 0.032768 * 0xffffffff */
        "r4.l = #0x7BD0\n"
        "r3:2 = mpyu(r0, r4)\n"
        "r1:0 = mpyu(r1, r4)\n" 
        "{r0 = add(r0 ,#1)\n"      /* compensation for fractional number lost after decimal point */
        "r3:2 = lsr(r3:2, #32)}\n" /* valid result is in r2 */
        "%0 = add(r0, r2)\n"
        : "=r" (result)
        : "r" (us)
        : "r0", "r1", "r2", "r3", "r4"
    );
    return result;
  }
#endif /* 0 */
/*---------------------------------------------------------------------------*/
/*  Nominial TCXO clock freq in hertz.                                       */
/*---------------------------------------------------------------------------*/
#define TIMETICK_NOMINAL_FREQ_HZ        32768uL

/*---------------------------------------------------------------------------*/
/*  Nominial TCXO clock freq in hertz.                                       */
/*---------------------------------------------------------------------------*/
#define TIMETICK_NOMINAL_FREQ_KHZ       32uL

#define QTIMER_TIMETICK_FROM_US(us) ((us * 32768ull) / (1000ul * 1000ul))

// 1000*1000/32768 = 30.517578125
#define QTIMER_TIMETICK_TO_US(ticks) (((unsigned long long)(ticks)*30ul)+((unsigned long long)(ticks)>>1)+((unsigned long long)(ticks)>>6)+((unsigned long long)(ticks)>>9))
  /* Conversion from sleep clock ticks to microseconds at the nominal frequency */

/*---------------------------------------------------------------------------*/
/* MINIMUM Microseconds Value = 100 Microseconds (Sleep timer)               */
/*---------------------------------------------------------------------------*/
#define QTIMER_MIN_DURATION             100uL

/*---------------------------------------------------------------------------*/
/* MAXIMUM Microseconds Value = 36 hours                                     */
/*---------------------------------------------------------------------------*/
#define QTIMER_MAX_DURATION             ((unsigned long long)(36ul * 3600ul * 1000ul * 1000uLL))
//#define QTIMER_MAX_DURATION             (5 * 60 * 1000 * 1000)

// Sleep clock is 32768 Hz
#define QTIMER_MAX_DURATION_TICKS       (36ul * 3600ul * 32768ul)

#define QTIMER_TEST_COUNT               0xFFFFFFF0
/*---------------------------------------------------------------------------*/
/* Set up timer for match value 1 tick ~ 30 us                               */
/*---------------------------------------------------------------------------*/
#define QTIMER_MATCH_SETUP_TIME         1

/*---------------------------------------------------------------------------*/
/* Sleep Timer Error Margin, setup time (2 ticks) + resolution (1 tick) ~90us*/
/*---------------------------------------------------------------------------*/
#define QTIMETICK_ERROR_MARGIN          3

/*---------------------------------------------------------------------------*/
/* Sleep Timer Calculation Margin for round/chunc difference                 */
/*---------------------------------------------------------------------------*/
#define QTIMETICK_CALCULATION_MARGIN    1

#endif // end of QTIMER_TXCO

/*---------------------------------------------------------------------------*/
/* MAXIMUM Timer Value                                                        */
/*---------------------------------------------------------------------------*/
#define QTIMETICK_MAX                   0xFFFFFFFF

/*---------------------------------------------------------------------------*/
/* MINIMUM Timer Value                                                       */
/*---------------------------------------------------------------------------*/
#define QTIMETICK_MIN                   0x0

#define QTIMER_CLIENT_ACK_SIGMASK       0x1

/*---------------------------------------------------------------------------*/
/*  qtimer ipc related defines                                               */
/*---------------------------------------------------------------------------*/
#define QTIMER_IPC_CREATE              1
#define QTIMER_IPC_DELETE              2
#define QTIMER_IPC_GET_ATTR            3
/* SLEEP_TIMER */
#define QTIMER_IPC_SLEEP               4
#define QTIMER_IPC_GET_TICKS           5
#define QTIMER_IPC_GROUP_EN            6
#define QTIMER_IPC_GROUP_DIS           7
#define QTIMER_IPC_INTERRUPT           8
#define QTIMER_IPC_PC                  9
#define QTIMER_IPC_RESTART            10
#define QTIMER_IPC_STOP               11

/*---------------------------------------------------------------------------*/ /*  qtimer group defines                                                     */
/*  qtimer group defines                                                     */
/*---------------------------------------------------------------------------*/
#define QTIMER_MAX_GROUPS              5
#define QTIMER_DEFAULT_GROUP           0

/*---------------------------------------------------------------------------*/
/* Time, in TCXO clock counts, from 0 to 0xFFFFFFFF (~3.555 minutes )        */
/*---------------------------------------------------------------------------*/
typedef unsigned long long              qtimetick_type;
typedef unsigned int                    qtimetick_word_t;

#endif /* _TIMER_DEFINES_H */
