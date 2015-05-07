/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Development Kit,
 * 'ucos' (micro-C/OS) TASK/EXTension sample.
 *  Copyright Lauterbach GmbH
 * 
 *  indent -linux -i4 -ts8 -l132 ucos.c
 *  indent -ts8 -i4 -l132 -bli0 -bap -npsl -npcs -br -ce -cdw -cli0 -cbi0 -sc ucos.c
 */
#include "t32ext.h"

/* Prototypes */
uint32 MagicToId(uint32);
char *MagicToName(uint32);
T32MagicDef *GetMagic(void);
uint32 *GetTaskList(void);
T32StackDef *GetTaskStack(uint32);
T32CmdInitRtn CmdTlParser(void);
T32CmdDisplayDef *CmdTlInit(void);
void CmdTlDisplay(void);
T32CmdInitRtn CmdTsParser(void);
T32CmdDisplayDef *CmdTsInit(void);
void CmdTsDisplay(void);
uint32 FctTaskConfig(uint32, uint32);
T32ALDisplayDef *AnaListInit(void);
T32ALCheckResult *AnaListCheck(void);
void AnaListDisplay(T32ALCheckResult *);
T32ASCheckDef *AnaStatInit(void);
uint32 AnaStatCheck(void);
T32ATCheckRtn AnaTaskstInit(void);
T32ATCheckResult *AnaTaskstCheck(void);

/* Globals */
uint32 par_magic = 0;		/* magic address         */
uint32 par_tcblist = 0;		/* address of tcb list   */
uint32 par_tcbtab = 0;		/* address of tcb table  */
uint32 par_tnum = 0;		/* number of tasks       */

/* tcb structure offsets */
#define TCB_STKPTR	0
#define TCB_STKBASE	4
#define TCB_STKSIZE	8
#define TCB_STATE	10
#define TCB_PRIO	11
#define TCB_DELAY	12
#define TCB_EVENT	0x12
#define TCB_NEXT	0x16
#define TCB_NAME	0x1e
#define TCB_LEN		34

/* Register this TASK extension with TRACE32. */
void main(void)
{
    static T32CmdDef tasklist =
    {	/* Definition for new command "TASK.TaskList" */
	"TASKLIST",		/* long command form            */
	"TL",			/* short command form           */
	"TaskList",		/* softkey representation       */
	"__EMUCOS_TL",		/* help specification           */
	CmdTlParser		/* Parser routine               */
    };

    static T32CmdDef taskstate = {
	"TASKSTATE", "TASKS", "TASKState", "__EMGENERIC_TASKS",
	CmdTsParser
    };

    static T32FctDef taskconfig =
    {	/* Definition for a new function "TASK.CONFIG" */
	"TASK.CONFIG",		/* function name                        */
	PAR_HEX,		/* return parameter is hexadecimal      */
	PAR_STRING_DIRECT,	/* first parameter is string w/o ""     */
	PAR_NONE,		/* no second parameter                  */
	"__EMUCOS_FUNC",	/* help specification                   */
	FctTaskConfig		/* function calculation routine         */
    };

    static T32AnaListDef analist =
    {	/* Definition for new Analyzer.List Display */
	"__EMUCOS_AL",		/* help specification                   */
	AnaListInit		/* Initialization routine               */
    };

    static T32AnaStatDef anastat =
    {	/* Definition for Analyzer Statistics   */
	"__EMUCOS_AS",		/* help specification                   */
	AnaStatInit		/* Initialization routine               */
    };

    static T32AnaTaskstateDef anataskst =
    {	/* Definition for Task State Analysis   */
	"__EMUCOS_AT",		/* help specification                   */
	AnaTaskstInit		/* Initialization routine               */
    };

    /* Get Parameters and init Globals */
    par_magic = T32ArgumentGet(0);
    par_tcblist = T32ArgumentGet(1);
    par_tcbtab = T32ArgumentGet(2);
    par_tnum = T32ArgumentGet(3);

    T32DefineTaskId(MagicToId);			/* Define a Magic to Id translation */
    T32DefineTaskName(MagicToName);		/* Define Magic to Name translation */
    T32DefineGetMagic(GetMagic);		/* Define the "magic location" routine */
    T32DefineTaskList(GetTaskList);		/* Define a Task List calculation routine */
    T32DefineTaskStack(GetTaskStack);		/* Define a Task Stack calculation routine */
    T32DefineCommand(&tasklist);		/* Define a "TASK.TaskList" command" */
    T32DefineCommand(&taskstate);		/* Define a "TASK.TASKState" command" */
    T32DefineFunction(&taskconfig);		/* Define a "TASK.CONFIG" function */
    T32DefineAnalyzerList(&analist);		/* Define Analyzer List display routine */
    T32DefineAnalyzerStat(&anastat);		/* Define Analyzer statistic routine */
    T32DefineAnalyzerTaskstate(&anataskst);	/* Define Analyzer task state routine */
}


