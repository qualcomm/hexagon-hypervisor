/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QTIMER_PORTS_H_
#define _QTIMER_PORTS_H_

//#define QDSP6_RGPT_BASE	            0xAB000000ul
extern unsigned int QDSP6_QTIMER_BASE;
//#define QDSP6_UGPT_BASE	            0xAB004000ul
//extern unsigned int QDSP6_UGPT_BASE;
#define ADSP_RGPT_MATCH_VAL         ((QDSP6_QTIMER_BASE) + 0x0)
#define ADSP_RGPT_COUNT_VAL         ((QDSP6_QTIMER_BASE) + 0x4)
#define ADSP_RGPT_ENABLE            ((QDSP6_QTIMER_BASE) + 0x8)
#define ADSP_RGPT_CLEAR             ((QDSP6_QTIMER_BASE) + 0xC)
#define ADSP_UGPT_MATCH_VAL         ((QDSP6_QTIMER_BASE) + 0x0)
#define ADSP_UGPT_COUNT_VAL         ((QDSP6_QTIMER_BASE) + 0x4)
#define ADSP_UGPT_ENABLE            ((QDSP6_QTIMER_BASE) + 0x8)
#define ADSP_UGPT_CLEAR             ((QDSP6_QTIMER_BASE) + 0xC)

//#ifdef MACHINE_Q6ZEBU
//#define USER_GENERAL_PURPOSE_TIMER 4 
//#endif
//
//#ifdef MACHINE_Q6SIM
//#define USER_GENERAL_PURPOSE_TIMER 3 
//#endif

//extern int USER_GENERAL_PURPOSE_TIMER;
extern int QUBE_GENERAL_PURPOSE_TIMER;

#endif /* _QTIMER_PORTS_H_ */
