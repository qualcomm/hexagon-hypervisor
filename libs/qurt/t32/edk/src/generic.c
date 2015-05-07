/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Development Kit,
 * 'Generic' TASK/EXTension sample.
 *  Copyright Lauterbach GmbH
 * 
 *  indent -linux -i4 -ts8 -l132 generic.c
 *  indent -ts8 -i4 -l132 -bli0 -bap -npsl -npcs -br -ce -cdw -cli0 -cbi0 -sc generic.c
 */
#include "t32ext.h"

/* This sample was originally made for m68k. */
#define CPU "M68K"

/* Prototypes */
uint32 MagicToId(uint32 magic);
char *MagicToName(uint32);
T32MagicDef *GetMagic(void);
uint32 *TaskList(void);
T32StackDef *TaskStack(uint32);

T32CmdInitRtn TlParser(void);
T32CmdDisplayDef *TlInit(void);
void TlDisplayOne(void);
void TlDisplayAll(void);

T32CmdInitRtn MlParser(void);
T32CmdDisplayDef *MlInit(void);
void MlDisplay(void);

T32CmdInitRtn TstParser(void);
T32CmdDisplayDef *TstInit(void);
void TstDisplay(void);

T32CmdInitRtn TestParser(void);
T32CmdDisplayDef *TestInit(void);
void TestDisplay(void);

T32ALDisplayDef *AnaListInit(void);
T32ALCheckResult *AnaListCheck(void);
void AnaListDisplay(T32ALCheckResult *);
T32ASCheckDef *AnaStatInit(void);
uint32 AnaStatCheck(void);
T32ATCheckRtn AnaTaskstInit(void);
T32ATCheckResult *AnaTaskstCheck(void);

uint32 FctTaskConfig(uint32, uint32);

/* Globals */
uint32 par_magic = 0;
uint32 par_tlist = 0;
uint32 par_mlist = 0;
uint32 argvalue;
char argbuffer[32];

/* task control block structure offsets */
#define TCB_ID		0x04
#define TCB_NAME	0x08
#define TCB_PRIO	0x0c
#define	TCB_STATE	0x10
#define TCB_NEXT	0x1c
#define TCB_STKSTRT	0x20
#define TCB_STKEND	0x24
#define TCB_STKPTR	0x28
#define TCB_LEN		0x30

/* mailbox control block structure offsets */
#define MCB_ID		0x04
#define MCB_NAME	0x08
#define MCB_CONT	0x0c
#define MCB_NEXT	0x10
#define MCB_LEN		0x1c

/* Register this TASK extension with TRACE32. */
void main(void)
{
    static T32CmdDef tasklist = { "TASKLIST", "TL", "TaskList", "__EMXXX_TL", TlParser };
    static T32CmdDef mbxlist = { "MBXLIST", "ML", "MbxList", "__EMXXX_ML", MlParser };
    static T32CmdDef taskstate = { "TASKSTATE", "TASKS", "TASKState", "__EMXXX_TASKS", TstParser };
    static T32CmdDef test = { "TEST", "TEST", "", "", TestParser };

    static T32AnaListDef analist = { "__EMXXX_AL", AnaListInit };
    static T32AnaStatDef anastat = { "__EMXXX_AS", AnaStatInit };
    static T32AnaTaskstateDef anataskst = { "__EMXXX_AT", AnaTaskstInit };

    static T32FctDef taskconfig = { "TASK.CONFIG", PAR_HEX, PAR_STRING_DIRECT, PAR_NONE,
	"__EMXXX_FUNC", FctTaskConfig
    };

    /* Get Parameters and init Globals */
    par_magic = T32ArgumentGet(0);
    par_tlist = T32ArgumentGet(1);
    par_mlist = T32ArgumentGet(2);

    /* Define Task Id and Task Name Translation */
    T32DefineTaskId(MagicToId);
    T32DefineTaskName(MagicToName);

    /* Define Task List & Stack Routine */
    T32DefineTaskList(TaskList);
    T32DefineTaskStack(TaskStack);

    /* Define the "magic location" routine */
    T32DefineGetMagic(GetMagic);

    /* Define command extensions */
    T32DefineCommand(&tasklist);
    T32DefineCommand(&mbxlist);
    T32DefineCommand(&taskstate);
    T32DefineCommand(&test);

    /* Define a "TASK.CONFIG" function */
    T32DefineFunction(&taskconfig);

    /* Define the analyzer routines */
    T32DefineAnalyzerList(&analist);
    T32DefineAnalyzerStat(&anastat);
    T32DefineAnalyzerTaskstate(&anataskst);
}


/**********************  Parameter Setup  **********************/
int par_setup(void)
{
    if (!par_magic)
	par_magic = T32SymbolAddrGet("Current_Task");

    if (!par_tlist)
	par_tlist = T32SymbolAddrGet("Created_Tasks_List");

    if (!par_mlist)
	par_mlist = T32SymbolAddrGet("Created_Mailboxes_List");

    if ((par_magic == (uint32) - 1) || (par_tlist == (uint32) - 1))
	return 1;
    else if (par_mlist == (uint32) - 1)
	return 2;
    else
	return 0;
}