/**********************  Magic to Id Translation  **********************/
uint32 MagicToId(uint32 magic)
{
    if (!par_tcbtab)
	par_tcbtab = T32SymbolAddrGet("OSTCBTbl");

    return ((magic - par_tcbtab) / TCB_LEN);
}


/**********************  Magic to Name Translation  **********************/
char *MagicToName(uint32 magic)
{
    static char name[16];
    uint32 nameptr;

    nameptr = T32ReadLong(magic + TCB_NAME);
    T32ReadBuffer(name, nameptr, 16);
    name[15] = 0;

    return (name);
}


/**********************  Calculation of "magic" location  **********************/
T32MagicDef *GetMagic(void)
{
    static T32MagicDef magic = { 0, 0, 0, 0 };
    magic.address = T32SymbolAddrGet("OSTCBCur");
    magic.size = T32SymbolSizeGet("OSTCBCur");
    return &magic;
}


/**********************  Calculation of Task List  **********************/
uint32 *GetTaskList(void)
{
    static uint32 magics[100];
    uint i = 0;
    uint32 tcbptr;

    if (!par_tcblist)
	par_tcblist = T32SymbolAddrGet("OSTCBList");

    tcbptr = T32ReadLong(par_tcblist);

    while (tcbptr && (i < 99)) {
	magics[i] = tcbptr;
	tcbptr = T32ReadLong(tcbptr + TCB_NEXT);
	i++;
    }

    magics[i] = 0;

    return magics;
}


/*****************  Calculation of Task Stack Characteristics  ***************/
T32StackDef *GetTaskStack(uint32 magic)
{
    static T32StackDef stack = { 0, 0, 0, 0 };	/* structure to describe task stack */

    stack.magic = magic;
    stack.bottom = T32ReadLong(magic + TCB_STKBASE);
    stack.top = stack.bottom - T32ReadLong(magic + TCB_STKSIZE);
    stack.sp = T32ReadLong(magic + TCB_STKPTR);

    return &stack;
}


/**********************  Command TASK.TaskList  **********************/

/* Parser: returns initialization routine */
T32CmdInitRtn CmdTlParser(void)
{
    return CmdTlInit;		/* Return initialization routine */
}


/* Initialization of tasklist */
T32CmdDisplayDef *CmdTlInit(void)
{
    static T32CmdDisplayDef display =	/* Display definition for TASK.TaskList */
    {
	WINDOW,			/* Type of display              */
	0, 80,			/* default heigth and width     */
	"magic     id   name      state     prio  delay   waiting at",	/* headline */
	CmdTlDisplay		/* Display routine              */
    };

    static T32CmdDisplayDef errdisp = {
	MESSAGE, 0, 0, "Sorry: Couldn't get symbol addresses", 0
    };

    if (!par_magic)
	par_magic = T32SymbolAddrGet("OSTCBCur");

    if (!par_tcblist)
	par_tcblist = T32SymbolAddrGet("OSTCBList");

    if (!par_tcbtab)
	par_tcbtab = T32SymbolAddrGet("OSTCBTbl");

    if (!par_tnum)
	par_tnum = T32SymbolSizeGet("OSTCBTbl") / T32SymbolTypeSizeGet("OS_TCB");

    /* return display definition */
    if (par_tcblist == -1)
	return &errdisp;
    else
	return &display;
}


