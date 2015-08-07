/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>

#define ETM_CLK_EN          0      // Clock enable R/W
#define ETM_CTRL            1      // Overall ETM configuration R/W
#define ETM_RESET           2      // Reset control R/W
#define ETM_VERSION         3      // Version identifier R only
#define TRIG0_SAC0_ADDR     4      // SAC reference address R/W
#define TRIG0_SAC1_ADDR     5      // SAC reference address R/W
#define TRIG0_DC_DATA       6      // DC reference data R/W
#define TRIG0_DC_MASK       7      // DC mask R/W
#define TRIG0_CTRL0         8      // Trigger block control R/W
#define TRIG0_CTRL1         9      // Trigger block control R/W
#define TRIG0_COUNT        10      // Trigger counter initial value R/W
#define TRIG1_SAC0_ADDR    11      // SAC reference address R/W
#define TRIG1_SAC1_ADDR    12      // SAC reference address R/W
#define TRIG1_DC_DATA      13      // DC reference data R/W
#define TRIG1_DC_MASK      14      // DC mask R/W
#define TRIG1_CTRL0        15      // Trigger block control R/W
#define TRIG1_CTRL1        16      // Trigger block control R/W
#define TRIG1_COUNT        17      // Trigger counter initial value R/W
#define TRIG2_SAC0_ADDR    18      // SAC reference address R/W
#define TRIG2_SAC1_ADDR    19      // SAC reference address R/W
#define TRIG2_DC_DATA      20      // DC reference data R/W
#define TRIG2_DC_MASK      21      // DC mask R/W
#define TRIG2_CTRL0        22      // Trigger block control R/W
#define TRIG2_CTRL1        23      // Trigger block control R/W
#define TRIG2_COUNT        24      // Trigger counter initial value R/W
#define TRIG3_SAC0_ADDR    25      // SAC reference address R/W
#define TRIG3_SAC1_ADDR    26      // SAC reference address R/W
#define TRIG3_DC_DATA      27      // DC reference data R/W
#define TRIG3_DC_MASK      28      // DC mask R/W
#define TRIG3_CTRL0        29      // Trigger block control R/W
#define TRIG3_CTRL1        30      // Trigger block control R/W
#define TRIG3_COUNT        31      // Trigger counter initial value R/W
#define SEQ_TCTRL0         32      // Sequencer transition control R/W
#define SEQ_TCTRL1         33      // Sequencer transition control R/W
#define PROF_CTRL          34      // Profiling control R/W
#define PROF_REGION_CNT    35      // Profiling region counter R/W
#define PROF_SRC_CTRL0     36      // Profiling counter source control R/W
#define PROF_SRC_CTRL1     37      // Profiling counter source control R/W
#define ETB_RD_PTR         38      // R/W
#define ETB_WR_PTR         39      // R/W
#define ETB_TR_CNT         40      // R/W
#define ETB_DEPTH          41      // RO
#define ETB_CTRL           42      // R/W
#define ETB_STATUS         43      // R/W
#define ETB_RD_BUF         44      // RO, autoincrement
#define ETB_WR_RAM         45      // WO return 0x0 on R, autoincrement
#define ASYNC_PERIOD       46      // Generation period R/W
#define ISYNC_PERIOD       47      // Isync generation period R/W
#define GSYNC_PERIOD       48      // Global thread sync period R/W
#define GSYNC_COUNTER_LOW  49      // Lower gsync count reg R/W
#define GSYNC_COUNTER_HIGH 50      // Higher gsync count reg R/W
#define TEST_BUS_CTRL      51      // Testbus programming R/W
#define ETM_SEQ_STATE      52      // Current sequencer state
#define ETM_ATID           53      // Trace Source ID

static int qurt_etm_type = 0;
static int qurt_etm_route = 0;
static int qurt_etm_filter = 0;

extern unsigned int QURTK_etm_atid __attribute__((section(".data")));
extern unsigned int QURTK_etm_cfg_base __attribute__((section(".data")));

#define ETM_WRITE(IDX,VAL) do { ETM_BASE[IDX] = (VAL); } while (0)
#define ETM_READ(IDX) (ETM_BASE[IDX])

volatile unsigned int *ETM_BASE;

unsigned int qurt_etm_set_config(unsigned int type, unsigned int route, unsigned int filter)
{
	qurt_etm_type = type;
	qurt_etm_route = route;
	qurt_etm_filter = filter;
	return QURT_ETM_SETUP_OK;
}

static inline unsigned int qurt_get_filter()
{
	unsigned int ret = 0;
	/* EJP: insert hw tnum filtering */;
	return ret;
}

static inline unsigned int qurt_get_type()
{
	unsigned int ret = 0x003f0004;
	if (qurt_etm_type == 0) ret |= (1<<28);
	if (qurt_etm_type & QURT_ETM_TYPE_PC_ADDR) ret |= (1<<28);
	if (qurt_etm_type & QURT_ETM_TYPE_MEMORY_ADDR) ret |= (1<<29);
	return ret;
}

static void etm_init_and_start()
{
	unsigned int etm_ctrl = 0;
	ETM_WRITE(ETM_CLK_EN,1);
	ETM_WRITE(ETM_RESET,1);
	ETM_WRITE(ETM_RESET,0);
	(void)ETM_READ(ETM_VERSION);
	ETM_WRITE(ETM_ATID,QURTK_etm_atid);
	if (qurt_etm_type & QURT_ETM_TYPE_CYCLE_ACCURATE) etm_ctrl |= 0x20;
	if (qurt_etm_type & QURT_ETM_TYPE_CYCLE_COARSE) etm_ctrl |= 0x04;
	if (qurt_etm_route == QURT_ETM_ROUTE_TO_QDSS) {
		/* To the QDSS! */
		etm_ctrl |= 0x00A00410;
	} else {
		/* To the ETB! */
		ETM_WRITE(ETB_WR_PTR,0);
		ETM_WRITE(ETB_RD_PTR,0);
		ETM_WRITE(ETB_CTRL,1);
		etm_ctrl |= 0x0020001f;
	}
	ETM_WRITE(TRIG0_SAC0_ADDR,0);
	ETM_WRITE(TRIG0_SAC1_ADDR,0);
	ETM_WRITE(TRIG0_CTRL0,qurt_get_filter());
	ETM_WRITE(TRIG0_CTRL1,qurt_get_type());
	ETM_WRITE(TRIG0_COUNT,1);
	ETM_WRITE(ETM_CTRL,etm_ctrl);
}

static void etm_stop()
{
	ETM_WRITE(TRIG0_SAC0_ADDR,0);
	ETM_WRITE(TRIG0_SAC1_ADDR,0);
	ETM_WRITE(TRIG0_CTRL0,0);
	ETM_WRITE(TRIG0_CTRL1,0);
	ETM_WRITE(ETM_CTRL,0);
}

unsigned int qurt_etm_enable(unsigned int enable_flag)
{
	ETM_BASE = (volatile unsigned int *)QURTK_etm_cfg_base;
	if (enable_flag) etm_init_and_start();
	else etm_stop();
	return QURT_ETM_SETUP_OK;
}

unsigned int qurt_etm_set_breakpoint(unsigned int type, unsigned int address, unsigned int data, unsigned int mask)
{
	return QURT_ETM_SETUP_OK;
}

unsigned int qurt_etm_set_breakarea(unsigned int type, unsigned int start_address, unsigned int end_address, unsigned int count)
{
	return QURT_ETM_SETUP_OK;
}

