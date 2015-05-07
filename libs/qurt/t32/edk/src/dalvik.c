/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Development Kit,
 *  Dalvik VM Awareness EXTension.
 *  Copyright Lauterbach GmbH
 *  Use with TRACE32 build 37970 or later.
 *
 *  indent -linux -i4 -ts8 -l132 dalvik.c
 *  indent -ts8 -i4 -l132 -bli0 -bap -npsl -npcs -br -ce -cdw -cli0 -cbi0 -sc dalvik.c
 */
#include "t32ext.h"

// Configuration defines
#define VM_MAX_TASK        TASK_NUMBER_MAX
#define VM_MAX_NAME_LEN    TASK_NAMELEN_SZ
#define VM_MAX_STACKDEPTH  20
#define VM_MAX_DVMDEPTH    16
#define VM_VN_DVMFPTR      "rFP"
// Overwritable Configuration Values
static char *VM_PROCNAME = "t32_dalvik_vm";	// fallback: "dalvik_mterp"
static char *VM_VN_DVMSTATE = "rUNKNOWN";	// rGLUE or rSELF

// Readability defines
enum
{ DVM_NONE = 0, DVM_PROCESS, DVM_THREAD };

// VMList Globals
static T32TaskEntry tasklist[VM_MAX_TASK];
static char name[4096];

// VMView Globals
static magic32_t magic_view;
static char magic_name[VM_MAX_NAME_LEN];

// Display mode global
static uint8 displaymode;

// Common Globals
static uint32 tasks_read = 0;
static uint32 ofs_TASK_mm, siz_TASK_name, ofs_TASK_name, ofs_TASK_p_pptr, ofs_MM_arg_start;
static uint32 ofs_DVM_IS_Method, siz_DVM_Method, ofs_DVM_MethodClazz, ofs_DVM_MethodName;
static uint32 siz_DVM_ClassObject, ofs_DVM_ClassDesc, ofs_DVM_ClassSource, ofs_DVM_pDvmDex;
static uint32 siz_DVM_JStackSave, ofs_DVM_JStackPrev, ofs_DVM_JStackMethod, ofs_DVM_JStackCurPC;
static uint32 siz_DVM_Dex, ofs_DVM_DexHeader, ofs_DVM_MemMapAddr, ofs_DVM_MemMapSize;
static uint32 flg_initvars = 0;

// Local Prototypes
static void CmdVMListDisplay(void);
static void CmdVMViewDisplay(void);
static void CmdVMAInfoDisplay(void);
static void CmdVMClassload(void);
static T32CmdDisplayDef *CmdVMListInit(void);
static T32CmdDisplayDef *CmdVMViewInit(void);
static T32CmdDisplayDef *CmdVMAInfoInit(void);
static T32CmdDisplayDef *CmdVMClassloadInit(void);
static T32CmdInitRtn CmdVMListParser(void);
static T32CmdInitRtn CmdVMViewParser(void);
static T32CmdInitRtn CmdVMAInfoParser(void);
static T32CmdInitRtn CmdVMClassloadParser(void);
static void initvars(void);

// libc/built-in prototypes (see edklibc.h)
typedef __typeof__(sizeof(int)) size_t;
char *strcpy(char *dst, const char *src);
void *memset(void *dst, int val, size_t len);

/****************** Initialize Extension ***********************/

/** Register this TASK extension with TRACE32. */
void main(void)
{
    //       new EXT.* command:  long_cmd,  short,  softkey,   help_spec,               parser_routine
    static T32CmdDef vmlist = { "VMLIST", "VML", "VMList", "__VM_DALVIK__EXT_VML", CmdVMListParser };
    static T32CmdDef vmview = { "VMVIEW", "VMV", "VMView", "__VM_DALVIK__EXT_VMV", CmdVMViewParser };
    static T32CmdDef vmainfo = { "VMAINFO", "VMAI", "VMAInfo", "__VM_DALVIK__EXT_VMAI", CmdVMAInfoParser };
    static T32CmdDef vmcload = { "VMCLASS", "VMC", "VMClass", "__VM_DALVIK__EXT_VMCL", CmdVMClassloadParser };
    // Init Globals
    magic_view = 0;
    initvars();
    // Define commands
    T32DefineCommand(&vmlist);
    T32DefineCommand(&vmview);
    T32DefineCommand(&vmainfo);
    T32DefineCommand(&vmcload);
}