/* Display Routine */
void CmdTlDisplay(void)
{
    uint32 tcbptr;
    byte tcb[TCB_LEN];

    tcbptr = T32ReadLong(par_tcblist);	/* read first tcb pointer */

    while (tcbptr) {
	T32Printf("%8x ", tcbptr);	/* display magic = tcb address */
	T32Printf("%3d.  ", (tcbptr - par_tcbtab) / TCB_LEN);	/* display task id */
	T32ReadBuffer(tcb, tcbptr, TCB_LEN);	/* read whole tcb */
	T32DisplayStringTarget(T32Extract(tcb + TCB_NAME, 4), 10);	/* display task name */

	/* display task status */
	if (tcbptr == T32ReadLong(par_magic))
	    T32Printf("running   ");
	else
	    switch (*(tcb + TCB_STATE)) {
	    case 0:
		T32Printf("ready     ");
		break;
	    case 1:
		T32Printf("semaphore ");
		break;
	    case 2:
		T32Printf("mailbox   ");
		break;
	    case 3:
		T32Printf("queue     ");
		break;
	    default:
		T32Printf("unknown   ");
		break;
	    }

	T32Printf(" %3d. ", *(tcb + TCB_PRIO));	/* display priority */
	T32Printf("%5d.  ", T32Extract(tcb + TCB_DELAY, 2));	/* display delay */

	/* display waiting at */
	if (*(tcb + TCB_STATE)) {
	    uint32 event = T32Extract(tcb + TCB_EVENT, 4);
	    T32EventDef("TASK.EX.OBJ %x", event);
	    T32Printf("%8x", event);
	    T32Printf(" ");
	    T32EventDef("TASK.EX.OBJ %x", event);
	    T32DisplaySymbol(event, 16);
	}

	/* next tcb */
	T32Printf("\n");
	tcbptr = T32Extract(tcb + TCB_NEXT, 4);
    }
}


/**********************  Command TASK.TASKState  **********************/

/* Parser: returns initialization routine */
T32CmdInitRtn CmdTsParser(void)
{
    return CmdTsInit;
}


/* Initialization of taskstate */
T32CmdDisplayDef *CmdTsInit(void)
{
    static T32CmdDisplayDef display = {
	MESSAGE, 0, 0, 0, CmdTsDisplay
    };

    static T32CmdDisplayDef errdisp = {
	MESSAGE, 0, 0,
	"Sorry: Couldn't get symbol addresses", 0
    };

    if (!par_magic)
	par_magic = T32SymbolAddrGet("OSTCBCur");

    if (!par_tcbtab)
	par_tcbtab = T32SymbolAddrGet("OSTCBTbl");

    if (!par_tnum)
	par_tnum = T32SymbolSizeGet("OSTCBTbl") / T32SymbolTypeSizeGet("OS_TCB");

    if (par_tcbtab == -1)
	return &errdisp;
    else
	return &display;
}


/* Display Routine */
void CmdTsDisplay(void)
{
    uint32 tcbptr;
    uint i;

    /* get number of tasks */
    if (!par_tnum) {
	T32Printf("No Tasks found. Check Configuration.");
	return;
    }

    T32BreakpointSet(par_magic, 4, BP_A);	/* set Alpha breakpoints on magic address */
    tcbptr = par_tcbtab;			/* get first tcb state pointer */

    for (i = 0; i < par_tnum; i++) {
	T32BreakpointSet(tcbptr + TCB_STATE, 1, BP_A);	/* set Alpha breakpoints on state address */
	tcbptr += TCB_LEN;
    }

    T32Printf("Alpha Breakpoints set on %d task state words", par_tnum);
}


/******************  Function TASK.CONFIG  ******************/
uint32 FctTaskConfig(uint32 par_one, uint32 par_two)
{
    T32DebugPrintf("parameter 1: 0x%8x = %s", par_one, (char *) par_one);

    if (!strcmp((char *) par_one, "magic")) {
	if (!par_magic)
	    par_magic = T32SymbolAddrGet("OSTCBCur");
	return par_magic;
    }

    return 0;
}