/************************* subroutines **********************/


/**********************  Magic to Id Translation  **********************/
/* Magic to Id Translation: for magic display on various task windows */
uint32 MagicToId(uint32 magic)
{
    return (T32ReadLong(magic + TCB_ID));
}


/**********************  Magic to Name Translation  **********************/
char *MagicToName(uint32 magic)
{
    static char name[20];
    uint32 nameptr;

    nameptr = T32ReadLong(magic + TCB_NAME);
    T32ReadBuffer(name, nameptr, 16);
    name[16] = 0;

    return (name);
}


/**********************  Calculation of "magic" location  **********************/
T32MagicDef *GetMagic(void)
{
    static T32MagicDef magic = { 0, 0, 0, 0 };
    magic.address = T32SymbolAddrGet("Current_Task");
    magic.size = 4;
    return &magic;
}


/**********************  Task List Routine  **********************/
uint32 *TaskList(void)
{
    static uint32 tcbs[100];
    uint32 tcbptr;
    uint i = 0;

    par_setup();

    tcbptr = T32ReadLong(par_tlist);

    while (tcbptr && (i < 99)) {
	tcbs[i++] = tcbptr;
	tcbptr = T32ReadLong(tcbptr + TCB_NEXT);
    }

    tcbs[i] = 0;
    return tcbs;
}


/**********************  Task Stack Routine  **********************/
T32StackDef *TaskStack(uint32 magic)
{
    static T32StackDef stack = { 0, 0, 0, 0 };

    stack.magic = magic;
    stack.top = T32ReadLong(magic + TCB_STKSTRT);
    stack.bottom = T32ReadLong(magic + TCB_STKEND);
    stack.sp = T32ReadLong(magic + TCB_STKPTR);

    return &stack;
}


/************************* TASK.TASKLIST **********************/

/* Parser: returns initialization routine */
T32CmdInitRtn TlParser(void)
{
    argvalue = T32ParseValue("<task>");

    return TlInit;
}


/* Initialization */
T32CmdDisplayDef *TlInit(void)
{
    static const char header[] = "magic  id   state    prio  name";
    static T32CmdDisplayDef displayone = { WINDOW, 0, 80, header, TlDisplayOne };
    static T32CmdDisplayDef displayall = { WINDOW, 0, 80, header, TlDisplayAll };
    static T32CmdDisplayDef errdisp = { MESSAGE, 0, 0, "Sorry: Couldn't get symbol addresses", 0 };

    if (par_setup() == 1)
	return &errdisp;

    if (argvalue)
	return &displayone;
    else
	return &displayall;
}


/* Display Routines */
void tldisplayline(uint32 tcbptr, byte * tcb, uint32 running)
{
    uint32 tmp;

    /* display magic = tcb address */
    T32Printf("%4x  ", tcbptr);

    /* display task id */
    T32Printf("%3d.  ", T32Extract(tcb + TCB_ID, 4));

    /* display task status */
    switch (*(tcb + TCB_STATE)) {
    case 0:
	T32Printf("RUNNING   ");
	break;
    case 1:
	T32Printf("READY     ");
	break;
    case 2:
	T32Printf("WAITING   ");
	break;
    case 3:
	T32Printf("SUSPENDED ");
	break;
    default:
	T32Printf("unknown   ");
	break;
    }

    /* display priority */
    T32Printf("%3d. ", *(tcb + TCB_PRIO));

    /* display task name */
    T32DisplayStringTarget(T32ReadLong(tcbptr + TCB_NAME), 16);

    T32Printf("\n");
}


void TlDisplayOne(void)
{
    uint32 tcbptr, running;
    byte tcb[TCB_LEN];

    tcbptr = argvalue;
    running = T32ReadLong(par_magic);

    /* read whole tcb */
    T32ReadBuffer(tcb, tcbptr, TCB_LEN);

    tldisplayline(tcbptr, tcb, running);
    T32Printf("\n");

    /* display extensions */

    T32DisplayAttribute(ATTR_GREY_UL);
    T32Printf("something additional\n");
    T32DisplayAttribute(ATTR_DEFAULT);

}


void TlDisplayAll(void)
{
    uint32 tcbptr, running;
    byte tcb[TCB_LEN];

    /* read first tcb pointer */
    tcbptr = T32ReadLong(par_tlist);
    running = T32ReadLong(par_magic);

    while (tcbptr) {
	/* read whole tcb */
	T32ReadBuffer(tcb, tcbptr, TCB_LEN);

	tldisplayline(tcbptr, tcb, running);

	/* next tcb */
	tcbptr = T32Extract(tcb + TCB_NEXT, 4);
    }
}


/************************* TASK.MBXLIST **********************/

/* Parser: returns initialization routine */
T32CmdInitRtn MlParser(void)
{
    return MlInit;
}


/* Initialization */
T32CmdDisplayDef *MlInit(void)
{
    static const char header[] = "magic  id   content  name";
    static T32CmdDisplayDef display = { WINDOW, 0, 80, header, MlDisplay };
    static T32CmdDisplayDef errdisp = { MESSAGE, 0, 0, "Sorry: Couldn't get symbol addresses", 0 };

    if (par_setup())
	return &errdisp;

    return &display;
}