static void initvars()
{
    if (flg_initvars > 0)
	return;
    /* fallback for older setup scripts */
    {
	long test = T32SymbolAddrGet(VM_PROCNAME);
	if (test == -1)
	    VM_PROCNAME = "dalvik_mterp";
    }
    /* Linux - kernel structs */
    ofs_TASK_mm = T32SymbolTypeOffsetGet("struct task_struct.mm");
    siz_TASK_name = T32SymbolTypeSizeGet("struct task_struct.comm");
    ofs_TASK_name = T32SymbolTypeOffsetGet("struct task_struct.comm");
    ofs_TASK_p_pptr = T32SymbolTypeOffsetGet("struct task_struct.parent");	// 2.6
    ofs_MM_arg_start = T32SymbolTypeOffsetGet("struct mm_struct.arg_start");
    if (ofs_TASK_p_pptr == -1)
	ofs_TASK_p_pptr = T32SymbolTypeOffsetGet("struct task_struct.p_pptr");	// 2.4
    if (siz_TASK_name > VM_MAX_NAME_LEN)
	siz_TASK_name = VM_MAX_NAME_LEN;
    /* Dalvik VM - interpreter (save) state */
    ofs_DVM_IS_Method = T32SymbolTypeOffsetGet("struct InterpState.method");	// Android 2.x
    VM_VN_DVMSTATE = "rGLUE";
    if (ofs_DVM_IS_Method == -1) {
	ofs_DVM_IS_Method = T32SymbolTypeOffsetGet("struct Thread.interpSave.method");	// Android 4.x
	VM_VN_DVMSTATE = "rSELF";
    }
    /* Dalvik VM - method struct */
    siz_DVM_Method = T32SymbolTypeSizeGet("struct Method");
    ofs_DVM_MethodClazz = T32SymbolTypeOffsetGet("struct Method.clazz");
    ofs_DVM_MethodName = T32SymbolTypeOffsetGet("struct Method.name");
    /* Dalvik VM - class object struct */
    siz_DVM_ClassObject = T32SymbolTypeSizeGet("struct ClassObject");
    ofs_DVM_ClassDesc = T32SymbolTypeOffsetGet("struct ClassObject.descriptor");
    ofs_DVM_ClassSource = T32SymbolTypeOffsetGet("struct ClassObject.sourceFile");
    ofs_DVM_pDvmDex = T32SymbolTypeOffsetGet("struct ClassObject.pDvmDex");
    /* Dalvik VM - Java call stack */
    siz_DVM_JStackSave = T32SymbolTypeSizeGet("struct StackSaveArea");
    ofs_DVM_JStackPrev = T32SymbolTypeOffsetGet("struct StackSaveArea.prevFrame");
    ofs_DVM_JStackMethod = T32SymbolTypeOffsetGet("struct StackSaveArea.method");
    ofs_DVM_JStackCurPC = T32SymbolTypeOffsetGet("struct StackSaveArea.xtra.currentPc");
    /* Dalvik VM - Dex File */
    siz_DVM_Dex = T32SymbolTypeSizeGet("struct DvmDex");
    ofs_DVM_DexHeader = T32SymbolTypeOffsetGet("struct DvmDex.pHeader");
    ofs_DVM_MemMapAddr = T32SymbolTypeOffsetGet("struct DvmDex.memMap.addr");
    ofs_DVM_MemMapSize = T32SymbolTypeOffsetGet("struct DvmDex.memMap.length");
    /* Display mode */
    displaymode = 0;
    /* init complete */
    flg_initvars++;
}