/******************  Analyzer.LIST Extension  ******************/
T32ALDisplayDef *AnaListInit(void)
{
    static T32ALDisplayDef display =
    {	/* Display Definition   */
	AnaListCheck,		/* Record Check Routine */
	AnaListDisplay		/* Display Routine      */
    };

    if (!par_magic)
	par_magic = T32SymbolAddrGet("OSTCBCur");

    return &display;
}


T32ALCheckResult *AnaListCheck(void)
{
    static T32ALCheckResult result = { 0, 0 };

    if ((T32AnaGetCurrItem(ANA_ACCESS) == ANA_ACCESS_WRITE) && (T32AnaGetCurrItem(ANA_ADDRESS) == par_magic))
	result.lines = 1;
    else
	result.lines = 0;

    return &result;
}


void AnaListDisplay(T32ALCheckResult * result)
{
    uint32 magic;

    magic = T32AnaGetCurrItem(ANA_DATA32);
    T32Printf("--- TASK = %8x = ", magic);
    T32DisplayStringTarget(T32ReadLong(magic + TCB_NAME), 10);
}


/******************  Analyzer.STATistic.TASK  ******************/

T32ASCheckDef *AnaStatInit(void)
{
    static T32ASCheckDef check;

    if (!par_magic)
	par_magic = T32SymbolAddrGet("OSTCBCur");

    check.preview.ignoresize = 4;	/* switch preview filter on */

    /* set address to ignore and addresses to check to magic address */
    check.preview.ignoreaddr = check.preview.checkaddr1 = check.preview.checkaddr2 = par_magic;

    check.check = AnaStatCheck;	/* Specify check routine */

    return &check;
}


uint32 AnaStatCheck(void)
{
    uint32 result = 0;

    if ((T32AnaGetCurrItem(ANA_ACCESS)  == ANA_ACCESS_WRITE) && (T32AnaGetCurrItem(ANA_ADDRESS) == par_magic)) {
	result = T32AnaGetCurrItem(ANA_DATA32);	/* get task magic */
	T32AnaSkipRecord(0);
    }

    return result;
}


/******************  Analyzer.STATistic.TASKstate  ******************/


uint32 ATmagics[100];

T32ATCheckRtn AnaTaskstInit(void)
{
    uint i;
    uint32 taddr;

    if (!par_magic)
	par_magic = T32SymbolAddrGet("OSTCBCur");

    if (!par_tcbtab)
	par_tcbtab = T32SymbolAddrGet("OSTCBTbl");

    if (!par_tnum)
	par_tnum = T32SymbolSizeGet("OSTCBTbl") / T32SymbolTypeSizeGet("OS_TCB");

    if (par_tnum > 100)
	par_tnum = 100;

    taddr = par_tcbtab;

    for (i = 0; i < par_tnum; i++) {
	ATmagics[i] = taddr;
	taddr += TCB_LEN;
    }

    return AnaTaskstCheck;
}


T32ATCheckResult *AnaTaskstCheck(void)
{
    static T32ATCheckResult result = { 0, 0 };
    uint32 addr;

    if (T32AnaGetCurrItem(ANA_ACCESS) != ANA_ACCESS_WRITE) {
	result.magic = 0;
	result.state = 0;
	return &result;
    }

    addr = T32AnaGetCurrItem(ANA_ADDRESS);
    if (addr == par_magic) {
	result.magic = T32AnaGetCurrItem(ANA_DATA32);
	result.state = TS_RUNNING;
	T32AnaSkipRecord(0);
	return &result;
    }

    if (T32AnaTrackTaskState(TCB_STATE, 2, &result)) {
	switch (result.state & 0xff) {
	case 0:
	    result.state = TS_READY;
	    break;
	case 1:
	case 2:
	case 3:
	    result.state = TS_WAITING;
	    break;
	default:
	    result.state = TS_UNDEFINED;
	    break;
	}
    }

    result.magic = 0;
    result.state = 0;

    T32AnaSkipRecord(0);
    return &result;
}

/**eof*/