/* Display Routines */
void MlDisplay(void)
{
    uint32 mcbptr;
    byte mcb[MCB_LEN];

    /* read first mcb pointer */
    mcbptr = T32ReadLong(par_mlist);

    while (mcbptr) {
	/* read whole mcb */
	T32ReadBuffer(mcb, mcbptr, MCB_LEN);

	/* display magic = mcb address */
	T32Printf("%4x  ", mcbptr);

	/* display mailbox id */
	T32Printf("%3d.  ", T32Extract(mcb + MCB_ID, 4));

	/* display content */
	T32Printf("%8x  ", T32Extract(mcb + MCB_CONT, 4));

	/* display mailbox name */
	T32DisplayStringTarget(T32ReadLong(mcbptr + MCB_NAME), 16);

	T32Printf("\n");

	/* next mcb */
	mcbptr = T32Extract(mcb + MCB_NEXT, 4);
    }
}


/*********************  Command for Task State Marking  *********************/

/* Parser: returns initialization routine */
T32CmdInitRtn TstParser(void)
{
    return TstInit;
}


/* Initialization of taskstate */
T32CmdDisplayDef *TstInit(void)
{
    static T32CmdDisplayDef display = { MESSAGE, 0, 0, 0, TstDisplay };
    static T32CmdDisplayDef errdisp = { MESSAGE, 0, 0, "Sorry: Couldn't get symbol addresses", 0 };

    if (par_setup() == 1)
	return &errdisp;
    else
	return &display;
}


/* Display Routine */
void TstDisplay(void)
{
    uint32 tcbptr;
    uint i = 0;

    /* set Alpha breakpoints on magic address */
    T32BreakpointSet(par_magic, 4, BP_A);

    tcbptr = T32ReadLong(par_tlist);

    while (tcbptr && (i < 99)) {
	/* set Alpha breakpoints on state address */
	T32BreakpointSet(tcbptr + TCB_STATE, 1, BP_A);
	i++;
	tcbptr = T32ReadLong(tcbptr + TCB_NEXT);
    }

    T32Printf("Alpha Breakpoints set on %d task state words", i);
}


/*********************  Command for Task Test  *********************/

/* Parser: returns initialization routine */
T32CmdInitRtn TestParser(void)
{
    return TestInit;
}


/* Initialization of test */
T32CmdDisplayDef *TestInit(void)
{
    static const char header[] = "argument         value     symbol";
    static T32CmdDisplayDef display = { WINDOW, 0, 60, header, TestDisplay };

    par_setup();
    return &display;
}


void TestDisplay(void)
{
    const char *cpu = CPU;
    T32Printf("magic            %8x  ", par_magic);
    T32DisplaySymbol(par_magic, 32);
    T32Printf("\nTask List        %8x  ", par_tlist);
    T32DisplaySymbol(par_tlist, 32);
    T32Printf("\nMailbox List     %8x  ", par_mlist);
    T32DisplaySymbol(par_mlist, 32);
    T32Printf("\nMTD Version      %s  %s  ", cpu, __DATE__);
    T32Printf("\n");
}


/************************* A.LIST **********************/


T32ALDisplayDef *AnaListInit(void)
{
    static T32ALDisplayDef display = { AnaListCheck, AnaListDisplay };

    par_setup();
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
    T32Printf("--- TASK id = %d.", T32AnaGetCurrItem(ANA_DATA32));
}


/************************* A.STAT **********************/


T32ASCheckDef *AnaStatInit(void)
{
    static T32ASCheckDef check;

    par_setup();

    /* switch preview filter on */
    check.preview.ignoresize = 4;
    check.preview.ignoreaddr = check.preview.checkaddr1 = check.preview.checkaddr2 = par_magic;

    check.check = AnaStatCheck;
    return &check;
}


uint32 AnaStatCheck(void)
{
    uint32 result = 0;

    if ((T32AnaGetCurrItem(ANA_ACCESS) == ANA_ACCESS_WRITE) && (T32AnaGetCurrItem(ANA_ADDRESS) == par_magic)) {
	/* get task magic */
	result = T32AnaGetCurrItem(ANA_DATA32);
	T32AnaSkipRecord(0);
    }

    return result;
}


/************************* A.STAT.TASKS **********************/


T32ATCheckRtn AnaTaskstInit(void)
{
    /* Reading of all magics is done in TaskList() */
    par_setup();
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


/******************  Function TASK.CONFIG  ******************/
uint32 FctTaskConfig(uint32 par_one, uint32 par_two)
{
    T32DebugPrintf("parameter 1: 0x%8x = %s", par_one, (char *) par_one);

    if (!strcmp((char *) par_one, "magic")) {
	if (!par_magic)
	    par_magic = T32SymbolAddrGet("Current_Task");
	return par_magic;
    }

    if (!strcmp((char *) par_one, "magicsize"))
	return 4;

    T32FunctionError("unknown topic");
    return 0;
}


/*eof*/