/** check if the global data space holds task list data
    - if yes: get it
    - if not: discover data and copy to global
*/
static void syncglobaldata(void)
{
    if (SYSTEM_RUNNING == T32GetStatus()) {
	/* make sure we also reset any stored data */
	tasks_read = 0;
	T32CopyToGlobal((uint8 *) & tasks_read, sizeof(uint32));
	T32DebugPrintf("syncglobaldata: *** Run/Step detected ***");
	return;
    }

    T32CopyFromGlobal((uint8 *) & tasks_read, sizeof(uint32));
    if (tasks_read) {
	T32CopyFromGlobal((uint8 *) tasklist, sizeof(tasklist));
    } else {
	tasks_read = T32ReadTaskList(tasklist, VM_MAX_TASK);
	T32CopyToGlobal((uint8 *) tasklist, sizeof(tasklist));
	T32CopyToGlobal((uint8 *) & tasks_read, sizeof(uint32));
    }
}

/********************** Command EXT.VMList **********************/

/** Parse arguments and return pointer to init routine. */
T32CmdInitRtn CmdVMListParser(void)
{
    displaymode = T32ParseValue("dispmode");
    T32CopyToGlobal((uint8 *) & displaymode, sizeof(displaymode));
    return CmdVMListInit;	/* return initialization routine */
}

/** Initialize Extension. */
T32CmdDisplayDef *CmdVMListInit(void)
{
    static const char *header[] = { "magic   ", "pid ", "process/task name           ",
	"method name       ", "class descriptor   ", "class source ", "lvl", 0
    };
    static T32CmdDisplayDef display =	/* Display definition */
    {
	WINDOW2,		/* type of display */
	0, 100,			/* default heigth and width */
	(char *) header,	/* headline */
	CmdVMListDisplay	/* display routine */
    };
    return &display;
}

/** Compute and display the list of DalvikVM Processes and Threads. */
void CmdVMListDisplay(void)
{
    addr32_t  arg_start, mm, parent, taskptr;
    uint32    spaceid, curpid, dvmProcess;
    int dvmType, namelen, i, layer, rc, fTreeOpen;
    char tname[VM_MAX_NAME_LEN] = "ttest";
    char aname[VM_MAX_NAME_LEN] = "atest";
    char *vmname = (char *) 0;

    initvars();
    syncglobaldata();

    if (tasks_read == 0) {
	T32Printf("(no data)\n");
	return;
    }
    if (tasks_read > VM_MAX_TASK) {
	T32DisplayAttribute(ATTR_ERROR);
	T32Printf("(more data available than we can get...)\n");
	T32DisplayAttribute(ATTR_DEFAULT);
	tasks_read = VM_MAX_TASK;
    }

    T32DebugPrintf("dalvik.t32: VMList process scan -------------------");
    dvmProcess = -1;
    // Note: loop assumes that tasklist is sorted by (1)spaceid, (2)pid
    fTreeOpen = 0;
    for (i = 0; i < tasks_read; i++) {
	dvmType = DVM_NONE;	// reset marker for DVM process or subthread
	taskptr = tasklist[i].magic;   // magic value is also address
	spaceid = tasklist[i].spaceid;
	curpid = tasklist[i].pid;
	if (curpid == spaceid) {	// we are Process leader, check our parent
	    T32SetMemoryAccessSpaceId(spaceid);
	    rc = T32CheckMemory(taskptr + ofs_TASK_mm, 4);
	    if (!rc) {
		mm = T32ReadLong(taskptr + ofs_TASK_mm);
		if (mm == 0)
		    rc = -1;	// also don't like null pointer
	    }
	    if (!rc) {
		arg_start = T32ReadLong(mm + ofs_MM_arg_start);
		rc = T32CheckMemory(taskptr + ofs_TASK_name, siz_TASK_name);
	    }
	    if (!rc) {
		T32ReadBuffer(tname, taskptr + ofs_TASK_name, siz_TASK_name);
		tname[VM_MAX_NAME_LEN - 1] = '\0';
		if (vmname) {
		    parent = T32ReadLong(taskptr + ofs_TASK_p_pptr);
		    if (parent) {
			rc = T32CheckMemory(parent + ofs_TASK_name, siz_TASK_name);
			if (!rc) {
			    char pname[VM_MAX_NAME_LEN] = "[NOACCESS]";
			    T32ReadBuffer(pname, parent + ofs_TASK_name, siz_TASK_name);
			    pname[VM_MAX_NAME_LEN - 1] = '\0';
			    if (!strcmp(pname, vmname)) {
				dvmType = DVM_PROCESS;
				dvmProcess = spaceid;
			    }
			}
		    }
		} else if (!strcmp(tname, "app_process")) {
		    // no "dvmType=DVM_PROCESS;" : does itself not run VM code
		    vmname = "app_process";
		} else if (!strcmp(tname, "zygote")) {
		    // no "dvmType=DVM_PROCESS;" : does itself not run VM code
		    vmname = "zygote";
		}
	    }			// else do_not_know_if_this_is_a_dvmProcess
	} else if (dvmProcess == spaceid) {
	    dvmType = DVM_THREAD;
	}			// else {processing_for_other_tasks(); }

	if (dvmType) {
	    addr32_t address = 0x4711, addrvar = 0x0815;
	    uint32   level;
	    uint8   fDvmFound = 0, fVisible = 0;
	    if (T32IsLineDisplayed(1) || (displaymode == 1)) {
		if (dvmType == DVM_PROCESS) {
		    fVisible = 1;
		}
		if (fTreeOpen && (dvmType == DVM_THREAD)) {
		    fVisible = 1;
		}
		if (fVisible) {
		    T32EventDef("EXT.VMView %x", taskptr);
		    T32MenuDef("EXT.VMV");
		    T32DisplayHex8(taskptr);
		    T32DisplaySeparator();
		    T32DisplayHex4(curpid);
		    T32DisplaySeparator();
		    T32SetMemoryAccessSpaceId(spaceid);
		    if (dvmType == DVM_PROCESS) {
			fTreeOpen = T32TreeDef(spaceid);
			T32EventDef("EXT.VMView %x", taskptr);
			T32MenuDef("EXT.VMV");
			T32DisplayStringTarget(arg_start, VM_MAX_NAME_LEN);
		    }
		    if (dvmType == DVM_THREAD) {
			T32DisplayString("   ");
			T32EventDef("EXT.VMView %x", taskptr);
			T32MenuDef("EXT.VMV");
			T32DisplayStringTarget(taskptr + ofs_TASK_name, siz_TASK_name);
		    }
		    T32DisplaySeparator();
		    if (fTreeOpen) {
			// speedup: do call stack analysis only if tree is open
			for (level = 0; level < VM_MAX_STACKDEPTH; level++) {
			    rc = T32TaskStackGetCaller(taskptr, level, name, &address);
			    if (address == 0)
				break;
			    if (!strcmp(name, VM_PROCNAME)) {
				rc = T32TaskStackGetPointer(taskptr, level, VM_VN_DVMSTATE, &addrvar);
				if (!rc && !!addrvar) {
				    uint32 pMethod, pMethodName, pClass, pClassDesc, pClassSrc, pDvmDex, pHeader;
				    T32SetMemoryAccessSpaceId(spaceid);
				    pMethod = T32ReadLong(addrvar + ofs_DVM_IS_Method);	// InterpState->Method
				    rc = T32CheckMemory(pMethod, siz_DVM_Method);
				    if (!rc) {
					pMethodName = T32ReadLong(pMethod + ofs_DVM_MethodName);	// Method->name
					pClass = T32ReadLong(pMethod + ofs_DVM_MethodClazz);	// Method->ClassObject
					rc = T32CheckMemory(pClass, siz_DVM_ClassObject);
					if (!rc) {
					    pClassDesc = T32ReadLong(pClass + ofs_DVM_ClassDesc);	// ClassObject->descriptor
					    pClassSrc = T32ReadLong(pClass + ofs_DVM_ClassSource);	// ClassObject->sourceFile
					    pDvmDex = T32ReadLong(pClass + ofs_DVM_pDvmDex);	// ClassObject->pDvmDex
					    if (!!pDvmDex)
						pHeader = T32ReadLong(pDvmDex + ofs_DVM_DexHeader);	// DvmDex->pHeader
					    T32DisplayStringTarget(pMethodName, VM_MAX_NAME_LEN);
					    T32DisplaySeparator();
					    if (!!pClass && !!pClassDesc && !!pHeader) {
						rc = T32CheckMemory(pClassDesc, 2);	// check if at least minimum length is accessible
						if (!rc) {
						    char buf[256];
						    rc = T32ReadBuffer(buf, pClassDesc, 255);
						    buf[255] = '\0';
						    T32EventStringDef("JAVA.LOADCLASS %a %s", pHeader, buf);	// new '%a' syntax adds spaceid
						}
					    }
					    T32DisplayStringTarget(pClassDesc, VM_MAX_NAME_LEN);
					    T32DisplaySeparator();
					    T32DisplayStringTarget(pClassSrc, VM_MAX_NAME_LEN);
					    fDvmFound = 1;	// VM found with accessible 'Method' and "ClassObject' structures
					}
				    }
				}	// if memory is accessible and addrvar is not not nullptr
				break;	// always exit if VM_PROCNAME was found
			    }	// if dvm process found
			}	// for level
			if (!fDvmFound) {
			    T32DisplaySeparator();
			    T32DisplaySeparator();
			}
			T32DisplaySeparator();
			T32Printf("%3d", level);	// maximum stack level analyzed (or where DVM was found)
			T32DisplaySeparator();
			T32Printf("\n");
		    } else {
			T32DisplaySeparator();
			T32DisplaySeparator();
			T32DisplaySeparator();
			T32DisplaySeparator();
			T32Printf("\n");
		    }
		}		// if entry visible
	    } else {		// if line displayed
		T32Printf("\n");	// line is not displayed (\n to move forward one line)
	    }
	}			// if dvmType
	T32MemoryAccess(MEM_ACCESS_DEFAULT);
    }				// for all tasks
}

/********************** Command EXT.VMView **********************/

/** Parse arguments and return pointer to init routine. */
T32CmdInitRtn CmdVMViewParser(void)
{
    memset(magic_name, 0, VM_MAX_NAME_LEN);
    magic_view = T32ParseValueOrString(magic_name, VM_MAX_NAME_LEN, "magic");
    return CmdVMViewInit;	/* Return initialization routine */
}

/** Initialize Extension. */
T32CmdDisplayDef *CmdVMViewInit(void)
{
    static const char *header[] = { "method name       ", "class descriptor                         ",
	"class source         ", "frmCurPC", "frameptr", 0
    };
    static T32CmdDisplayDef display = { WINDOW2, 0, 100, (char *) header, CmdVMViewDisplay };
    return &display;
}

/** Display DalvikVM Stack for given task. */
void CmdVMViewDisplay(void)
{
    static uint32 spaceid;
    static uint32 curpid;
    static uint32 magic_idx = 0;
    addr32_t address, pDvmState, pFramePrev, pFrameMethod, pFrameCurPC, pMethodName, pLocalFP;
    uint32 level, dvmlvl;
    int rc;

    initvars();

    T32DebugPrintf("dalvik.t32: VMView DVM stack scan -----------------");

    if (tasks_read == 0 || magic_idx == 0) {
	// only do this if not already initialized
	int i;
	syncglobaldata();
	if (magic_view) {
	    for (i = 0; i < tasks_read; i++) {
		if (magic_view == tasklist[i].magic) {
		    magic_idx = i;
		    break;
		}
	    }
	} else if (!!magic_name[0]) {
	    for (i = 0; i < tasks_read; i++) {
		if (!strcmp(magic_name, tasklist[i].name)) {
		    magic_view = tasklist[i].magic;
		    magic_idx = i;
		    break;
		}
	    }
	}
	if (magic_idx == 0) {
	    T32Printf("(no data) magic_view=%x\n", magic_view);
	    return;

	}
	spaceid = tasklist[magic_idx].spaceid;
	curpid = tasklist[magic_idx].pid;
    }

    T32SetMemoryAccessSpaceId(spaceid);

    for (level = 0; level < VM_MAX_STACKDEPTH; level++) {
	rc = T32TaskStackGetCaller(magic_view, level, name, &address);
	if (address == 0)
	    break;
	if (!strcmp(name, VM_PROCNAME))	// name found -> stop
	    break;
    }

    // show DVM (JAVA) stack
    pLocalFP = 0;
    rc = T32TaskStackGetPointer(magic_view, level, VM_VN_DVMFPTR, &pLocalFP);
    if (!rc && !!pLocalFP) {
	for (dvmlvl = 0; dvmlvl < VM_MAX_DVMDEPTH; dvmlvl++) {
	    char *NAreason = "NO_ACCESS";
	    addr32_t pClass = 0;
	    addr32_t pClassDesc = 0;
	    addr32_t pClassSrc = 0;
	    addr32_t pDvmDex = 0;
	    addr32_t pHeader = 0;
	    pLocalFP -= siz_DVM_JStackSave;	// rFP points to END of "StackSaveArea"
	    rc = T32CheckMemory(pLocalFP, siz_DVM_JStackSave);
	    if (!!rc) {
		T32Printf("[StackSaveArea unreadable (pid=%4x). Stop.]\n", curpid);
		break;
	    }
	    pFramePrev = T32ReadLong(pLocalFP + ofs_DVM_JStackPrev);
	    pFrameMethod = T32ReadLong(pLocalFP + ofs_DVM_JStackMethod);
	    pFrameCurPC = T32ReadLong(pLocalFP + ofs_DVM_JStackCurPC);

	    if (T32IsLineDisplayed(1)) {
		rc = T32CheckMemory(pFrameMethod, siz_DVM_Method);
		if (!rc) {
		    uint32 pMethodName;
		    pMethodName = T32ReadLong(pFrameMethod + ofs_DVM_MethodName);	// FrameMethod->name
		    T32DisplayStringTarget(pMethodName, VM_MAX_NAME_LEN);
		    T32DisplaySeparator();
		} else {
		    NAreason = (!pFrameMethod) ? "(n/a)" : "NO_ACCESS";
		    T32DisplayString(NAreason);
		    T32DisplaySeparator();
		}
		if (!rc) {
		    pClass = T32ReadLong(pFrameMethod + ofs_DVM_MethodClazz);	// FrameMethod->ClassObject
		    rc = T32CheckMemory(pClass, siz_DVM_ClassObject);
		}
		if (!rc) {
		    pClassDesc = T32ReadLong(pClass + ofs_DVM_ClassDesc);	// ClassObject->descriptor
		    pClassSrc = T32ReadLong(pClass + ofs_DVM_ClassSource);	// ClassObject->sourceFile
		    pDvmDex = T32ReadLong(pClass + ofs_DVM_pDvmDex);		// ClassObject->pDvmDex
		    if (!!pDvmDex)
			pHeader = T32ReadLong(pDvmDex + ofs_DVM_DexHeader);	// DvmDex->pHeader
		    if (!!pClass && !!pClassDesc && !!pHeader) {
			rc = T32CheckMemory(pClassDesc, 128);
			if (!rc) {
			    char buf[256];
			    rc = T32ReadBuffer(buf, pClassDesc, 256);
			    buf[255] = '\0';
			    T32EventStringDef("JAVA.LOADCLASS %a %s", pHeader, buf);
			    //T32DebugPrintf("[prep] JAVA.LOADCLASS %a %s", pHeader, buf);
			}
		    }
		    if (!!pClassDesc)
			T32DisplayStringTarget(pClassDesc, VM_MAX_NAME_LEN);
		    else
			T32DisplayString("(NULL)");
		    T32DisplaySeparator();
		    if (!!pClassSrc)
			T32DisplayStringTarget(pClassSrc, VM_MAX_NAME_LEN);
		    else
			T32DisplayString("(NULL)");
		    T32DisplaySeparator();
		} else {
		    T32DisplayString(NAreason);
		    T32DisplaySeparator();
		    T32DisplayString(NAreason);
		    T32DisplaySeparator();
		}
		T32EventDef("Data.ListJava %a", pFrameCurPC);	// new '%a' syntax adds spaceid (J:%x is address only)
		/* TRACE32 build 33924 and later supports setting the memory access type to "J"
		 * before event definition and to "D" afterwards; this allows definitions
		 * of the type "Data.List J:<spaceid>:<address>".
		 */
		T32Printf("%8x", pFrameCurPC);
		T32DisplaySeparator();
		/* change from %x to %a after Var.View accepts 3-part addresses */
		if (1 == 1) {
		    uint8 i, v; 
		    char  z[] = "Var.View (StackSaveArea*)(0x0000:%x)";
		    for (i = 0; i < 4; i++) {
			v = (char) ((spaceid >> (12 - i * 4)) & 0x0f);
			if (v > 9)
			    v = v - 10 + 'A';
			else
			    v = v + '0';
			z[28 + i] = v;
		    }
		    T32DebugPrintf("###+### spaceid = %x, event='%s'", spaceid, z);
		    T32EventDef(z, pLocalFP);
		} else {
		    T32EventDef("Var.View (StackSaveArea*)(%a)", pLocalFP);
		}
		T32Printf("%8x", pLocalFP);
		T32DisplaySeparator();
	    }			//if line displayed
	    T32Printf("\n");
	    pLocalFP = pFramePrev;
	    if (!pLocalFP)
		break;		// end of frame pointer chain
	}			// end_for dvmlvl (Java stack frames)
    }
    
    T32MemoryAccess(MEM_ACCESS_DEFAULT);
}

/********************** Command EXT.VMAInfo *********************/
/** Show DalvikVM Awareness Information. */
void CmdVMAInfoDisplay(void)
{
    T32CopyFromGlobal((uint8 *) & displaymode, sizeof(displaymode));
    T32Printf("Dalvik VM Awareness\n\nCompiled: " __DATE__
	      "\nTaskMax#: %d.\nDispMode: %d (%s)\n",
	      VM_MAX_TASK, displaymode, (displaymode == 0) ? "Fast" : (displaymode == 1) ? "Full" : "Unknown");
}

T32CmdDisplayDef *CmdVMAInfoInit(void)
{
    static T32CmdDisplayDef display = { WINDOW, 10, 40, "Status", CmdVMAInfoDisplay };
    return &display;
}

T32CmdInitRtn CmdVMAInfoParser(void)
{
    return CmdVMAInfoInit;	/* return initialization routine */
}

/********************** Command EXT.VMClassload **********************/

/** Globals. */
static magic32_t vmc_magicview;
static char vmc_magicname[VM_MAX_NAME_LEN];

/** Parse arguments and return pointer to init routine. */
T32CmdInitRtn CmdVMClassloadParser(void)
{
    memset(vmc_magicname, 0, VM_MAX_NAME_LEN);
    vmc_magicview = T32ParseValueOrString(vmc_magicname, VM_MAX_NAME_LEN, "magic");
    T32DebugPrintf("EXT.VMC - parse");
    return CmdVMClassloadInit;
}

/** Initialize Extension. */
T32CmdDisplayDef *CmdVMClassloadInit(void)
{
    static T32CmdDisplayDef display = { MESSAGE, 0, 0, 0, CmdVMClassload };
    T32DebugPrintf("EXT.VMC - display");
    return &display;
}

/** Issue Java.LOADCLASS commands for a given task. */
void CmdVMClassload(void)
{
    static uint32 spaceid = 0;
    static uint32 curpid = 0;
    static uint32 magic_idx = 0;
    addr32_t address, pDvmState, pFramePrev, pFrameMethod, pFrameCurPC, pMethodName, pLocalFP;
    uint32 level, dvmlvl;
    int rc;

    initvars();

    if (tasks_read == 0 || magic_idx == 0) {
	// only do this if not already initialized
	int i;
	syncglobaldata();
	if (vmc_magicview) {
	    for (i = 0; i < tasks_read; i++) {
		if (vmc_magicview == tasklist[i].magic) {
		    magic_idx = i;
		    break;
		}
	    }
	} else if (!!vmc_magicname[0]) {
	    for (i = 0; i < tasks_read; i++) {
		if (!strcmp(vmc_magicname, tasklist[i].name)) {
		    vmc_magicview = tasklist[i].magic;
		    magic_idx = i;
		    break;
		}
	    }
	}
	if (magic_idx == 0) {
	    T32Printf("EXT.VMC - magic or taskname not found");
	    return;

	}
	spaceid = tasklist[magic_idx].spaceid;
	curpid = tasklist[magic_idx].pid;
    }

    T32SetMemoryAccessSpaceId(spaceid);

    // walk Native  Stack to find VM Interpreter frame
    for (level = 0; level < VM_MAX_STACKDEPTH; level++) {
	rc = T32TaskStackGetCaller(vmc_magicview, level, name, &address);
	if (address == 0)
	    break;
	if (!strcmp(name, VM_PROCNAME))	// name found -> stop
	    break;
    }

    // issue Java.LOADCLASS commands for descriptors 
    pLocalFP = 0;
    rc = T32TaskStackGetPointer(vmc_magicview, level, VM_VN_DVMFPTR, &pLocalFP);
    if (!rc) {
	for (dvmlvl = 0; pLocalFP && (dvmlvl < VM_MAX_DVMDEPTH); dvmlvl++) {
	    addr32_t pClass = 0;
	    addr32_t pClassDesc = 0;
	    addr32_t pClassSrc = 0;
	    addr32_t pDvmDex = 0;
	    addr32_t pHeader = 0;
	    pLocalFP -= siz_DVM_JStackSave;	// rFP points to END of "StackSaveArea"
	    rc = T32CheckMemory(pLocalFP, siz_DVM_JStackSave);
	    if (!!rc) {
		T32Printf("EXT.VMC - StackSaveArea unreadable (lvl=%d).", dvmlvl);
		break;
	    }
	    pFramePrev = T32ReadLong(pLocalFP + ofs_DVM_JStackPrev);
	    pFrameMethod = T32ReadLong(pLocalFP + ofs_DVM_JStackMethod);
	    pFrameCurPC = T32ReadLong(pLocalFP + ofs_DVM_JStackCurPC);

	    rc = T32CheckMemory(pFrameMethod, siz_DVM_Method);
	    if (0 && rc) { // only activate for debugging
		T32DebugPrintf("EXT.VMC - Info: Method Area %s (lvl=%d)",
		               (!pFrameMethod) ? "(n/a)" : "NO_ACCESS", dvmlvl);
	    }
	    if (!pFrameMethod)
	    	rc = 1; // no use to try to continue
	    if (!rc) {
		pClass = T32ReadLong(pFrameMethod + ofs_DVM_MethodClazz);	// FrameMethod->ClassObject
		rc = T32CheckMemory(pClass, siz_DVM_ClassObject);
	    }
	    if (!rc) {
		pClassDesc = T32ReadLong(pClass + ofs_DVM_ClassDesc);	// ClassObject->descriptor
		pDvmDex = T32ReadLong(pClass + ofs_DVM_pDvmDex);	// ClassObject->pDvmDex
		if (!!pDvmDex)
		    pHeader = T32ReadLong(pDvmDex + ofs_DVM_DexHeader);	// DvmDex->pHeader
		if (!!pClass && !!pClassDesc && !!pHeader) {
		    rc = T32CheckMemory(pClassDesc, 256);
		    if (!rc) {
			char buf[256], cmd[296];
			rc = T32ReadBuffer(buf, pClassDesc, 256);
			buf[255] = '\0';
			T32Sprintf(cmd, "Java.LOADCLASS J:0x%4x:0x%8x %s", spaceid, pHeader, buf);
			T32Execute(cmd);
		    }
		}
	    }
		
	    pLocalFP = pFramePrev;
	    if (!pLocalFP)
		break; // end of frame pointer chain
	} // end_for dvmlvl (Java stack frames)
	T32DebugPrintf("EXT.VMC - processed %d DVM stack frames", dvmlvl);
    }

    T32MemoryAccess(MEM_ACCESS_DEFAULT);
}

//eof
