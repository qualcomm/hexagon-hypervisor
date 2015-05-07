/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "global_types.h"	// <-- size4u_t and friends.
#include "t32ext.h"
#include "t32map.h"
#include "ril_defines_v4.h"	// more defines, including global symbols, also a few enums/structs
#include "reg_offsets_v4.h"	// redefine all registers.  Are t hese from ISS? REG_UGP, etc
#include "consts_autogen.h"	// asm_offsets.h
//#include "mydll.h"
//#include "rw_compress.h"
#include "util.h"

//olgak@olgak /cygdrive/z/olgak_P4/main/latest/osam/edk_windows32
//$ ./bin/make qurt_model.t32

/* Defines */
#define QURT_START         0x0
#define ASID_FROM_SSR(x)    ((x>>8) & 0x3f)
// for libc heap macros
#define SIZEOFME(x)  ((char*)(&(x) + 1) - (char*)(&(x)))

/* Prototypes */
#include "util.h"

#define OSAM_MAX_THREADS 160
#define MAX_PDS 8
#define MAX_WAITERS 32
#define MAX_MUTEXES 128
#define SEARCH_DEPTH 16

typedef struct _holder_waiters
{
    ulong holder;
    ulong mutex;
    ulong wait_count;
    ulong pad;
    ulong waiters[MAX_WAITERS];
}Holder_Waiters;

Holder_Waiters res_matrix[MAX_MUTEXES];

ulong deadlock_path[SEARCH_DEPTH];
// trace buffer dump related global variables
typedef struct _index_pcycles
{
    unsigned long cur_index;
    unsigned long original_index;
}Index_Pcycles;

typedef struct _trace_record
{
    unsigned long pcycles;
    unsigned long tag;
    unsigned long w0;
    unsigned long w1;
}Trace_Record;

Index_Pcycles ip[6];
Index_Pcycles ip_tcm[6];
unsigned long trace_buf, trace_ddr_buf, trace_tcm_buf;
unsigned long trace_buf_size, ddr_buffer_size, tcm_buffer_size;
unsigned long tcm_trace_buffer_enabled = 0;
unsigned long max_hthread = 0;
Trace_Record prev_trace[6];
Trace_Record prev_tcm_trace[6];
unsigned long prev_context_pcycles[6];
unsigned long prev_context_pcycles_tcm[6];
ulong max_threads_ddr = 0;
ulong max_threads_tcm = 0;

// POSIX data structure offsets
ulong off_pthread_id;
ulong off_pthread_pend_signals;
ulong off_pthread_wait_signals;
ulong off_pthread_attr;
ulong off_pthread_priority;
ulong off_pthread_name;
ulong off_pthread_staddr;
ulong off_pthread_stsize;
ulong off_pthread_magic;
ulong len_pthread = 0;
ulong tl_flag = 0;

ulong len_thread_name;
ulong off_thread_name;

ulong off_pthread_mq_fd;
ulong off_pthread_mq_attr;
ulong off_pthread_mq_max;
ulong off_pthread_mq_cur;
ulong off_pthread_mq_waits;
ulong off_pthread_mq_thread;
ulong off_pthread_mq_next;

ulong fd_size;
ulong fd_base;
ulong ltcb_ptr;
void displayPtlInfoLine(ulong cur_thr);
unsigned int ct0(unsigned int d);
u32_t QURT_getPhysAddr( u32_t pg);

/************* Demand paging code **************************/
/***TASK.DLP****/
#define IsSetupDLPI() ((par_dl_pool_array == -1) ? 0 : 1)
cmd_display_t*  CmdDLPoolInfoInit (void);
void CmdDLPoolInfoDisplayAll (void);
void InitDLPools(void);
void displayDLPoolInfoLine (ulong index, byte* pi);
void displayDLLoadLogEntryLine (ulong index, byte* entry);
void GetProgModGlobFromPath (char* path, char** program, char** module, char** global);

ulong par_dl_pool_array = -1;          /* address of DL pool array */
ulong len_dl_pool_entry = 0;          /* length of one DL pool entry */
ulong len_dl_pool_array = 0;          /* length of DL pool array */
ulong len_dl_load_log_entry=0;        /* length of DL load log entry */
ulong off_dl_pool_pool = 0;
ulong off_dl_pool_size = 0;
ulong off_dl_pool_list = 0;
ulong off_dl_pool_list_count = 0;
ulong off_dl_pool_list_index = 0;
ulong off_dl_pool_replace_count = 0;
ulong off_dl_load_log_list = 0;
ulong off_dl_load_log_list_count = 0;
ulong off_dl_load_log_list_index = 0;
ulong off_dl_load_log_entry_addr = 0;
ulong off_dl_load_log_entry_evic = 0;
ulong off_dl_load_log_entry_thread = 0;
/******************************************************************/
ulong head_addr = 0;
ulong size_offset = 0;
ulong check_size = 0;
ulong curr_block = 0;
ulong size_block = 0;

void PrintOnePagetable(unsigned int space_addr);
char l1tbl [1024*4];
char l2tbl [1024*4];
/******************************************************************/
//qphys_pool -> mem_pool_t (after 32B chars)
//mem_pool_t -> coalesced
ulong len_pool;
ulong off_pool_coalesced;
ulong len_mem_list;

// typedef struct memory{
//      addr;
//      size;
//      struct {
//          memory_t *tqe_next;
//          memory_t *tqe_prev;
//      }
//  } memory_t;
ulong len_mem;
ulong off_mem_addr;
ulong off_mem_size;
ulong off_mem_tqe_next;

//typedef struct qmem_region {
//    qmem_t                      * virt_mem;
//    unsigned int                magic;          /* This is needed to verify the type of */
//    qmem_pool_t                 * phys_pool;
//    qmem_t                      * phys_mem;
//    u32_t                       size;
//    qmem_mapping_t              mapping_type;
//    qmem_cache_mode_t           cache_mode;
//    qmem_region_type_t          type;
//    qobj_t                      obj;
//    u32_t                       owner_thread;
//} qmem_region_t;
//

ulong len_vma;
ulong len_phys_region;
ulong off_phys_list;
ulong off_phys_mem;
ulong off_region_phys_mem;
ulong off_region_virt_mem;
ulong off_region_size;
ulong off_owner_thread;
ulong off_vma;
ulong off_vma_list;
ulong off_region_type;

//struct space
//
ulong off_proc_state;
ulong off_proc_name;

unsigned int asid_info[MAX_PDS][OSAM_MAX_THREADS];

ulong               GetHWThreadNum(ulong tcbptr);
unsigned long       OS_Initialized(void);
unsigned long       Process_Initialized(void);
ulong               returnMagic    (ulong);
ulong*              GetTranslationList   (void);
ulong               TaskSpaceId    (ulong);

os_obj_handle_t*    GetSpaceId     (void);
os_obj_handle_t*    GetTaskHandle  (void);

T32StackExInitRtn   TaskStackInit  (void);
T32StackExResult*   TaskStackCalc  (ulong, ulong );

ulong               GetTcbPtr      (void);
ulong               GetCurrTask    (void);
arch_context_t*     GetContext     (ulong);

ulong               MagicToSwtid   (ulong);
char*               MagicToName    (ulong);
ulong*              TaskList       (void);

info_cmd_init_t       TlParser     (void);
cmd_display_t*        TlInit       (void);

void                  TlDisplayOne (void);
void                  TlDisplayAll (void);

ulong                 TaskTraceId(ulong);

info_cmd_init_t       SlParser     (void);
cmd_display_t*        SlInit       (void);
void                  SlDisplay    (void);
//void                  rtosInfo     (void);

info_cmd_init_t       PtParser     (void);
cmd_display_t*        PtInit       (void);
void                  PtDisplay (void);

info_cmd_init_t       PtlParser   (void);
cmd_display_t*        PtlInit     (void);
void                  PtlDisplay  (void);

info_cmd_init_t       PmqlParser   (void);
cmd_display_t*        PmqlInit     (void);
void                  PmqlDisplay  (void);

info_cmd_init_t       IrqlParser   (void);
cmd_display_t*        IrqlInit     (void);
void                  IrqL1Display  (void);
void                  IrqL2Display  (void);

info_cmd_init_t       FtxlParser   (void);
cmd_display_t*        FtxlInit     (void);
void                  FtxlDisplay  (void);

info_cmd_init_t       DbgParser   (void);
cmd_display_t*        DbgInit     (void);
void                  DbgDisplay  (void);

info_cmd_init_t       MemParser   (void);
cmd_display_t*        MemInit     (void);
void                  MemDisplay  (void);

info_cmd_init_t       TraceParser   (void);
cmd_display_t*        TraceInit     (void);
void                  TraceDisplay  (void);

info_cmd_init_t       TraceParserRaw   (void);
cmd_display_t*        TraceInitRaw     (void);
void                  TraceDisplayRaw  (void);

/***************** Demand paging code ************/
info_cmd_init_t       CmdDLPoolInfoParser   (void);
cmd_display_t*        CmdDLPoolInfInit     (void);
void                  CmdDLPoolInfoDisplay  (void);

info_cmd_init_t     CmdDLLoadLogParser     (void);
cmd_display_t*      CmdDLLoadLogInit       (void);
void                CmdDLLoadLogDisplayLog (void);
/*************************************************/

info_cmd_init_t       TestParser   (void);
cmd_display_t*        TestInit     (void);
void                  TestDisplay  (void);

/*********** config bitmask *************/
info_cmd_init_t       ConfigParser (void);
cmd_display_t*        ConfigInit   (void);
void                  ConfigDisplay(void);
/***************************************/

/*********** kernel heap **************/
info_cmd_init_t       AppHeapParser (void);
cmd_display_t*        AppHeapInit   (void);
void                  AppHeapDisplay(void);
char *                NameFromAsid (unsigned int);
/*************************************/

/********** stack overflow ************/
info_cmd_init_t      CmdStackOvfParser (void);
cmd_display_t*       StackOvfInit      (void);
void                 StackOvfDisplay   (void);
void                detect_stack_ovf  (ulong);

/********* stack ***********************/
info_cmd_init_t      CmdGStackParser (void);
cmd_display_t*       GStackInit      (void);
void                 GStackDisplay   (void);
/***************************************/

/*********** page table display ********/
info_cmd_init_t      PgtParser  (void);
cmd_display_t*       PgtInit    (void);
void                 PgtDisplay (void);
void                 PgtDisplayOne (void);
/***************************************/

/********** process display ************/
info_cmd_init_t      CmdProcessParser (void);
cmd_display_t*       ProcessInit   (void);
void                 ProcessDisplay(void);
void             displayTCB(void);
/***************************************/

/********** task.option ***************/
info_cmd_init_t       CmdOption (void);
cmd_display_t*        OptionInit (void);
void                  OptionDisplay(void);
/*************************************/

/**********task.resetQ6Z*************/
info_cmd_init_t      CmdResetQ6Z (void);
cmd_display_t*       ResetQ6ZInit (void);
void                 ResetQ6ZDisplay(void);
/************************************/

// Utility functions
ulong   ThreadNo        (ulong );
int isRunning (size4u_t tcb_ptr);
int isWaiting (size4u_t tcb_ptr);

/* Globals */

long par_magic      = 0;
long par_tcblist    = 0;
long par_slist      = 0;
long phy_start_addr = 0;
ulong par_configs = 0;
long par_kernel_heap =0;
long par_app_heap = 0;
long par_app_heap_size = 0;
ulong par_image_vend = 0;
ulong par_image_vstart = 0;
ulong par_heap_base = 0;
ulong par_kheap_size = 0;
ulong par_pt_table = 0;
ulong par_asid_list = 0;
ulong context_size;
ulong island_mode;

int swtmap[6];

ulong argvalue[2];
byte argbuffer[200];

/********   q6zip-related   *********/
sint32 extMemoryAccess (T32AddressHandle, T32BufferHandle, uint32);
void fixup_vararg(char *format, ...);
static byte load_flag[4096]={0};
static byte load_flag_rx[4096]={0};
static unsigned int recompress_cnt[4096]={0};
static byte RWBuffer[4096]={0};
static byte q6zip_on =0;
static byte q6zip_rw=0;
static byte q6zip_path[200] = {0};

ulong rw_decompressed_va_start =0;
ulong rw_decompressed_va_end = 0;
ulong text_decompressed_va_start = 0;
ulong text_decompressed_va_end =0;
ulong rw_info;
ulong rw_stats;
ulong rw_stats_one_size;
ulong len_rw_info, len_rw_table;
ulong off_info_table;
ulong off_obj_size, off_obj_ptr;
ulong len_rw_stats, off_recompress_cnt;
ulong rx_metadata;
ulong len_rx_metadata;
ulong off_dictionary=0;
ulong off_n_blocks=0;
ulong off_compressed_va=0;

ulong debug_state = SYSTEM_DOWN;
/*****************************************/

/* Initialization of Extension */
void main (void)
{
    static info_command_t tasklist  =
    {   "TASKLIST", "TL",   "TaskList",
        "__EM_TL: List of all TCBs",
        TlParser
    };
    // Definition for new command "TASK.SpaceList"
    static info_command_t spacelist =
    {   "SPLIST",  "SL",   "SpaceList",
        "__EM_SL: List of all Address Spaces",
        SlParser
    };

    // Definition for new command "TASK.PthreadList"
    static info_command_t pthreadlist =
    {   "PTLIST",  "PTL",   "PthreadList",
        "__EM_PTL: List of all posix threads",
        PtlParser
    };

    // Definition for new command "TASK.PmsgqList"
    static info_command_t pmsgqlist =
    {   "PMQLIST",  "PMQL",   "PmsgqList",
        "__EM_PMQL: List of all posix msgqs",
        PmqlParser
    };

    // Definition for new command "TASK.PmsgqList"
    static info_command_t irqlist =
    {   "IRQLIST",  "IRQL",   "IrqList",
        "__EM_IRQL: List of all interrupts",
        IrqlParser
    };

    // Definition for new command "TASK.FutexList"
    static info_command_t ftxlist =
    {   "FTXLIST",  "FTXL",   "FutexList",
        "__EM_FTXL: List of all futexes",
        FtxlParser
    };

    // Definition for new command "TASK.DbgBuf"
    static info_command_t dbgbuf =
    {   "DBGBUF",  "DBG",   "DebugBuf",
        "__EM_DBG: debug buffer dump",
        DbgParser
    };

    // Definition for new command "TASK.MemList"
    static info_command_t memlist =
    {   "MEMLIST",  "ML",   "MemList",
        "__EM_MEM: List free physical memory",
        MemParser
    };

    // Definition for new command "TASK.TraceList"
    static info_command_t tracelist =
    {   "TRACELIST",  "TRACE",   "TraceList",
        "__EM_TRACE: Kernel Traces",
        TraceParser
    };

    // Definition for new command "TASK.TraceListRaw"
    static info_command_t tracelistraw =
    {   "TRACELISTRAW",  "TRACER",   "TraceListRaw",
        "__EM_TRACER: Kernel Raw Traces",
        TraceParserRaw
    };

    // Definition for new command "TASK.Test"
    static info_command_t testlist =
    {   "TEST",  "TEST",   "test",
        "__EM_TEST: test",
        TestParser
    };

    // Definitions for new command "TASK.ConfigList"
    static info_command_t configbtmask =
    {
        "CONFIGLIST", "CONFIGS", "ConfigList",
            "__EM_CONFIGS: ConfigList",
            ConfigParser
    };

    static info_command_t kernelheaplist =
    {
        "APPHEAP", "HEAP", "AppHeap",
           "__EM_HEAP: AppHeap",
            AppHeapParser
   };

   static info_command_t stackovflist =
   {
      "STACKOVF", "STACKOVF", "StackOvf",
         "__EM_STACKOVF: StackOvf",
         CmdStackOvfParser
   };

   static info_command_t gstacklist =
   {
      "GSTACK", "GSTACK", "GStack",
          "__EM_GSTACK: GStack",
          CmdGStackParser
   };

   static info_command_t pgtlist =
   {
        "PAGETABLE", "PGT", "PageTable",
           "__EM_PGT: PageTable",
            PgtParser
   };

   static info_command_t processinfo =
   {    "SYSINFO", "SYSINFO", "SysInfo",
       "__EM_SYSINFO",
       CmdProcessParser
   };

    /********************* Demand paging code ***************/

    //definition for new command "TASK.DLP"
    static info_command_t dlpoolinfo =
    {   "DLPOOLINFO", "DLP", "DlPoolInfo",
        "__EM_DL_PI",
        CmdDLPoolInfoParser
    };

    static info_command_t dlloadlog = // Definition for new command "TASK.DLLL"
    {   "DLLOADLOG",  "DLLL", "DlLoadLog",
        "__EM_DL_LL",
        CmdDLLoadLogParser
    };

    /*********************************************************/

    /*************************Option for q6zip*****************/
    //definition for new command "TASK.OPT"
    static info_command_t getoption =
    {   "OPT", "OPTION", "option",
        "__EM_OPTION",
        CmdOption
    };

    static info_command_t resetq6z =
    {   "RQZ", "RQZ", "resetqz",
        "__EM_RQZ",
        CmdResetQ6Z
    };
    /*********************************************************/

    //static const char stackdef_pattern = 0x00;

    //static T32StackExDef stackdef =
    //{
    //    TaskStackInit,
    //    1,
    //    &stackdef_pattern,
    //    0
    //};

    /* Get Magic Number or TCB head */
    T32DefineGetMagic      (GetTaskHandle);
    T32DefineGetSpaceId    (GetSpaceId);
   // T32DefineTaskStackEx   (&stackdef);

    DregisterTogetCurrTask(GetCurrTask);

    /* Define Task Id and Task Name Translation */
    DregisterTogetSwtid     (MagicToSwtid);
    DregisterTogetDebugName (MagicToName);

    /* Define Task List & Stack Routine */
    DregisterTogetTaskList       (TaskList);
    DregisterTogetASID           (TaskSpaceId);
    DregisterTogetTranslationList(GetTranslationList);
    DregisterTogetContext        (GetContext);

    /* Define Trace Id */
    DregisterTogetTraceId       (TaskTraceId);

    /* Define command extensions */
    DregisterCommand (&tasklist);
    DregisterCommand (&spacelist);
    DregisterCommand (&pthreadlist);
    DregisterCommand (&pmsgqlist);
    DregisterCommand (&irqlist);
    DregisterCommand (&ftxlist);
    DregisterCommand (&dbgbuf);
    DregisterCommand (&memlist);
    DregisterCommand (&tracelist);
    DregisterCommand (&tracelistraw);
    DregisterCommand (&dlpoolinfo);
    DregisterCommand (&dlloadlog);
    DregisterCommand (&testlist);
    DregisterCommand (&configbtmask);
    DregisterCommand (&kernelheaplist);
    DregisterCommand (&pgtlist);
    DregisterCommand (&stackovflist);
    DregisterCommand (&gstacklist);
    DregisterCommand (&processinfo);
    DregisterCommand (&getoption);
    DregisterCommand (&resetq6z);

   /* Define memory access behavior */
   T32DefineExtMemoryAccess(extMemoryAccess);
}

/**********************  Parameter Setup  **********************/

unsigned long OS_Initialized(void)
{
    ulong init_flag=0;
    ulong max_threads=0;
    par_tcblist = DgetSymbolAddress(TCB_LIST);
    par_slist = DgetSymbolAddress (SPACE_LIST);
    par_magic = par_tcblist;
    //DEBUG_MSG("par_tcblist %x\n", par_tcblist);
    //T32SetDefaultMagics(1, par_magic);
    par_configs = DgetSymbolAddress (CONFIG_BITMASK);
    par_kernel_heap = DgetSymbolAddress (KERNEL_HEAP_USED);
    par_image_vend = DgetSymbolAddress(IMAGE_VEND);
    par_image_vstart = DgetSymbolAddress(IMAGE_VSTART);
    par_heap_base = DgetSymbolAddress(HEAP_BASE);
    par_kheap_size = DgetSymbolAddress(KHEAP_SIZE);
    par_pt_table = DgetSymbolAddress(PTHREAD_TABLE);

    context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));
    max_threads = DreadLong(DgetSymbolAddress(MAX_THREADS));
    max_threads_tcm = DreadLong(DgetSymbolAddress(MAX_THREADS_TCM));  // threads in TCM
    max_threads_ddr = max_threads - max_threads_tcm;  // threads in DDR
    island_mode = DreadLong(DgetSymbolAddress(ISLAND_MODE));
    if (island_mode){
        return 1;
    }
    if ((par_slist!= (0xFFFFFFFF)) && (par_tcblist != (0xFFFFFFFF)))
    {
        if (par_tcblist != 0) //&& (DreadLong(par_tcblist) !=0))
        {
            init_flag = DgetSymbolAddress(QURT_INIT);
            if (init_flag != 0xFFFFFFFF)
                if ( (init_flag = DreadLong(init_flag)) == 0x01){
                    return 1;
                }
        }
        return 0;
    }
    DISP_MSG("Kernel symbol file not provided .... exiting\n");
    return 2;
}

unsigned int xor_digest(unsigned int ram_addr, unsigned int vm_addr, unsigned int size)
{
    unsigned int res = 0;
    int i=0;
    int max = size >> 2; //size in words
    ulong word1, word2;

    //x xor y = (x & ~y) | (~x & y);
    for (i = 0; i < max; i ++)
    {
        word1=DreadLong(ram_addr);
        word2=DreadLong(vm_addr);
        if ((word1 & ~word2) | (~word1 & word2))
        {
            res = 1; break;
        }
        ram_addr+=4;
        vm_addr+=4;
    }
    return res;
}

void fixup_vararg(char *format, ...)
{
    //fixup for possible compiler bug???
    return;
}

/************************ setting up memory access functionality *****************************/

/** Function to load the buffer for a requested address and size
*  step 1: save off data into compressed.bin
*  step 2: call decompressor on the idx of the page, as a result decompressedx.bin is created
*  step 3: load decompressed file into VM
*  step 4: load T32 buf with contents of VM
*  notes: decompression relies on load_flag, i.e. per each page decompressed, load_flag[page]=1
*  this prevents continous load of decompressed unchanged pages; also if page is recompressed
*  the decompression process starts anew; a current and new recompress_cnt is maintained for
*  that purpose;
*  decompressors should be copied to relevant location if "copy" workaround is used
*
**/

uint32 LoadRWBuffer_rw( uint32 address, uint32 size)
{
    uint32 idx = 0; //load flag idx and compressed file idx
    uint32 rw_idx, rw_start = 0; //index into rw table pointing to compressed objects
    uint32 compressed_data_addr;
    unsigned int decompressed_data[256] ;
    unsigned int compressed_size;
    uint32 res = 0;
    char filename_compressed[13] = "compressedrw";
    char filename_decompressed[15] = "decompressedrw";
    char filename_bin[5]=".bin";
    char temp_dir[9]="C:\\temp\\";
    //char temp_dir[9]="~~~/";
    unsigned int file_idx;
    ulong range_start=0, range_end=0;
    unsigned int cur_recompress_cnt = 0;

    /* calculate index into load_flag array based on address, already range-checked */
    idx = (uint32)(address - rw_decompressed_va_start) >> FOURK_SHIFT; //in 4K pages
    //DEBUG_MSG("idx %x rw decompr va %x\n", idx, rw_decompressed_va_start);
    T32CopyFromGlobal((byte*)(&load_flag[idx]), sizeof(byte));
    //T32DebugPrintf("load flag %x\n", load_flag[idx]);

    range_start = idx * SIZE_4K + Q6ZIP_VM_RW_OFFSET;
    range_end = range_start + SIZE_4K -1;
    //T32DebugPrintf("range start %x range end %x\n", range_start, range_end);

     /** check following to decide whether to re-decompress and reload:
     *   recompress count: rw_stats[block_num].recompress_count, same idx as dlpager_rw_info.table
     *          object has been recompressed and needs reload
     **/
    T32CopyFromGlobal((uint8*)(&recompress_cnt[idx]), sizeof(unsigned int));
    if (DreadLong(rw_stats))
        cur_recompress_cnt = DreadLong(DreadLong(rw_stats)+ idx*rw_stats_one_size + off_recompress_cnt);

    //T32DebugPrintf("recompress cnt: %d address rw stat %x address cnt %x idx %d, size of stat rec %x\n",
                    //cur_recompress_cnt,DreadLong(rw_stats),DreadLong(rw_stats)+ idx*rw_stats_one_size + off_recompress_cnt,
                    //idx, rw_stats_one_size);

    if((load_flag[idx] == 0)||(cur_recompress_cnt != recompress_cnt[idx])) //flag needs to reset on write
    {
        /* get pointer of compressed object */
        rw_start = (uint32)(rw_decompressed_va_start - text_decompressed_va_start) >> FOURK_SHIFT;
        rw_idx = (uint32)( ((address - text_decompressed_va_start)>>FOURK_SHIFT)  - rw_start);
        //rw_idx = idx;
        compressed_data_addr = ( (DreadLong(rw_info + off_info_table))+ 8*rw_idx);
        if ((compressed_data_addr == 0) || (!DreadLong(compressed_data_addr)))
        {
            DEBUG_MSG("No compressed data for this index %d address 0x%x\n", rw_idx, address);
            //memset((void *)RWBuffer, 0x0, size);
            return 1;
        }
        compressed_size = DreadLong(compressed_data_addr + off_obj_size)- 1;
        //digest = xor_digest(compressed_data_addr, range_start, compressed_size);

        /* save object as a compressed.bin file */
        file_idx = idx;
        DEBUG_MSG("compressed data %x rw_idx %x, decompressed_data %x obj_size %x rw_start %x\n",
                        compressed_data_addr, rw_idx, decompressed_data, compressed_size,  rw_start);
        T32Execute("Data.SAVE.Binary %s%s%d%s %x++%x", temp_dir, filename_compressed, file_idx, filename_bin,
                                                       DreadLong(compressed_data_addr), compressed_size);
        DEBUG_MSG("saving %s%s%d%s\n", temp_dir, filename_compressed, file_idx, filename_bin);
        #if 0
        //this is a lib.a solution, put on hold for now
        //unsigned int deltaUncompress(unsigned int *compressed,unsigned int *uncompressed,unsigned int out_len);
        res = OSAMdeltaUncompress((unsigned int *)compressed_data_addr, decompressed_data, compressed_size, 1); //size in words
        T32DebugPrintf("decompressed word 0 %x\n", decompressed_data[0]);
        T32DebugPrintf("res %d\n", res);
        memcpy((void *)buffer, (const void *)decompressed_data, size);
        #endif

        /* decompress into a file with python */
        //T32Execute("cd C:\\T32\\temp\\q6zip");
        //T32Execute("cd %s", q6zip_path);
        //DEBUG_MSG("compressor path: %s+++++++++\n",q6zip_path);
        T32Execute("cd C:\\temp");
        T32Execute("OS.area python rw_decompress_file.py %d", file_idx); //decompress one file

        /* load the file into VM into a certain range*/
        DEBUG_MSG("range start %x range end %x\n", range_start, range_end);
        T32Execute ("Data.LOAD.Binary %s%s%d%s vm:0x%x--0x%x /NOCLEAR", temp_dir, filename_decompressed, file_idx,
                                                                        filename_bin, range_start, range_end);

        load_flag[idx]=1;
        T32CopyToGlobal((byte*)(&load_flag[idx]), sizeof(byte));
        DEBUG_MSG("load_flag_rw[%d] %d \n", idx, load_flag[idx]);

        recompress_cnt[idx]=cur_recompress_cnt;
        T32CopyToGlobal((uint8*)(&recompress_cnt[idx]), sizeof(unsigned int));
    }
    /* read load_flag array to see if the file for this address is loaded */
    T32SetMemoryAccessClassString ("VM");
    //read within the 4K page
    if (!T32CheckMemory(range_start,4))
        T32ReadBuffer(RWBuffer, (range_start + (address&0xFFF)), size);
    else{
        //FIXME: probably need to indicate error
        //load_flag[idx]=0;
        //T32CopyToGlobal((byte*)(&load_flag[idx]), sizeof(byte));
    }
    T32SetMemoryAccessClassString ("MEM_ACCESS_DEFAULT");

    return 0;
}

uint32 LoadRWBuffer_text(uint32 address, uint32 size)
{
    char filename_ext[5]=".bin";
    //char temp_dir[9]="C:\\temp\\"; //use "~~~/" instead
    char temp_dir[9]="~~~/";
    char filename_compressed[13] = "compressedro";
    char filename_decompressed[15]="decompressedro";
    char dictionary[9]="dict.bin";

    ulong compressed_data_addr, compressed_data_size, compressed_data_addr_next;
    uint32 idx, range_start, range_end, rx_num_blocks;

    /* check to see if a page is already loaded, get idx into load array */
    idx = (uint32)(address - text_decompressed_va_start)>>FOURK_SHIFT;
    T32CopyFromGlobal((byte*)(&load_flag_rx[idx]), sizeof(byte));
    //DEBUG_MSG("load flag ro %x idx %d\n",load_flag_rx[idx], idx);
    /* calculate range into vm where the read will happen after all is done */

    range_start = idx*SIZE_4K;
    range_end = range_start + SIZE_4K -1;

    /* if page not loaded ... do decompression and load */

    if(load_flag_rx[idx]==0)
    {
        /* load a page */
        rx_num_blocks = DreadLong(DreadLong(rx_metadata) + off_n_blocks);
        //compressed_data_addr = compressed_va[block_num]
        compressed_data_addr = DreadLong(rx_metadata)+off_compressed_va+idx*4;
        compressed_data_addr_next = DreadLong(rx_metadata) + off_compressed_va+(idx+1)*4;
        if ((idx+1) == rx_num_blocks)
        {
            compressed_data_size = text_decompressed_va_end - DreadLong(compressed_data_addr) -1;
        }else{
            compressed_data_size = DreadLong(compressed_data_addr_next) - DreadLong(compressed_data_addr);
        }
        DEBUG_MSG(" off_compressed_va %x compressed_text adddr %x for addr %x, size %x\n",
                    off_compressed_va, compressed_data_addr, address, compressed_data_size);

        /* store into a bin file */
        T32Execute("Data.SAVE.binary %s%s%d%s 0x%x++0x%x\n", temp_dir, filename_compressed, idx, filename_ext,
                                                             DreadLong(compressed_data_addr),compressed_data_size-1);
        //T32Execute("cd z:\\olgak_P4\\main\\latest\\osam\\edk_windows32\\q6zip");
        T32Execute("cd C:\\temp");
        //DEBUG_MSG("compressor path: %s+++++++++\n",q6zip_path);
        //T32Execute("cd %s", q6zip_path);
        T32Execute("OS.area python q6zip_ro_uncompress.py C:\\temp\\%s%d.bin C:\\temp\\dict.bin C:\\temp\\%s%d.bin",
        filename_compressed,idx, filename_decompressed,idx);

        T32Execute ("Data.LOAD.Binary %s%s%d%s vm:0x%x--0x%x /NOCLEAR /NOSYMBOL", temp_dir, filename_decompressed, idx,
                                                                        filename_ext, range_start, range_end);

        load_flag_rx[idx]=1;
        T32CopyToGlobal((byte*)(&load_flag_rx[idx]), sizeof(byte));
        T32Printf("load_flag_rx[%d] %d\n", idx, load_flag_rx[idx]);
    }
    //DEBUG_MSG("in ro, reading Ro\n");
    T32SetMemoryAccessClassString ("VM");
    if (!T32CheckMemory(range_start,4))
    {
        T32ReadBuffer(RWBuffer, (range_start + (address&0xFFF)), size);
    }
    //else
    {
     //   load_flag_rx[idx]=0;
     //   T32CopyToGlobal((byte*)(&load_flag_rx[idx]), sizeof(byte));
        //T32CopyFromGlobal ((byte *)&q6zip_on, sizeof(byte));
        //q6zip_on=0;
        //T32CopyToGlobal((byte *)&q6zip_on, sizeof(byte));
    }
    //T32DebugPrintf("returing from rx read q6zip_on++ %d\n", q6zip_on++);
    T32SetMemoryAccessClassString ("MEM_ACCESS_DEFAULT");

    return 0;
}

//this ought to be called on first access to RO data, not earlier,
//on time of access to the RO data, task.option will have the path, otherwise path will be null;
void q6zip_InitRO(void)
{
    char filename_bin[5]=".bin";
    char temp_dir[9]="C:\\temp\\";
    char filename_dict[5]="dict";
    ulong dict_addr, dict_size;

    T32CopyFromGlobal((uint8*)q6zip_path, sizeof(byte)*199);
    if(q6zip_path[0] == 0)
    {
        DEBUG_MSG("q6zip path not set! \n");
        return;
    }
    T32CopyFromGlobal ((byte *)&q6zip_on, sizeof(byte));
    if (q6zip_on==0)
    {
        //clean off temp dir
        //T32Execute("rm cm*, com*, dec*, di*");
        //FIXME: makesure rx_metatdata non 0
        dict_addr = DreadLong(rx_metadata) + off_dictionary;
        DEBUG_MSG ("dict addr %x\n", dict_addr);
        dict_size = Q6ZIP_DICT_SIZE*4-1; //in bytes

        /* this copy should be within the if */
        T32Execute("copy %s\\q6zip_ro_decompressor C:\\temp\\q6zip_ro_decompressor", q6zip_path);
        T32Execute("copy %s\\q6zip_ro_decompressor.exe C:\\temp\\q6zip_ro_decompressor.exe", q6zip_path);
        T32Execute("copy %s\\q6zip_ro_uncompress.py C:\\temp\\q6zip_ro_uncompress.py", q6zip_path);
        if((dict_addr)&&(DreadLong(rx_metadata)))
        {
            q6zip_on=1;
            T32CopyToGlobal((byte *)&q6zip_on, sizeof(byte));
            /* save dictionary into a .bin, do it only once */
            T32Execute("Data.Save.Binary %sdict.bin 0x%x++0x%x", temp_dir, dict_addr, dict_size);
            DEBUG_MSG("setting q6zip on %d\n", q6zip_on);
        }
        T32Execute("cd %s", q6zip_path);

    }//end if q6zip_on
}

void q6zip_InitRW(void)
{

    T32CopyFromGlobal((uint8*)q6zip_path, sizeof(byte)*199);
    if (q6zip_path[0] == 0)
    {
        DEBUG_MSG("q6zip path not set!\n");
        return;
    }
    T32CopyFromGlobal ((byte *)&q6zip_rw, sizeof(byte));
    if (q6zip_rw==0)
    {
        T32Execute("cd %s", q6zip_path);
        T32Execute("copy %s\\rw_decompress_file.py C:\\temp\\rw_decompress_file.py", q6zip_path);
        T32Execute("copy %s\\rw_py_compress.py C:\\temp\\rw_py_compress.py", q6zip_path);
        q6zip_rw=1;
        T32CopyToGlobal((byte *)&q6zip_rw, sizeof(byte));
    }
}

sint32 extMemoryAccess (T32AddressHandle ahdl, T32BufferHandle bhdl, uint32 mode)
{
     //uint32 address = T32GetAHAddress32  (ahdl);
     uint32 request_address = T32GetAHAddress32  (ahdl);
     uint32 spaceid = T32GetAHSpaceId    (ahdl);
     uint32 size    = T32GetBHSize       (bhdl);
     uint32 r;

     if (size > 4096)
     {
         size = 4096;
         T32SetBHSize (bhdl, 4096);
     }
     //T32DebugPrintf("extMemoryAccess invoked! req address 0x%x\n", request_address);

     /* based on whether the feature is enabled or not, do all one-time processing here */
     T32CopyFromGlobal ((byte *)&q6zip_on, sizeof(byte));
     if (DgetSymbolAddress(START_VA_UNCOMPRESSED_RW))
     {
         rw_decompressed_va_start = DreadLong(DgetSymbolAddress(START_VA_UNCOMPRESSED_RW));
         rw_decompressed_va_end = DreadLong(DgetSymbolAddress(END_VA_UNCOMRESSED_RW));

         /* get RW symbols */
         //maybe make a separate function to read all the necessary symbols
         //dlpager_rw_info.table->ptr and size
         len_rw_info = T32SymbolTypeLink("dlpager_rw_info_t");
         off_info_table = T32SymbolTypeOffsetGet(".table");

         len_rw_table = T32SymbolTypeLink("dlpager_rw_table_t");
         off_obj_ptr = T32SymbolTypeOffsetGet(".ptr");
         off_obj_size = T32SymbolTypeOffsetGet(".size");

         len_rw_stats = T32SymbolTypeLink("rw_stats_t");
         off_recompress_cnt = T32SymbolTypeOffsetGet(".recompress_count");
         rw_stats_one_size = T32SymbolTypeSizeGet("rw_stats_t");

         rw_info = DgetSymbolAddress(RW_INFO);
         rw_stats = DgetSymbolAddress(RW_STATS);
         //add extra checks, since loading scripts may access unmapped memory before dlpager is setup
         //this shall
         if ((request_address >= rw_decompressed_va_start) && (request_address < rw_decompressed_va_end))
             q6zip_InitRW();
     }//if rw enabled

     if (DgetSymbolAddress(START_VA_UNCOMPRESSED_TEXT))
     {
         /* get RO symbols */
         text_decompressed_va_start = DreadLong(DgetSymbolAddress(START_VA_UNCOMPRESSED_TEXT));
         text_decompressed_va_end = DreadLong(DgetSymbolAddress(END_VA_UNCOMPRESSED_TEXT));
         rx_metadata = DgetSymbolAddress(RX_METADATA);
         len_rx_metadata = T32SymbolTypeLink("dlpager_rx_metadata_t");
         off_dictionary = T32SymbolTypeOffsetGet(".dictionary");
         off_n_blocks = T32SymbolTypeOffsetGet(".n_blocks");
         off_compressed_va = T32SymbolTypeOffsetGet(".compressed_va");
         //add extra chcecks, since loading scripts may access unmapped memory before dlpager is setup
         if ((request_address >= text_decompressed_va_start) && (request_address < text_decompressed_va_end))
             q6zip_InitRO();
     }//if ro enabled

     if (mode == T32EXTMEMORYACCESS_READ)
     {
         /* fill buffer from somewhere, e.g. */
         //DEBUG_MSG("something soooo wroooong\n");
         //fixup_vararg("something is boken here...%d %x %x \n", size,  rw_decompressed_va_start,  rw_decompressed_va_end);
         //DEBUG_MSG("load buffer %x req addr %x size %d, q6zip_rw %d, 0x%x -- 0x%x\n", RWBuffer, request_address, size, q6zip_rw,
         //               rw_decompressed_va_start, rw_decompressed_va_end);

         //DEBUG_MSG("size %d",size);
         if((q6zip_rw) && (request_address >= rw_decompressed_va_start) && (request_address < rw_decompressed_va_end))
         {
            r = LoadRWBuffer_rw(request_address, size);
         }
         else if ((q6zip_on) && (request_address >= text_decompressed_va_start) && (request_address < text_decompressed_va_end))
         {
            r = LoadRWBuffer_text(request_address, size);
         }
         else
         {
            //maybe reset everything on error
            DEBUG_MSG("address 0x%x rw range 0x%x--0x%x rx range 0x%x--0x%x q6zip_rw %d\n", request_address, rw_decompressed_va_start,
                                               rw_decompressed_va_end, text_decompressed_va_start,text_decompressed_va_end,
                                               q6zip_rw);

            DEBUG_MSG("address not within compressed segments ...\n");
            return T32EXTMEMORYACCESS_ERRMODE;
         }
         // transfer buffer to TRACE32
         T32SetBHContent (bhdl, 0, RWBuffer, size);    // handle, offset into bh, buffer, size
         return T32EXTMEMORYACCESS_OK;
     }
     else if (mode == T32EXTMEMORYACCESS_WRITE)
     {
         // transfer buffer from TRACE32
         //T32GetBHContent (bhdl, 0, RWBuffer, size);

         //write contents to a shared structure only, not to vm or any other stuff
         // write buffer to somewhere, e.g.
         //T32WriteBuffer (RWBuffer, request_address+0x1000, size);

         /* temporarily generate an error */
         T32Warning("Writes to compressed sections not supported in this osam version..\n");
         return T32EXTMEMORYACCESS_OK;
     }

     return T32EXTMEMORYACCESS_ERRMODE;  // unknown mode
}

/*************** initialization of processes ****************/
unsigned long Process_Initialized(void)
{
   //check name: each process should have name set
   //check OS_Initialized first:
   size4u_t current_asid;
   unsigned int i = argvalue[0];
   //if (OS_Initialized != 1)
   //{
   //   DISP_MSG("Kernel not initialized.... \n");
   //   return 0;
   //}
   //par_asid_list = DreadLong();
   if( (current_asid = (size4u_t)DgetSymbolAddress(PROCESS_TABLE)) == (size4u_t)-1)
   {
        //T32Printf("error in current space!\n");
        return 0;
   }
   //go through each process in list to see if all initialized
   //maintain a table of initted pocesses?

   current_asid = DreadLong(current_asid + 4*i);
   T32SymbolTypeLink("struct space");
   off_proc_state = T32SymbolTypeOffsetGet(".state");
   //T32Printf ("reading name of process: %s\n", MagicToName(DreadLong(current_asid + off_proc_name)));

   T32DebugPrintf ("cur asid: %x off %x  prcess state: %x\n", current_asid, off_proc_state, (DreadLong(current_asid + off_proc_state)));
   if ((DreadLong(current_asid + off_proc_state) != 2) &&
       (DreadLong(current_asid + off_proc_state) != 1)    )
       return 0;

   return 1;//initialized
}

/***********************************************************/

/************************* subroutines **********************/

//!FUNCTION: GetTaskHandle-------------------------
//! Get TCB list head of Current Task
//!------------------------------------------------
os_obj_handle_t* GetTaskHandle(void)
{
    static os_obj_handle_t magic = {0, 0, 0, 0};
    magic.address = GetTcbPtr();
    magic.size    = 4;
    //DEBUG_MSG("Current Magic: %x\n", magic.address);
    return &magic;
}

unsigned long GetTcbPtr(void)
{
    int tnum;
    unsigned long cur_tcbptr_addr = DgetSymbolAddress (IDLE_THREAD);
    if(OS_Initialized() == 1)
    {
      tnum = DgetCurrHwThread();
      cur_tcbptr_addr = returnMagic(tnum);
      if(cur_tcbptr_addr == ((ulong)-1))
        cur_tcbptr_addr = par_magic;
    }
    //DEBUG_MSG("Current tcbptr Address: %x\n", cur_tcbptr_addr);
    return cur_tcbptr_addr;

}

ulong GetCurrTask()
{
    ulong tcb_ptr = GetTcbPtr();
    return tcb_ptr;
}

/**********************  Magic to Id Translation  **********************/
/* Magic to Id Translation: for magic display on various task windows */

ulong MagicToSwtid (ulong tcbptr)
{
    return tcbptr;
}

/**********************  Magic to Name Translation  **********************/
char* MagicToName (ulong tcbptr)
{
    /* 2 options:
     * 1) constant offset, performs better but needs to be updated with changes in utcb
     * 2) reading offset dynamically, slower
     * keep both, one on comments in case need arises for performance
     */
    static char  name[32]="N/A";
    static char  isr_name[32]="ISR";
    ulong utcb = 0;
    unsigned int off_thread_attr, len_thread_attr;

    if (tcbptr == 0)
        return (name);

    utcb = DreadLong(tcbptr + CONTEXT_ugpgp + 4);
    if( utcb == 0 ) {     // in ISR
       return (isr_name);
    }
    /* option 1 */
    //DreadBuffer(name, utcb + UTCB_thread_name0, 16);

    /* option 2 */
    //get thread name0 offset:
    len_thread_name = T32SymbolTypeLink("QURT_utcb_t");
    off_thread_attr = T32SymbolTypeOffsetGet(".attr");

    len_thread_attr = T32SymbolTypeLink("struct _qurt_thread_attr");
    off_thread_name = T32SymbolTypeOffsetGet(".name");

    DreadBuffer (name, utcb + off_thread_attr + off_thread_name, 16);
    name[16]=0;
    return (name);
}

char*  NameFromAsid (unsigned int asid)
{
   char *name;
   char elf_name[64] = "N/A";
   char *last;
   ulong process, proc_ptr;

   proc_ptr = DgetSymbolAddress(PROCESS_TABLE);
   process = DreadLong(proc_ptr + 4*asid);

   T32SymbolTypeLink("struct space");
   off_proc_name = T32SymbolTypeOffsetGet(".name");

   //T32DebugPrintf("process %x, off_proc_name %x\n", process, off_proc_name);
   DreadBuffer(elf_name, process + off_proc_name, 32);
   //T32Printf ("elf namme : %s\n", elf_name);
   if(elf_name[0] == 0)
   {
      //T32Printf ("elf name empty string\n");
       name = "";
       return name;
   }

   //T32DebugPrintf("elf_name:  %s\n", elf_name);
   //truncate ".pbn"
   name = (char *)strtok_r(elf_name, ".", &last );
   //T32DebugPrintf("asid name: %s, elf_name %s\n", name, elf_name);

   return name;
}

/**********************  Translation List Routine *****************/
// To debug this call in T32 insert DEBUG_MSG and trigger using mmu.scanid

ulong* GetTranslationList (void)
{
    static ulong addressspaces[MAX_PDS * 2]; // 40 PDs allowed
    ulong spaceptr, tmp;
    int i = 0;
    if (OS_Initialized () != 1)
        return 0;
    spaceptr = DgetSymbolAddress(SPACE_LIST);
    do
    {
        tmp = DreadLong(spaceptr + i*2);    // spaceId
            if (tmp != 0)
            {
            addressspaces[i]   = i;
            addressspaces[i+1] = tmp;     //MMU root pointer
            }
            DEBUG_MSG("ASID: %x PDIR: %x", addressspaces[i], addressspaces[i+1]);
            i = i+2;
    }
    while (i < MAX_PDS*2);
    addressspaces[i] = (ulong)-1;

    return (addressspaces);
}

/********************** Space ID **********************************/

os_obj_handle_t* GetSpaceId (void)
{
    ulong tcbptr, ssr;
    static os_obj_handle_t spaceid = {0, 0, 0, 0};
    tcbptr = GetCurrTask();
        ssr = DreadLong(tcbptr + CONTEXT_ssrelr + 4);
    spaceid.address = ASID_FROM_SSR(ssr);
    DEBUG_MSG("GetSpaceId: spaceid.address %x\n", spaceid.address);
    return &spaceid;
}

/********************** Task Space ID *****************************/

ulong TaskSpaceId( ulong threadptr)
{
    ulong ssr, tmp;
    ssr = DreadLong(threadptr + CONTEXT_ssrelr + 4);
    tmp = ASID_FROM_SSR(ssr);    // spaceId
    return tmp;
}

/********************* Task Trace ID ******************************/

ulong TaskTraceId( ulong threadptr)
{
    ulong tid;
    tid = DreadLong(threadptr + CONTEXT_tid);
    return tid;
}

/**********************  Task List Routine  **********************/

ulong* TaskList (void)
{
    static ulong tcbs[OSAM_MAX_THREADS];
    //ulong tcbhead, tcbptr, tmp, raw_id, context_size;
    ulong tcbhead, tcbptr, tmp, raw_id;
    int  i = 0, index = 0;
    union phv_t phv;
    if(OS_Initialized() ==0)
        return 0;

    //context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));
    if (!island_mode)
    {
        tcbptr = par_tcblist;
         do{
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if(phv.info.valid == 1 )
            {
                tcbs[i++] = tcbptr;
            }
        //DEBUG_MSG("TaskList tcbptr=%x phv.raw=%x phv.info.valid=%x\n",tcbptr,phv.raw,phv.info.valid);
            tcbptr = tcbptr + context_size;
            index++;
        } while(index < max_threads_ddr);
    }

    if (max_threads_tcm) {
        tcbptr = DgetSymbolAddress(TCB_LIST_TCM);
        index = 0;
        do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if(phv.info.valid == 1 )
            {
                tcbs[i++] = tcbptr;
            }
            tcbptr = tcbptr + context_size;
            index++;
            //sqiao
            //printf("In TCM: tcbptr=%8x\n",tcbptr);
        } while(index < max_threads_tcm);
    }

    tcbs[i] = 0;
    //i=0;
    //while(tcbs[i]) DEBUG_MSG("tcbs[%x]=%x ",i,tcbs[i++]); DEBUG_MSG("\n");
    return tcbs;
}

/**********************  Task Stack Routine  **********************/
//this routine seemed to have been displaying posix thread stack... not needed now, can delete it
//T32StackExInitRtn  TaskStackInit (void)
//{
//  return TaskStackCalc;
//}
#if 0
T32StackExResult* TaskStackCalc (ulong tcbptr, ulong type)
{
    static T32StackExResult stack = {0, 0, 0, 0};
    ulong fd_base;

    stack.magic = tcbptr;
    //stack.sp    = DreadLong (tcbptr + MEM_QDSP6_IRQ_CONTEXT_T_SP_START + MEM_QDSP6_IRQ_CONTEXT_T_SP_START );
    stack.sp    = DreadLong (tcbptr + CONTEXT_r2928 + 4);
    stack.high   = DreadLong (tcbptr + CONTEXT_ugpgp + 4); //this is ugp
    //stack.high = 0; //used to be 0, change for test only
    //stack.low  = DreadLong(fd_base + off_pthread_staddr);
    stack.low = 0;
    //T32Printf("tcbptr is : %x\n", tcbptr);
    return &stack;
}
#endif

//T32StackExResult* TaskStackCalc (ulong fd_base, ulong type)
//{
//    static T32StackExResult stack = {0, 0, 0, 0};
    //ulong fd_base;

//    stack.magic = fd_base;
//# if 0
    //fd_base = DreadLong(par_pt_table);
//    len_pthread = T32SymbolTypeLink("pthread_i");
//    off_pthread_attr = T32SymbolTypeOffsetGet(".attr");

//    len_pthread = T32SymbolTypeLink("pthread_attr_t");
//    off_pthread_staddr = T32SymbolTypeOffsetGet(".stackaddr");
//    T32DebugPrintf("fd_base: %x, off_pthread_attr: %x\n", fd_base, off_pthread_attr);
//    off_pthread_stsize = T32SymbolTypeOffsetGet(".stacksize");

//# endif

    //stack.sp    = DreadLong (tcbptr + MEM_QDSP6_IRQ_CONTEXT_T_SP_START + MEM_QDSP6_IRQ_CONTEXT_T_SP_START );
//    stack.sp = DreadLong (fd_base + off_pthread_attr + off_pthread_staddr);
    //DreadLong (tcbptr + CONTEXT_r2928 + 4);
//    stack.high   = 0;//DreadLong (tcbptr + CONTEXT_ugpgp + 4); //this is ugp
    //stack.high = 0; //used to be 0, change for test only
    //stack.low  = DreadLong(fd_base + off_pthread_staddr);
//    stack.low = 0;
    //T32Printf("tcbptr is : %x\n", tcbptr);
//    return &stack;
//}

/*********************** Task context Routine ***********************/

arch_context_t* GetContext (ulong tcbptr)
{
    static arch_context_t context[] = {
    {"r0",  0, 0},
    {"r1",  0, 0},
    {"r2",  0, 0},
    {"r3",  0, 0},
    {"r4",  0, 0},
    {"r5",  0, 0},
    {"r6",  0, 0},
    {"r7",  0, 0},
    {"r8",  0, 0},
    {"r9",  0, 0},
    {"r10", 0, 0},
    {"r11", 0, 0},
    {"r12", 0, 0},
    {"r13", 0, 0},
    {"r14", 0, 0},
    {"r15", 0, 0},
    {"r16", 0, 0},
    {"r17", 0, 0},
    {"r18", 0, 0},
    {"r19", 0, 0},
    {"r20", 0, 0},
    {"r21", 0, 0},
    {"r22", 0, 0},
    {"r23", 0, 0},
    {"r24", 0, 0},
    {"r25", 0, 0},
    {"r26", 0, 0},
    {"r27", 0, 0},
    {"r28", 0, 0},
    {"sp",  0, 0},
    {"fp",  0, 0},
    {"lr",  0, 0},
    {"elr", 0, 0},
    {"pc",  0, 0},
    {"ssr", 0, 0},
    {"badva", 0, 0},
    {0,     0, 0}
    };
    static arch_context_t emptycontext[] = { {"hardwarethread", 0, 0},  {0} };

    if(isRunning(tcbptr) && (GetHWThreadNum(tcbptr) != (ulong)-1))
    {
        emptycontext[0].address = GetHWThreadNum(tcbptr);
        return emptycontext;
    }

    context[0].address  = DreadLong(tcbptr + CONTEXT_r0100);
    context[1].address  = DreadLong(tcbptr + CONTEXT_r0100 + 4);
    context[2].address  = DreadLong(tcbptr + CONTEXT_r0302);
    context[3].address  = DreadLong(tcbptr + CONTEXT_r0302 + 4);
    context[4].address  = DreadLong(tcbptr + CONTEXT_r0504);
    context[5].address  = DreadLong(tcbptr + CONTEXT_r0504 + 4);
    context[6].address  = DreadLong(tcbptr + CONTEXT_r0706);
    context[7].address  = DreadLong(tcbptr + CONTEXT_r0706 + 4);
    context[8].address  = DreadLong(tcbptr + CONTEXT_r0908);
    context[9].address  = DreadLong(tcbptr + CONTEXT_r0908 + 4);
    context[10].address = DreadLong(tcbptr + CONTEXT_r1110);
    context[11].address = DreadLong(tcbptr + CONTEXT_r1110 + 4);
    context[12].address = DreadLong(tcbptr + CONTEXT_r1312);
    context[13].address = DreadLong(tcbptr + CONTEXT_r1312 + 4);
    context[14].address = DreadLong(tcbptr + CONTEXT_r1514);
    context[15].address = DreadLong(tcbptr + CONTEXT_r1514 + 4);
    context[16].address = DreadLong(tcbptr + CONTEXT_r1716);
    context[17].address = DreadLong(tcbptr + CONTEXT_r1716 + 4);
    context[18].address = DreadLong(tcbptr + CONTEXT_r1918);
    context[19].address = DreadLong(tcbptr + CONTEXT_r1918 + 4);
    context[20].address = DreadLong(tcbptr + CONTEXT_r2120);
    context[21].address = DreadLong(tcbptr + CONTEXT_r2120 + 4);
    context[22].address = DreadLong(tcbptr + CONTEXT_r2322);
    context[23].address = DreadLong(tcbptr + CONTEXT_r2322 + 4);
    context[24].address = 0;    //r24
    context[25].address = 0;    //r25
    context[26].address = 0;    //r26
    context[27].address = 0;    //r27
    context[28].address = DreadLong(tcbptr + CONTEXT_r2928);
    context[29].address = DreadLong(tcbptr + CONTEXT_r2928 + 4);    //sp
    context[30].address = DreadLong(tcbptr + CONTEXT_r3130);        //fp
    context[31].address = DreadLong(tcbptr + CONTEXT_r3130 + 4);    //lr
    context[32].address = DreadLong(tcbptr + CONTEXT_ssrelr);       //elr
    context[33].address = DreadLong(tcbptr + CONTEXT_ssrelr);       //pc
    context[34].address = DreadLong(tcbptr + CONTEXT_ssrelr + 4);   //ssr
    context[35].address = 0;    //badva

    return context;
}

ulong returnMagic(ulong tnum)
{
    ulong sgp;

    if(OS_Initialized() != 1)
        return (ulong)-1;
    sgp = DreadThreadRegister(tnum, REG_SGP);
    if((sgp ==0) || (par_tcblist ==0))
        return (ulong)-1;
    return DreadLong(sgp);

    ///* here's another reliable (but slow) way to implement this function
    //* union phv_t phv;
    //* tcbptr = par_tcblist;
    //* do {
    //*     phv.raw = DreadLong(tcbptr + CONTEXT_prio);
    //*     if(phv.info.valid == 1 && phv.info.hthread == tnum)
    //*             return tcbptr;
    //*     tcbptr = tcbptr + THREAD_CONTEXT_TOTALSIZE;
    //*     index++;
    //* } while(index < MAX_THREADS);
    //*/
    //
}

/************************* TASK.TASKLIST **********************/

/* Parser: returns initialization routine */

info_cmd_init_t TlParser (void)
{
  argvalue[0] = T32ParseValue ("<task>");
  return TlInit;
}

/* Initialization */

cmd_display_t* TlInit (void)
{
  char cmd_str[50];
  static const char header[] =
    "tcbptrr      UGP         threadID       priority    ASID     TNUM    state      Task Name";

  static cmd_display_t displayone = {WINDOW, 0, 240, header, TlDisplayOne};
  static cmd_display_t displayall = {WINDOW, 0, 240, header, TlDisplayAll};
  static cmd_display_t errdisp =
  {
      MESSAGE, 0, 0,
      "Sorry: Couldn't get task list", 0
  };

  if (OS_Initialized() != 1) return &errdisp;

  tl_flag = 0;
  if (argvalue[0]) return &displayone;
  else          return &displayall;
}

/* call this function only if you know the thread is running */
ulong GetHWThreadNum(ulong tcbptr)
{
    union phv_t phv;
    phv.raw = DreadLong(tcbptr + CONTEXT_prio);
    if (phv.info.hthread <= 5)
        return phv.info.hthread;
    return (ulong)-1;
}

void tldisplayline (ulong tcbptr)
{
    unsigned int my_context, stack_ovf, asid;
    size4u_t ssr, tnum, ugp, threadID;
    union phv_t phv;
    //unsigned long long tcycles, tcycles_hi;

    T32DisplayAttribute (ATTR_GREY);
    /* display magic = tcb address */
    T32EventDef("TASK.TL %x", tcbptr);
    T32Printf ("%8x", tcbptr);
    T32Printf("    ");

    /* display space id */
    //ssr = DreadLong(tcbptr + CONTEXT_ssrelr + 4);
    //T32Printf ("   %4x     ", ASID_FROM_SSR(ssr));
    //tcycles = DreadLong(tcbptr + CONTEXT_totalcycles);
    //tcycles_hi = DreadLong(tcbptr + CONTEXT_totalcycles + 4);
    //tcycles = ((tcycles_hi<<32) + tcycles)*0x2aaa>>16;
    //T32Printf(" %16x ", tcycles);

    ugp = DreadLong(tcbptr + CONTEXT_ugpgp + 4);
    T32EventDef ("v.v %%hex %%tree.open (QURT_utcb_t)%x", ugp);
    T32Printf("%8x    ",ugp);

    threadID = DreadLong(tcbptr + CONTEXT_thread_id);
    T32Printf("%8x    ",threadID);

    phv.raw = DreadLong(tcbptr + CONTEXT_prio);
    T32Printf("       %2x   ", phv.info.prio);

    /* display asid */
    asid = TaskSpaceId(tcbptr);
    T32Printf ("      %x    ", asid);

   if(isRunning(tcbptr))
    {
        tnum = GetHWThreadNum(tcbptr);
        /* Print HW TNUM */
        if(tnum >= 0 && tnum < 6)
            T32Printf("   %1x     ", tnum );
        DISP_MSG("RUNNING         ");
    }
    else if(DreadLong(tcbptr + CONTEXT_cfg) & 0x20)
    {
        DISP_MSG("      FROZEN    ");
    }
    else if (DreadLong(tcbptr+CONTEXT_cfg) & 0x80)
    {

        DISP_MSG("      CAPTURED  ");
    }
    else if (DreadLong(tcbptr + CONTEXT_cfg) & 0x200)
    {

        DISP_MSG("FREEZE_REQUESTED");
    }
    else if (isWaiting(tcbptr))
    {
        DISP_MSG("         WAITING         ");
    }
    else if (isSuspended(tcbptr))
    {
        DISP_MSG("         SUSPENDED       ");
    }
    else
    {
        DISP_MSG( "         READY          ");
    }

#if 0
    /* check for stack ovf: if occurred: addr of overwritten marker
                            if not ocurreed: blank
                            if no stack size: unknown               */
#endif
    /* display task name */

    T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", tcbptr);

    T32DisplayTaskName (tcbptr, 16);

    T32Printf("\n");
}

void TlDisplayOne (void)
{

    unsigned long tcbptr  = argvalue[0];
    tldisplayline(tcbptr);
    T32Printf("\n");

    T32DisplayAttribute (ATTR_GREY_UL);
    T32Printf ("Additional Thread Info\n");
    T32Printf("UTCB: ????????\n");
    T32ButtonDef("var.frame /locals /task %x", tcbptr);
    T32Printf("Stack Frame");
    T32Printf("\n");
}

void TlDisplayAll (void)
{
  //ulong tcbptr ,context_size,
  ulong tcbptr, index=0;
  union phv_t phv;
  if (!island_mode){

      /* read first space pointer */
      tcbptr = par_tcblist;
      DEBUG_MSG("\ntcbhead %x\n", tcbptr);
      do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
        if(phv.info.valid == 1 )
            {
                 //if (T32IsLineDisplayed(1))
                    tldisplayline (tcbptr);
                //else
                //    T32Printf("\n");
            }
            tcbptr = tcbptr + context_size;
            index++;
       } while(index < max_threads_ddr);

  }
   //display threads from TCM
   if (max_threads_tcm){
        index = 0;
        tcbptr = DgetSymbolAddress(TCB_LIST_TCM);
        do {
             phv.raw = DreadLong(tcbptr + CONTEXT_prio);
             DEBUG_MSG("tcbptr %x, phv.raw %x, phv.raw.valid %x\n", tcbptr, phv.raw, phv.info.valid);
             if(phv.info.valid == 1 )
             {
                //if (T32IsLineDisplayed(1))
                  tldisplayline (tcbptr);
                //else
                //     T32Printf("\n");
            }
            tcbptr = tcbptr + context_size;
            index++;
        } while (index < max_threads_tcm);
    }//end if
 }

/***************************************************************************************/
info_cmd_init_t PtParser (void)
{
    return PtInit;
}

cmd_display_t* PtInit (void)
{
    static cmd_display_t ptdisplayall =
    {
        WINDOW2, 0, 80,
        "PAGETABLE",
        PtDisplay
    };
    return &ptdisplayall;
}

void PtDisplay(void) {

    DISP_MSG("\n INfo pagetable not implemented\n");
}

/************************* TASK.SPACELIST **********************/

/* Parser: returns initialization routine */

info_cmd_init_t SlParser (void)
{
  return SlInit;
}

/* Initialization */

cmd_display_t* SlInit (void)
{
  char cmd_str[50];
  static const char header[] =
    "SpaceId  ThreadList";

  static cmd_display_t display = {WINDOW, 0, 80, header, SlDisplay};
  static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't print address spaces", 0};

  if (OS_Initialized() !=1) return &errdisp;

  return &display;
}

void clearAsidArray(void)
{
    int i,j;
    for (i=0;i<MAX_PDS;i++)
        for (j=0;j<OSAM_MAX_THREADS;j++)
            asid_info[i][j] = 0;
}

 /************************* TASK.PTHREADLIST **********************/

/* Parser: returns initialization routine */

info_cmd_init_t PtlParser (void)
{
  return PtlInit;
}

/* Initialization */

cmd_display_t* PtlInit (void)
{
  char cmd_str[50];
  static const char *header[] =
    {"ltcb     ", "Pthread_ID",  "priority", "Pending_signals",  "Waiting_signals", "Name", "stackaddr", "stacksize", 0};

  static cmd_display_t display = {WINDOW2, 0, 180, (char*)header, PtlDisplay};
  static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display posix thread list", 0};

  if (OS_Initialized() !=1) return &errdisp;

  return &display;
}

void PtlDisplay(void)
{
    ulong list_symbol, list_head, cur_thr;
    int i = 0;
    ulong fd_idx;
    static char  name[32]="N/A";

    len_pthread = T32SymbolTypeLink("pthread_i");

    off_pthread_id = T32SymbolTypeOffsetGet(".pthread");
    off_pthread_pend_signals = T32SymbolTypeOffsetGet(".sigpending");
    off_pthread_wait_signals = T32SymbolTypeOffsetGet(".sigwaiting");
    off_pthread_attr = T32SymbolTypeOffsetGet(".attr");
    off_pthread_magic = T32SymbolTypeOffsetGet(".magic");
    //T32Printf("printing magic: %x\n", DreadLong());

    len_pthread = T32SymbolTypeLink("pthread_attr_t");
    off_pthread_priority = T32SymbolTypeOffsetGet(".priority");
    off_pthread_name = T32SymbolTypeOffsetGet(".name");
    off_pthread_staddr = T32SymbolTypeOffsetGet(".stackaddr");
    off_pthread_stsize = T32SymbolTypeOffsetGet(".stacksize");

    list_symbol = DgetSymbolAddress(PTHREAD_LIST);
    if (list_symbol == -1)
    {
        T32Printf("POSIX libary is not used\n");
        return;
    }
    list_head = DreadLong(list_symbol);

   /* read size, which is max index for the pthread fd table
      to find new_fd: (size-1)*8 + fd_base;
      then iterate through all indecies: (size-1)*8 for size = [0:511] */

   fd_base = DreadLong(par_pt_table);
   //if (fd_base == 0) return;

   T32SymbolTypeLink("pthread_id_table");
   fd_size = DreadLong(par_pt_table + 8);
   DEBUG_MSG ("fd_size %x, fd_base: %x\n", DreadLong(par_pt_table + 8), fd_base);
   ltcb_ptr = DreadLong((fd_size-1)*8 + fd_base) ;
   //T32Printf ("looking at first pthread:\n");
   //T32Printf ("ltcb: %x\n", ltcb_ptr);
   for (i = 0; i < (fd_size-1); i++)
   {
        //if non zero object: print
       fd_idx = ((fd_size-1) - i)*8 + fd_base;
       cur_thr = DreadLong(fd_idx);
       //validate the object: fd_idx+4:
       //fd_table size > obj.X.index; fd[obj.X.index].obj.raw == obj.raw;
       //if (DreadLong(fd_idx) && DreadLong(fd_idx+4)
       if(DreadLong(cur_thr + off_pthread_magic) == POSIX_THREAD_MAGIC)
       {
          displayPtlInfoLine(cur_thr);
       }

   }

}

void displayPtlInfoLine (ulong cur_thr)
{
     static char  name[32]="N/A";

     //double click event
     T32EventDef ("v.v %%hex %%tree.open (struct pthread_i)%x", cur_thr);

     //display ltcb addr
     T32Printf("%8x", cur_thr);
     T32DisplaySeparator();

     //display id
     T32Printf("%8x", DreadLong(cur_thr+off_pthread_id));
     T32DisplaySeparator();

     //display priority
     T32Printf("%2x", DreadLong(cur_thr+off_pthread_attr+off_pthread_priority));
     T32DisplaySeparator();

     //display signal_pend
     T32Printf("%8x", DreadLong(cur_thr+off_pthread_pend_signals));
     T32DisplaySeparator();

     //display signal_wait
     T32Printf("%8x", DreadLong(cur_thr+off_pthread_wait_signals));
     T32DisplaySeparator();

     //display name
     DreadBuffer (name, cur_thr+off_pthread_attr+off_pthread_name, 16);
     name[16]=0;
     T32Printf("%s", name);
     T32DisplaySeparator();

     //display stackaddr
     T32Printf("%8x", DreadLong(cur_thr+off_pthread_attr+off_pthread_staddr));
     T32DisplaySeparator();

     //display stacksize
     T32Printf("%8x\n", DreadLong(cur_thr+off_pthread_attr+off_pthread_stsize));
     //T32DisplaySeparator();

     //display magic
     //T32Printf("%8x\n", DreadLong(cur_thr + off_pthread_magic));

}

 /************************* TASK.PMSGQLIST **********************/
/* Parser: returns initialization routine */

info_cmd_init_t PmqlParser (void)
{
  return PmqlInit;
}

cmd_display_t*        PmqlInit     (void)
{
  char cmd_str[50];
  static const char header[] =
    "Msgq_ptr  Msgq_fd   Max_items  Current_used  Waiting_threads";

  static cmd_display_t display = {WINDOW, 0, 180, header, PmqlDisplay};
  static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display posix msgq list", 0};

  if (OS_Initialized() !=1) return &errdisp;

  return &display;
}

void PmqlDisplay  (void)
{
    ulong list_head, cur_thr, max_msgqs_addr, i, mq_list, mq_addr, wait_addr;
    ulong max_msgqs = MAX_MSGQS_DEFAULT;

    if ((max_msgqs_addr = DgetSymbolAddress(MAX_MSGQS_VAR)) != -1)
    {
        max_msgqs = DreadLong(max_msgqs_addr);
    }
    else
    {
        T32Printf("POSIX is not used by target application\n");
        return;
    }

    len_pthread = T32SymbolTypeLink("mq");

    off_pthread_mq_fd = T32SymbolTypeOffsetGet(".mq_desc");
    off_pthread_mq_attr = T32SymbolTypeOffsetGet(".mq_attr");
    off_pthread_mq_waits = T32SymbolTypeOffsetGet(".wait_list");

    len_pthread = T32SymbolTypeLink("struct mq_attr");
    off_pthread_mq_max = T32SymbolTypeOffsetGet(".mq_maxmsg") + off_pthread_mq_attr;
    off_pthread_mq_cur = T32SymbolTypeOffsetGet(".mq_curmsgs") + off_pthread_mq_attr;

    len_pthread = T32SymbolTypeLink("thread_node");
    off_pthread_mq_thread = T32SymbolTypeOffsetGet(".thread");
    off_pthread_mq_next = T32SymbolTypeOffsetGet(".next");

    mq_list = DgetSymbolAddress(PMSGQ_LIST);
    for (i=0; i<max_msgqs; i++)
    {
        mq_addr = DreadLong(mq_list+i*4);
        if (mq_addr == 0)
            continue;
       T32EventDef ("v.v %%hex %%tree.open (mq *)%x", mq_addr);
       T32Printf("%8x  ",mq_addr);
       T32Printf("%8x  ",DreadLong(mq_addr+off_pthread_mq_fd));
       T32Printf("%8x   ",DreadLong(mq_addr+off_pthread_mq_max));
       T32Printf("%8x      ",DreadLong(mq_addr+off_pthread_mq_cur));
       wait_addr = DreadLong(mq_addr+off_pthread_mq_waits);
       while (wait_addr != 0)
        {
            T32Printf("%8x->",DreadLong(wait_addr+off_pthread_mq_thread));
            wait_addr = DreadLong(wait_addr+off_pthread_mq_next);
        }
        T32Printf("NULL\n");
    }

}
info_cmd_init_t       IrqlParser   (void)
{
    argvalue[0] = T32ParseValue("<task>");
    return IrqlInit;
}

cmd_display_t*        IrqlInit     (void)
{
    char cmd_str[50];
    static const char header1[] =
    "L1_IRQ Type               Signal_handle  Signal_mask   ISR";

    static const char header2[] =
    "L2_IRQ Type               Signal_handle  Signal_mask   ISR";

  static cmd_display_t display1 = {WINDOW, 0, 180, header1, IrqL1Display};
  static cmd_display_t display2 = {WINDOW, 0, 180, header2, IrqL2Display};
  static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display posix msgq list", 0};

  if (OS_Initialized() !=1) return &errdisp;

  if (argvalue[0]) return &display2;
  else return &display1;
}

void    IrqL1Display  (void)
{
    ulong int_table, i;
    ulong signal_handle, signal_mask, isr_context;
    ulong off_signal_handle = 0;
    ulong off_signal_mask = 4;
    ulong off_isr_context = 8;
    ulong off_fastfunc_ptr = 8;
    ulong irq_table_entry_size = 16;
    ulong max_l2vic_interrupts = 0;
    max_l2vic_interrupts = DreadLong(DgetSymbolAddress(QURT_MAX_L2VIC));

    int_table = DgetSymbolAddress(INT_TABLE);

    for (i = 0; i <= max_l2vic_interrupts; i++)   // only l2vic interrupts
    {
        signal_handle = DreadLong(int_table+i*irq_table_entry_size+off_signal_handle);

        if (signal_handle == 0) continue;

        signal_mask = DreadLong(int_table+i*irq_table_entry_size+off_signal_mask);
        isr_context = DreadLong(int_table+i*irq_table_entry_size+off_isr_context);

        T32Printf("%5d  ", i );
        if (signal_handle & 0x01)
        { // It's a fast interrupt handler
            T32Printf("Fast IRQ           N/A            N/A            %8x\n", isr_context);
        }
        else { // it's a regular interrupt  handler
            T32Printf("Normal IRQ         ");
            T32EventDef ("v.v %%hex %%tree.open (qurt_anysignal_t *)%x", signal_handle);
            T32Printf("%8x",signal_handle);
            T32Printf("       %8x      ", signal_mask);
            T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", isr_context);
            T32DisplayTaskName (isr_context, 16);
            T32Printf("\n");
        }
    }
}

void    IrqL2Display  (void)
{
    ulong i, l2table_addr;
    ulong off_signal_handle = 0;
    ulong off_signal_mask = 4;
    ulong off_isr_context = 8;
    ulong off_fastfunc_ptr = 8;
    ulong irq_table_entry_size = 16;
    ulong signal_handle, signal_mask, isr_context;
    l2table_addr  = argvalue[0];

    return;  // no other L2 display

}

/***************************** Command to dump futexes with blocking threads ***********************/

info_cmd_init_t       FtxlParser   (void)
{
    return FtxlInit;
}

cmd_display_t*        FtxlInit     (void)
{
    char cmd_str[50];
    static const char header[] =
        "Futex_addr   Object_type     Owner               Obj_Addr    Waiting_threads";

    static cmd_display_t display = {WINDOW, 0, 180, header, FtxlDisplay};
    static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display futex list", 0};

    if (OS_Initialized() !=1) return &errdisp;

    return &display;
}

ulong little_to_big_endian(ulong i)
{
    return ((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i&0xff000000)>>24);
}

ulong isValidUGP(ulong ugp_candidate)
{
    ulong ugp, tcbptr ,context_size, index=0;
    union phv_t phv;

    context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));
    if(ugp_candidate == 0)
        return 0;
    if (!island_mode){

        /* read first space pointer */
        tcbptr = par_tcblist;
        //DEBUG_MSG("\ntcbhead %x\n", tcbptr);
        do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if(phv.info.valid == 1 )
            {
                ugp = DreadLong(tcbptr+CONTEXT_ugpgp+4);
                if (ugp == ugp_candidate)
                    return tcbptr;
            }
            tcbptr = tcbptr + context_size;
            index++;
        } while(index < max_threads_ddr);

    }
    //now check tcm thread contexts
    if (max_threads_tcm)
    {
        index = 0;
        tcbptr = DgetSymbolAddress(TCB_LIST_TCM);
        do{
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if(phv.info.valid == 1 )
            {
                ugp = DreadLong(tcbptr+CONTEXT_ugpgp+ 4);
                 if (ugp == ugp_candidate)
                    return tcbptr;
             }
             tcbptr = tcbptr + context_size;
             index++;
          }while(index < max_threads_tcm);
      }

    return 0;
}

ulong isHolder(ulong thr)
{
    int i;
    for (i=0;i<MAX_MUTEXES;i++)
    {
        if (res_matrix[i].holder == 0)
            break;
        if (res_matrix[i].holder == thr)
            return 1;
    }
    return 0;
}

//(holder, waiter)
ulong isDeadlock(ulong src, ulong dest)
{
    int i, x, y;
    for (i = 0; i < SEARCH_DEPTH; i++)
        deadlock_path[i] = 0;

    // don't like recursive call, don't trust power pc cross-compiler and T32 host.
    for (i = 0; i < SEARCH_DEPTH; i++)
    {
        for (x = 0; x < MAX_MUTEXES; x++)
        {
            if (res_matrix[x].holder)
            {
                for (y = 0; y < res_matrix[x].wait_count; y++)
                {
                    if (res_matrix[x].waiters[y] == src)
                    {
                        if (dest == res_matrix[x].waiters[y])
                        {
                            return 1;
                        }
                        src = res_matrix[x].holder;
                        deadlock_path[i] = src;
                        goto restart_search;
                    }
                }
            }

        }
        return 0;
restart_search:
        break;
    }//end for SEARCH_DEPTH
    return 0;
}

void                  FtxlDisplay  (void)
{
    ulong futex_array[256];
    ulong futex_table, futex_num, i=0, remaining, min, max, size, j, k, tmp, obj_addr, holder;
    ulong x, y, valid_mutex = 0;
    //offsets within qurtk_futex_entry
    ulong ret, offset_obj, offset_queue, offset_next, offset_piholder;
    ulong futex_table_end, futex_size, queue, piholder;
    //indecies for reading fields
    ulong idx_phld, idx_obj, idx_q;

    for (x=0; x<MAX_MUTEXES; x++)
    {
        res_matrix[x].holder = 0;
        res_matrix[x].mutex = 0;
        res_matrix[x].wait_count = 0;
    }

    min = futex_table = DgetSymbolAddress(FUTEX_TABLE);
    futex_table_end = DreadLong(DgetSymbolAddress(FUTEX_TABLE_END));

    //min = futex_table = DgetSymbolAddress(FUTEX_TABLE);
    //futex_num = DreadLong(DgetSymbolAddress(FUTEX_NUM));
    futex_num = DreadLong(DgetSymbolAddress(MAX_THREADS));
    //remaining = 4*futex_num;//size of futex_objs: max_threads * num_words_in_struct (which is 4)
    remaining = (ulong)futex_table_end - futex_table + 1;
    max = min + remaining;

    ret = T32SymbolTypeLink("struct _qurtk_futex_entry");
    //typedef struct _qurtk_futex_entry
    //{
    //    struct _qurtk_futex_entry      *next_futex;
    //    QURTK_thread_context           *queue;     //waiter queue
    //    unsigned int                   *obj_addr;
    //    QURTK_thread_context           *holder_tcb;
    //} QURTK_futex_entry_t;

    offset_obj = T32SymbolTypeOffsetGet(".obj_addr");
    offset_queue = T32SymbolTypeOffsetGet(".queue");
    offset_piholder = T32SymbolTypeOffsetGet(".holder_tcb");
    futex_size = T32SymbolTypeSizeGet("struct _qurtk_futex_entry");

    // offset/4 to use as index for array access
    idx_phld = offset_piholder/4;
    idx_obj = offset_obj/4;
    idx_q = offset_queue/4;

    x = 0; y = 0; j = 0;
    while (remaining != 0)
    {
        size = (remaining >= 1024)?1024:remaining;
        DreadBuffer((char *)futex_array, futex_table+i*1024, size);
        //size/16: 16 = 4(bytes in a word) * 4(every 4th word needs to be inspected based on size of futex_entry struct)
        for (j=0; j<(size/16); j++)
        {
            tmp = little_to_big_endian(futex_array[j*4]); //read every 4th word, which is first elt of futex array
            queue = little_to_big_endian(futex_array[j*4 + idx_q]);

            //if ( (tmp != 0) && ((tmp < min) || (tmp > max)) )
            if (queue != 0)
            {
                //T32Printf("%8x     ", tmp);// <-- this is next field
                T32Printf("%8x      ", futex_table + i*1024 + j*futex_size);
                for (k=0; k<MAX_WAITERS; k++)
                {
                    //if (tmp == 0)
                    if (queue == 0)
                    {
                        T32Printf("NULL\n");
                        break;
                    }
                    if (k == 0)
                    {
                        //obj_addr = DreadLong(tmp+CONTEXT_futex_ptr);
                        obj_addr = little_to_big_endian(futex_array[j*4 + idx_obj]);
                        if (obj_addr == 0)
                            DEBUG_MSG("Error in futex_list: WAITING thread's futex_ptr == NULL!!\n");
                        holder = DreadLong(obj_addr);

                        piholder = little_to_big_endian(futex_array[j*4 + idx_phld]);
                        if (piholder != 0)
                        {
                            if (holder == 0xfe)
                            {
                                T32Printf ("pi mutex       N/A                  %8x    ", obj_addr);
                            }else{
                                T32Printf ("pi mutex       ");
                                T32DisplayTaskName (piholder, 16);
                                T32Printf("     %8x    ", obj_addr);
                                res_matrix[x].holder = piholder;
                                res_matrix[x].mutex = obj_addr;
                                valid_mutex = 1;
                            }
                            //add res_matrix logging for pimutex
                        }else if (holder == 0xfe)
                             T32Printf("mutex           N/A                 %8x    ", obj_addr);
                        else if ((holder = isValidUGP(holder)) != 0)
                        {
                            res_matrix[x].holder = holder;  // for deadlock detection
                            res_matrix[x].mutex = obj_addr;
                            // probably a mutex
                            T32Printf("mutex           ");
                            T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", holder);
                            T32DisplayTaskName (holder, 16);

                            T32EventDef ("v.v %%hex %%tree.open (qurt_mutex_t *)%x", obj_addr);
                            T32Printf ("    %8x    ",obj_addr);
                            valid_mutex = 1;
                        }else {
                            //signal or others
                            T32Printf("signal                              %8x    ", obj_addr);
                        }
                    }

                    //display highest 32 priority waiting threads
                    /* display task name */
                    T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", tmp);
                    T32DisplayTaskName (queue, 16);
                    T32Printf("->");
                    if (valid_mutex)
                    {
                        // for deadlock detection
                        //res_matrix[x].waiters[y++] = tmp;
                        res_matrix[x].waiters[y++] = queue;
                        res_matrix[x].wait_count++;
                    }

                    //DEBUG_MSG("wait_count=%x, y=%x \n",res_matrix[x].wait_count, y);
                    //tmp = DreadLong(tmp);
                    queue = DreadLong(queue);
                }//end if k=0
                // for deadlock dection (res_matrix)
                if (valid_mutex)
                {
                    x++; //next mutex
                    y = 0;
                    valid_mutex = 0;
                }
            }
        }
        remaining -= size;

        i++;
    }
    T32Printf("\n");
    T32Printf("==================================================================================\n");
    T32Printf("Deadlocks detected by T32 QURT extension:\n");
    T32Printf("==================================================================================\n\n");

    i = 0;
    for (x = 0; x < MAX_MUTEXES; x++)
    {
        if (res_matrix[x].holder == 0)
            break;
        if (res_matrix[x].holder != 0)
        {
            for (y = 0; y < res_matrix[x].wait_count; y++)
            {
                if (isHolder(res_matrix[x].waiters[y]) == 0)
                {
                    continue;
                }
                if (isDeadlock(res_matrix[x].holder, res_matrix[x].waiters[y]))
                {
                    i++;
                    T32Printf("Deadlock scenario %d: ",i);
                    T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", res_matrix[x].waiters[y]);
                    T32DisplayTaskName (res_matrix[x].waiters[y], 16);
                    T32Printf("=>");
                    T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", res_matrix[x].holder);
                    T32DisplayTaskName (res_matrix[x].holder, 16);
                    j = 0;
                    while (deadlock_path[j]) {
                        T32Printf("=>");
                        T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", deadlock_path[j]);
                        T32DisplayTaskName (deadlock_path[j], 16);
                        j++;
                    }
                    T32Printf("\n");
                }
            }
        }

    }

    if (i == 0)
        T32Printf("None\n");
//    for (x = 0; x < 64; x++)
//    {
//        T32Printf("holder:%8x ",res_matrix[x].holder);
//        T32Printf("waiters:");
//        for (y = 0; y < 32; y++)
//            T32Printf("%8x ",res_matrix[x].waiters[y]);
//        T32Printf("\n");
//    }
//
}

/***************************** Command to dump debug buffer ***********************/

info_cmd_init_t       DbgParser   (void)
{
    return DbgInit;
}

cmd_display_t*        DbgInit     (void)
{
    char cmd_str[50];
    static const char header[] =
        "Debug Buffer (8 KB)";

    static cmd_display_t display = {WINDOW, 0, 180, header, DbgDisplay};
    static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display debug buffer", 0};
    static cmd_display_t errdisp1 = {MESSAGE, 0, 0, "Sorry: Couldn't display debug buffer in island mode", 0};
    if (OS_Initialized() !=1) return &errdisp;
    if (island_mode) return &errdisp1;

    return &display;
}

int printBuf(char *buf, int size)
{
    char buff[128];
    int prev, i, j;
    prev = i = 0;
    while (i<size)
    {
        if (buf[i] == 0)
            return -1;

        if (buf[i] == 0x0a)
        {
            buf[i]=0;
            j = prev;
            while ((i-j) > 127)
            {//print up to 127 chars in a line
                //void *memcpy(void *dst, const void *src, size_t len);
                memcpy((void *)buff, &buf[j], 127);
                buff[127]=0;
                T32Printf("%s\n",buff);
                j += 127;
            }
            T32Printf("%s\n",&buf[j]);
            prev = i+1;
        }
        else if ((buf[i]<0x20) || (buf[i]>0x7e))
        {
            buf[i]=0x20; //space
        }
        ++i;
    }
    j=prev;
    while ((i-j) > 127)
    {//print up to 127 chars in a line
        memcpy(buff, &buf[j], 127);
        buff[127]=0;
        T32Printf("%s\n",buff);
        j += 127;
    }

    if (j != prev) j++;

    for (i=j; i<size; i++)
        T32Printf("%c",buf[i]);

    return 0;
}

void DbgDisplay(void)
{
    char buf[1024];
    ulong buf_start, index, i=0, addr, ret;

    buf_start = DgetSymbolAddress(QURT_DEBUG_BUF);
    if (buf_start == -1)
        buf_start = DgetSymbolAddress(DEBUG_BUF);

    index = DreadLong(DgetSymbolAddress(DEBUG_BUF_INDEX));

    DreadBuffer(buf, buf_start+index, 1);

    if (index == (DEBUG_BUFFER_SIZE - 1) || buf[0] == 0)
    {
        // start from beginning
        while (i < DEBUG_BUFFER_SIZE)
        {
            DreadBuffer(buf, (buf_start+i), 1024);
            ret = printBuf(buf, 1024);
            if (ret == -1)
                return;
            i += 1024;
        }
    }
    else
    {
        // start from index and print the entire circular buffer
        i = index;
        while (1)
        {
            if ( (i+1024) >= DEBUG_BUFFER_SIZE )
            {
                //Gonna hit bottom boundary, read remaining part
                DreadBuffer(buf, (buf_start+i), (DEBUG_BUFFER_SIZE-i));
                ret = printBuf(buf, (DEBUG_BUFFER_SIZE-i));
                if (ret == -1) return;
                i = 0;
                continue;
            }
            if ( (i<index) && ((i+1024)>=index) )
            {
                // hit where we start, it's last print
                DreadBuffer(buf, (buf_start+i), (index-i));
                printBuf(buf, (index-i));
                return;
            }
            DreadBuffer(buf, buf_start+i, 1024);
            ret = printBuf(buf, 1024);
            if (ret == -1) return;
            i += 1024;
        }
    }
}

/*************** memory display *********************************/

info_cmd_init_t       MemParser   (void)
{
    return MemInit;
}

cmd_display_t*        MemInit     (void)
{
    char cmd_str[50];
    static const char header[] =
        "Free physical memory resource list";

    static cmd_display_t display = {WINDOW, 0, 180, header, MemDisplay};
    static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display free memory list", 0};
    static cmd_display_t errdisp1 = {MESSAGE, 0, 0, "Sorry: Couldn't display free memory list in island mode", 0};
    if (OS_Initialized() !=1) return &errdisp;
    if (island_mode) return &errdisp1;
    return &display;
}

void print_mem(int start, int size, int k)
{
    if (k == 0) T32Printf("    (%5x000->%5x000) ", start, start+size);
    else if ((k%4) == 0) T32Printf("(%5x000->%5x000)\n    ", start, start+size);
    else T32Printf("(%5x000->%5x000) ",start,start+size);
}

char *index_to_size(int i)
{
    switch (i)
    {
        case 0: return "4KB";
        case 1: return "8KB";
        case 2: return "16KB";
        case 3: return "32KB";
        case 4: return "64KB";
        case 5: return "128KB";
        case 6: return "256KB";
        case 7: return "512KB";
        case 8: return "1MB";
        case 9: return "2MB";
        case 10:return "4MB";
        case 11:return "8MB";
        case 12:return "16MB";
        case 13:return "32MB";
        case 14:return "64MB";
        case 15:return "128MB";
        case 16:return "256MB";
        case 17:return "512MB";
        case 18:return "1GB";
        case 19:return "2GB";
        case 20:return "4GB";
        default: T32Printf("unknown memory block size\n");
    }
    return "unknown memory size";
}

void                  MemDisplay  (void)
{
    ulong pool_start, cur_pool, i, j, k, tmp, s, coalesced, phys_pool_struct, off_pool_name, off_config, len_config, tmp_config;
    ulong memlist_first, memlist_last, alloc_list, static_list, next_shared, off_pool, off_pNext, next_pool;
    ulong static_phys, static_virt;
    ulong cur_mem, tcbptr;
    ulong start, size;
    ulong proc_ptr, process, a;
    ulong shmem_list, off_shmem_phys_region, len_shmem, off_next_shared, off_shmem_vma_list;
    char name[32];
    ulong entry[2];

    pool_start = DgetSymbolAddress(POOLS);
    pool_start = DreadLong(pool_start);
    len_pool = T32SymbolTypeLink("struct mem_pool");
    off_pool_coalesced = T32SymbolTypeOffsetGet(".coalesced");

    phys_pool_struct = T32SymbolTypeLink("struct phys_pool");
    off_pool = T32SymbolTypeOffsetGet(".pool");
    off_pNext = T32SymbolTypeOffsetGet(".pNext");
    off_config = T32SymbolTypeOffsetGet(".pConfig");
    len_config = T32SymbolTypeLink("struct phys_mem_pool_config");
    off_pool_name = T32SymbolTypeOffsetGet(".name");

    len_pool = T32SymbolTypeSizeGet("struct phys_pool");
    len_mem_list = T32SymbolTypeLink("struct mem_list");
    len_mem_list = T32SymbolTypeSizeGet("struct mem_list");
    // mem_list_t free_list[MEM_MAX_IDX+1]
    // typedef struct mem_list{
    //      memory_t *tqh_first; //changed to head and tail, respectively
    //      memory_t *tqh_last;
    //      } mem_list_t;

    len_mem = T32SymbolTypeLink("struct memory");
    off_mem_addr = T32SymbolTypeOffsetGet(".pageno");
    off_mem_size = T32SymbolTypeOffsetGet(".page_count");
    off_mem_tqe_next = off_mem_size + 4; //T32 cannot get the tqe_next offset of type "strucyt memory"

    //BLASTK_region_allocated_list is a list of phys mem regions: struct phys_mem_region
    len_phys_region = T32SymbolTypeLink("struct phys_mem_region");
    off_phys_mem = T32SymbolTypeOffsetGet(".phys_mem");
    off_vma = T32SymbolTypeOffsetGet(".vma");

    //len_region = T32SymbolTypeLink("struct mem_region"); //in user space it is unsigned int, in kernel its a struct!
    //kernel structures for reference
    //
    //typedef struct vma_node {
    //  struct vma_node * next;     //A linked list of all VMAs per address space
    //  unsigned int magic;
    //  mem_region_type_t vma_flags; //indicates it's a shared or file backed VMA
    //  mem_t  *virt_mem;
    //  u32_t   owner_thread;
    //  struct phys_mem_region *phys_region_list;
    //  struct vma_node *next_shared;   //linklist of shared VMAs that mapped to one region
    //  u32_t  fd;  // file descriptor where vma is mapped from
    //  u32_t offset; // offset within the file vma starts
    //} vma_node_t;

    //typedef struct phys_mem_region {
    //  struct phys_mem_region     * next;      //A linklist of all physical memory regions in system
    //  unsigned int                magic;      /* This is needed to verify the type of memory region*/
    //  mem_pool_t                 * phys_pool;
    //  mem_t                      * phys_mem;
    //  struct phys_mem_region      *next_scattered; //linked list of scattered regions that backs up one VMA
    //  u32_t                       count;      // count how many VMA mapped to this (shared) phys_region
    //  mem_mapping_t              mapping_type;
    //  mem_cache_mode_t           cache_mode;
    //  vma_node_t                  *vma;
    //  u32_t                       vma_offset; //offset within the VMA, used by VIRTUAL_FIXED mapping to get the vaddr
                                            //of the mapped region
    //} phys_mem_region_t;

   //typedef struct phys_pool {
   //   qurt_qdi_obj_t qdiobj;
   //   struct phys_pool *pNext;
   //   struct phys_mem_pool_config *pConfig;
   //   int qdi_handle;
   //   mem_pool_t pool;
   // }phys_pool_t;

   //struct phys_mem_pool_config{
   //   char name[32];
   //   struct range {
   //      unsigned int start;
   //      unsigned int size;
   //    } ranges[16];
   // };

    len_vma = T32SymbolTypeLink("struct vma_node");
    off_region_virt_mem = T32SymbolTypeOffsetGet(".virt_mem");
    off_owner_thread = T32SymbolTypeOffsetGet(".owner_thread");
    off_phys_list   = T32SymbolTypeOffsetGet(".phys_region_list");
    off_region_type = T32SymbolTypeOffsetGet(".vma_flags");
    off_next_shared = T32SymbolTypeOffsetGet(".next_shared");

    len_shmem= T32SymbolTypeLink("struct shmem_str");
    off_shmem_phys_region = T32SymbolTypeOffsetGet(".phys_mem_region");
    off_shmem_vma_list = T32SymbolTypeOffsetGet(".vma_share_list");

    cur_pool = pool_start;
    next_pool = DreadLong(pool_start + off_pNext);
    for (i = 0; i< MAX_POOLS; i++)
    {
        if(cur_pool == 0) break;
        tmp = cur_pool + off_pool;

        tmp_config = DreadLong(cur_pool + off_config);
        DreadBuffer(name, tmp_config +off_pool_name, 32);
        name[31] = 0;
        coalesced =DreadLong(cur_pool + 32 + off_pool_coalesced);

        T32Printf("Physical Pool: \"%s\", ",name);
        if (coalesced) T32Printf("coalesced.\n");
        else T32Printf("NOT coalesced.\n");

        T32Printf(" List of free memory blocks in order of size:\n");
        T32Printf("    --------------------------------------------------------------------\n");

        for (j = 0; j <= MEM_MAX_IDX; j++)
        {
            memlist_first = DreadLong(tmp+j*len_mem_list);
            memlist_last = DreadLong(tmp+j*len_mem_list+4);
            if (memlist_first != 0)
            {
                T32Printf("    <%s blocks left>:\n", index_to_size(j));
                cur_mem = memlist_first;
                k = 0;

                while (cur_mem != memlist_last)
                {
                    //print cur_mem's starting and ending address range
                    start = DreadLong(cur_mem+off_mem_addr);
                    size = DreadLong(cur_mem+off_mem_size);
                    cur_mem = DreadLong(cur_mem+off_mem_tqe_next);
                    print_mem(start, size, k);
                    k++;
                }

                start = DreadLong(cur_mem+off_mem_addr);
                size = DreadLong(cur_mem+off_mem_size);
                print_mem(start, size, k);
                T32Printf("\n\n");
            }
        }
        cur_pool = DreadLong(cur_pool + off_pNext);
    }

    static_list = DgetSymbolAddress(STATIC_MEMS);
    //static memory: 64 bit pgt format
    T32Printf("\n\nSTATICALLY ALLOCATED MEMORY LIST\n");
    T32Printf("  ------------------------------------------------------------------\n");
    T32Printf("Virtual_addr  Physical_addr    Size       ASID\n");
    T32Printf("\n");
    s = 0;
    do
    {
        DreadBuffer((char *)entry, static_list + s*8, 8);
        //T32Printf ("entry[0] %8x\n", little_to_big_endian(entry[0]));

        static_phys = entry[0]; //first 4 bytes
        if (static_phys == 0) break;
        static_virt = entry[1]; //second 4 bytes
        T32Printf("%8x       ", (little_to_big_endian(static_virt)<<12));
        T32Printf("%5x000        ",  QURT_getPhysAddr(little_to_big_endian(static_phys)));
        T32Printf("0x%x      ",  QURT_pgsize_encode_to_size[ct0(static_phys)]);
        T32Printf("SHARED\n");
        s++;
    }while (static_phys != 0);//end while

    //dynamic memory
    //get space id: start from 0, display entire vma list, shared marked as shared;
    //then go on to other asid's, display only local ones, since shared are redundant in vma list of each asid
   a = 0; //asid
   proc_ptr = DgetSymbolAddress(PROCESS_TABLE); //g_spaces
   process = DreadLong(proc_ptr + 4*a); //space id 0
   T32SymbolTypeLink("struct space");
   off_vma_list = T32SymbolTypeOffsetGet(".vma_list");

    alloc_list = DreadLong(process + off_vma_list);

    T32Printf("\n\nDYNAMICALLY ALLOCATED MEMORY REGION LIST\n");
    T32Printf("  ------------------------------------------------------------------\n");
    T32Printf("Virtual_addr  Physical_addr    Size     Owner thread    ASID \n");
    T32Printf("\n");

    for (a = 0; a < MAX_PROC; a++)
    {
        if (!DreadLong(proc_ptr +4*a)) break;
        alloc_list = DreadLong(DreadLong(proc_ptr + 4*a) + off_vma_list);
        while (alloc_list != 0)
        {
           if (DreadLong(alloc_list + off_region_type) == REGION_LOCAL){
                //virt
                T32Printf("%5x000      ", DreadLong(DreadLong(alloc_list + off_region_virt_mem) + off_mem_addr));
                //phys
                T32Printf("%5x000      ", DreadLong(DreadLong(DreadLong(alloc_list + off_phys_list) + off_phys_mem) + off_mem_addr));
                //size
                T32Printf("0x%x000        ", DreadLong(DreadLong(DreadLong(alloc_list + off_phys_list) + off_region_virt_mem) + off_mem_size));
                //owner
                T32Printf("%8x       ", DreadLong(alloc_list + off_owner_thread));
                //ASID
                T32Printf("%1x      \n", a);
             }
            alloc_list = DreadLong(alloc_list);
            //Do not display next region, as vma list contains shared and non-shared in one list, so next ptr is not iformative to user
        }//while
    }//while

    //show shared memory
    shmem_list = DgetSymbolAddress(SHMEM_LIST);
    if (shmem_list)
    shmem_list = DreadLong(shmem_list);
    while (shmem_list != 0)
    {
       next_shared = DreadLong(shmem_list + off_shmem_vma_list); //first one
       while (next_shared != 0){
            //display non-static, go to next_shared to get all other owners
            if (DreadLong(DreadLong(DreadLong(shmem_list + off_shmem_phys_region) + off_vma) + off_owner_thread)){
                //virt
                T32Printf("%5x000      ", DreadLong(DreadLong(next_shared + off_region_virt_mem) + off_mem_addr));
                //phys
                T32Printf("%5x000      ", DreadLong(DreadLong(DreadLong(shmem_list + off_shmem_phys_region)+ off_phys_mem)+ off_mem_addr));
                //size
                T32Printf("0x%x000          ", DreadLong(DreadLong(DreadLong(shmem_list + off_shmem_phys_region) + off_phys_mem)+ off_mem_size));
                //owner
                T32Printf("%x           ", DreadLong(next_shared + off_owner_thread));
                //asid
                T32Printf("SHARED (owner is asid)\n");

              }
          //T32Printf("this next shared %x\n", next_shared);
          next_shared = DreadLong(next_shared + off_next_shared);
      }
      shmem_list = DreadLong(shmem_list);
      //T32Printf("shmem_list %x\n", shmem_list);
    }
}
/********************************************************************************/
info_cmd_init_t     TraceParserRaw (void)
{
    argvalue[0] = T32ParseValue("<task>");
    return TraceInitRaw;
}

cmd_display_t*        TraceInitRaw     (void)
{
    char cmd_str[50];
    static const char header[] =
        "PCYCLES          TNUM   RAW(64 bit)  ";

    static cmd_display_t display = {WINDOW, 0, 180, header, TraceDisplayRaw};
    static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display kernel traces", 0};

    if (OS_Initialized() !=1) return &errdisp;
    else
        return &display;

}

void TraceDisplayRaw(void)
{
#if 0
  //call python from here ... call "wf.task.tracer" so that the script is executed just once
  //wf.* effectively freezes display and refresh
    char buf[16] = {0};
    int  hello_return = -1;
    int i = 3;
    char filename[18] ; //d e c o m p r e s s e d x . b i n
    char ending[5]=".bin";

    T32Printf("q6zip dir is %s\n", q6zip_path);
    hello_return = hello(buf, 16);
    if(hello_return ==0){
        T32Printf("hello executed, buf[1]: %c \n", buf[1]);
    }
    T32Printf("executing file write ... \n");
    //T32Execute("OS.area python file_write.py test.txt");
    T32Printf("check for test.txt in cur dir .. \n");
//#if 0
    T32Printf("loading bin \n");
    //make a file name: decompressed3.bin
    strcpy(filename, "decompressed");
    T32Printf("filename %s\n", filename);
    //T32Printf("filename %s\n", filename);
    //T32Execute ("Data.LOAD.Binary \"decompressed3.bin\" vm:0--0x1000 /NOCLEAR");
    T32Execute ("Data.LOAD.Binary %s%d%s vm:0--0x1000 /NOCLEAR", filename, i, ending);
    T32SetMemoryAccessClassString ("VM");
    T32ReadBuffer (buf, 0, 16);
    buf[16]=0;
    T32DebugPrintf("buf %x\n", T32Endian(*((ulong*)buf)));
    T32DebugPrintf("buf + 4 %x\n", T32Endian(*((ulong*)(buf+4))));
    T32SetMemoryAccessClassString ("MEM_ACESS_DEFAULT");
    T32DebugPrintf("finished loading \n");
    byte tbuf[4] = {1,2,3,4};
    T32WriteBuffer(tbuf, 0xA000003c, 4);

//#endif
#endif
    //read the file generated by python into some static buffer allocated here
}
/**********************************************************************************/

info_cmd_init_t       TraceParser   (void)
{
    argvalue[0] = T32ParseValue("<task>");
    return TraceInit;

}

cmd_display_t*        TraceInit     (void)
{
 char cmd_str[50];
    static const char header[] =
    "PCYCLES          TNUM  EVENT               SCHED_TIME  TRACING_DATA";

 static cmd_display_t display = {WINDOW, 0, 180, header, TraceDisplay};
  static cmd_display_t errdisp = {MESSAGE, 0, 0, "Sorry: Couldn't display kernel traces", 0};
static cmd_display_t errdisp1 = {MESSAGE, 0, 0, "Please provide TNUM in task.tracelist command line", 0};

  if (OS_Initialized() !=1) return &errdisp;

  if (argvalue[0]) return &display;
  else return &errdisp1;

}

/****************************************TASK.DLP********************/

info_cmd_init_t       CmdDLPoolInfoParser (void)
{
    argvalue[0] = T32ParseValue("<task>");
    return CmdDLPoolInfoInit;
}

cmd_display_t*  CmdDLPoolInfoInit (void)
{
    static const char* header[] =   // window headline
        { "Index", "Pool Size ", "Pool Address", "Total Pages", "Pages Used", "\% Full", "Load Count", "Replace Count","Log Entries", "Used Size", "Log File",0 };

    static cmd_display_t displayall =   // Display definition for TASK.DLPI
    {
        WINDOW2,        // Type of display TBD...Ramesh
        10, 120,          // default heigth and width
        (char*) header, // headline
        CmdDLPoolInfoDisplayAll    // Display routine
    };

    static cmd_display_t errdisp =     // Error message
    {
       MESSAGE,
       0, 0,
        "Sorry: Couldn't get symbol (swap_pools)",
       0
    };

    static cmd_display_t vererrdisp =     // Error message
    {
       MESSAGE,
       0, 0,
        "Sorry: Trace32 version 11180 is required to use this feature",
       0
    };

    /*if (T32_version_numer < 11180)
        return &vererrdisp; TBD...Ramesh */

    InitDLPools(); //looks redundant, but useful

    // return display definition
    if (!IsSetupDLPI())
        return &errdisp;

        return &displayall;
}

void InitDLPools(void)
{
    len_dl_pool_array = T32SymbolSizeGet ("swap_pools");
    len_dl_pool_entry = T32SymbolTypeSizeGet ("struct swap_page_pool");
    //sturct load_log_list_entry->struct dl
    len_dl_load_log_entry = T32SymbolTypeSizeGet ("struct load_log_list_entry");

    off_dl_pool_pool = T32SymbolTypeOffsetGet ("struct swap_page_pool.active_page_pool");
    off_dl_pool_size = T32SymbolTypeOffsetGet ("struct swap_page_pool.active_page_pool_size");

    off_dl_pool_list = T32SymbolTypeOffsetGet ("struct swap_page_pool.active_page_list");
    off_dl_pool_list_count = T32SymbolTypeOffsetGet ("struct swap_page_pool.active_page_list_count");
    off_dl_pool_list_index = T32SymbolTypeOffsetGet ("struct swap_page_pool.active_page_list_index");

    off_dl_pool_replace_count = T32SymbolTypeOffsetGet ("struct swap_page_pool.page_replacement_count");

    off_dl_load_log_list = T32SymbolTypeOffsetGet ("struct swap_page_pool.load_log_list");
    off_dl_load_log_list_count = T32SymbolTypeOffsetGet ("struct swap_page_pool.load_log_list_count");
    off_dl_load_log_list_index = T32SymbolTypeOffsetGet ("struct swap_page_pool.load_log_list_index");

    off_dl_load_log_entry_addr = T32SymbolTypeOffsetGet("struct load_log_list_entry.loaded_vaddr");
    off_dl_load_log_entry_evic = T32SymbolTypeOffsetGet("struct load_log_list_entry.evicted_vaddr");
    off_dl_load_log_entry_thread = T32SymbolTypeOffsetGet("struct load_log_list_entry.thread");

    DEBUG_MSG("len_dl_pool_entry is %d", len_dl_pool_entry);
    DEBUG_MSG("len_dl_pool_array is %d", len_dl_pool_array);
    DEBUG_MSG("len_dl_load_log_entry is %d", len_dl_load_log_entry);
    DEBUG_MSG("off_dl_pool_pool is %d", off_dl_pool_pool);
    DEBUG_MSG("off_dl_pool_size is %d", off_dl_pool_size);
    DEBUG_MSG("off_dl_pool_list is %d", off_dl_pool_list);
    DEBUG_MSG("off_dl_pool_list_count is %d", off_dl_pool_list_count);
    DEBUG_MSG("off_dl_pool_list is %d", off_dl_pool_list_index);
    DEBUG_MSG("off_dl_pool_replace_count is %d", off_dl_pool_replace_count);
    DEBUG_MSG("off_dl_load_log_list is %d", off_dl_load_log_list);
    DEBUG_MSG("off_dl_load_log_list_count is %d", off_dl_load_log_list_count);
    DEBUG_MSG("off_dl_load_log_list is %d", off_dl_load_log_list_index);

    par_dl_pool_array = T32SymbolAddrGet ("swap_pools");
    DEBUG_MSG("dl_pool_array is %x", par_dl_pool_array);
}

void CmdDLPoolInfoDisplayAll (void)
{
    ulong index;
    ulong array_len = len_dl_pool_array / len_dl_pool_entry;
    ulong max_pools = 10;
    byte   pool[len_dl_pool_entry];

    for(index=0; index<array_len && index<max_pools; index++) {
        ulong pool_ptr = (ulong)((byte*)par_dl_pool_array + (len_dl_pool_entry*index));

        T32ReadBuffer (pool, pool_ptr, len_dl_pool_entry);
        displayDLPoolInfoLine (index, pool);
    }
}

void displayDLPoolInfoLine (ulong index, byte* pi)
{
    ulong tmp, tmp2;

    // display pool index
    T32Printf   ("%d", index);
    T32DisplaySeparator();

    // display pool size
    tmp = T32Extract (pi + off_dl_pool_size, 4);
    T32Printf   ("%d KB", tmp*4);
    T32DisplaySeparator();

    // display pool addr
    tmp = T32Extract (pi + off_dl_pool_pool, 4);
    T32Printf   ("0x%x", tmp);
    T32DisplaySeparator();

    // display total pages
    tmp = T32Extract (pi + off_dl_pool_size, 4);
    T32Printf   ("%d pages", tmp);
    T32DisplaySeparator();

    // display pages used
    tmp = T32Extract (pi + off_dl_pool_list_count, 4);
    T32Printf   ("%d pages", tmp);
    T32DisplaySeparator();

    // display pct full
    tmp = T32Extract (pi + off_dl_pool_list_count, 4);
    tmp2 = T32Extract (pi + off_dl_pool_size, 4);
    T32Printf   ("%d%%", (tmp * 100) / tmp2);
    T32DisplaySeparator();

    // total number of loads
    tmp = T32Extract (pi + off_dl_pool_list_count, 4);
    tmp2 = T32Extract (pi + off_dl_pool_replace_count, 4);
    T32Printf   ("%d loads", tmp+tmp2);
    T32DisplaySeparator();

    // display number of evictions
    tmp = T32Extract (pi + off_dl_pool_replace_count, 4);
    T32Printf   ("%d evicts", tmp);
    T32DisplaySeparator();

    // display log entries
    tmp = T32Extract (pi + off_dl_load_log_list_count, 4);
    T32Printf   ("%d entries", tmp);
    T32DisplaySeparator();

    // display used log size
    T32Printf   ("%d KB", tmp * len_dl_load_log_entry / 1024);
    T32DisplaySeparator();

    // display view button
    T32ButtonDef ("TASK.DlLoadLog %d", index+1);
    T32Printf   (" View ");
    T32DisplaySeparator();

    T32Printf ("\n");
}

T32CmdInitRtn CmdDLLoadLogParser (void)
{
    argvalue[0] = T32ParseValue ("<index>");
    argvalue[1] = T32ParseValue ("<thread>");
    return CmdDLLoadLogInit;    /* Return initialization routine */
}

T32CmdDisplayDef* CmdDLLoadLogInit (void)
{
     static const char* headerlog[] =   // window headline
        { "Entry", "Load Addr ", "Loaded Symbol", "Loaded Module",/*"Thread    ,"*/ "Thread Name", /*"Load Page ",*/ "Evict Addr",  "Evicted Symbol", "Evicted Module",0 };

    static cmd_display_t displaylog =   // Display definition for TASK.DLLL <index>
    {
        WINDOW2,        // Type of display TBD...Ramesh
        0, 100,          // default heigth and width
        (char*) headerlog, // headline
        CmdDLLoadLogDisplayLog    // Display routine
    };

    static cmd_display_t errdisp =     // Error message
    {
       MESSAGE,
       0, 0,
        "Sorry: Couldn't get symbol (swap_pools)",
       0
    };

    //InitL4ThreadList(); //looks redundant, but we need threadlist info TBD...Ramesh
    InitDLPools(); //looks redundant, but useful

    // return display definition
    if (!IsSetupDLPI())
        return &errdisp;

    return &displaylog;

}

void CmdDLLoadLogDisplayLog (void)
{
    ulong index, pool_ptr, log_ptr, log_count, log_index,c;
    long i;
    ulong max_entries = 65536;
    byte   entry[len_dl_load_log_entry];

    /* Grab the index from TASK.DLLL argument */
    index = argvalue[0]-1;

    /* calculate the pool pointer */
    pool_ptr = par_dl_pool_array + (len_dl_pool_entry*index);

    /* Get the log pointer/index/count from pool structure */
    log_ptr = T32ReadLong ( pool_ptr + off_dl_load_log_list );
    log_count = T32ReadLong ( pool_ptr + off_dl_load_log_list_count );
    log_index = T32ReadLong ( pool_ptr + off_dl_load_log_list_index );

    /* Cap the entries to show */
    if( log_count > max_entries ) {
      log_count = max_entries;
    }

    /* Loop through the entries */
    for(i=log_index-1,c=log_count; c>0; i--, c--) {

    /* Wrap around if need be */
        if( i < 0 ) {
      i = log_count - 1;
    }

    /* Display the entry, only if the window is going to display it */
    if( T32IsLineDisplayed(1) ) {
          T32ReadBuffer (entry, log_ptr + (i*len_dl_load_log_entry), len_dl_load_log_entry);
       displayDLLoadLogEntryLine (c, entry);
    } else {
          T32Printf("\n");
    }
    }
}

void GetProgModGlobFromPath (char* path, char** program, char** module, char** global)
{
    char* strchr (char*, char);

    if (path[0] && path[0]=='\\' && path[1]=='\\')
    {
        *program = path+2;
        *module  = strchr (*program, '\\');
        if (*module) {
            **module = 0;
        *((*module)++);
    }
        if (*module)
        {
            *global = strchr (*module, '\\');
            if (*global) {
                **global = 0;
            *((*global)++);
        }
            if (*global)
            {
                char* tmp = strchr (*global, '\\');
            }
        }
    }
}

void GetNameFromThreadID(ulong tmp, char *name)
{
  ulong tcbptr ,context_size, index=0, thread_id;
  union phv_t phv;
  unsigned int off_thread_attr, len_thread_attr;

  context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));

    if (OS_Initialized () != 1)
        DEBUG_MSG("OS_init() failed!\n");
  if (!island_mode){

      /* read first space pointer */
      tcbptr = par_tcblist;
      do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if(phv.info.valid == 1 )
            {
                thread_id = DreadLong(tcbptr + CONTEXT_thread_id);
                if (thread_id == tmp)
                {
                    unsigned int utcb = DreadLong(tcbptr + CONTEXT_ugpgp + 4);
                    //DreadBuffer (name, utcb + UTCB_thread_name0, 16);
                    //get thread name0 offset:
                    len_thread_name = T32SymbolTypeLink("QURT_utcb_t");
                    off_thread_attr = T32SymbolTypeOffsetGet(".attr");

                    len_thread_attr = T32SymbolTypeLink("struct _qurt_thread_attr");
                    off_thread_name = T32SymbolTypeOffsetGet(".name");

                    DreadBuffer (name, utcb + off_thread_attr + off_thread_name, 16);
                    name[16]=0;
                }
            }
            tcbptr = tcbptr + context_size;
            index++;
     } while(index < max_threads_ddr);
  }

}

void displayDLLoadLogEntryLine (ulong index, byte* entry)
{
    char name[16];
    ulong address;
    char path[256];
    char* program = 0;
    char* module = 0;
    char* global = 0;
    ulong tmp, tmp2;

    // display entry number
    T32Printf   ("%d", index);
    T32DisplaySeparator();

    // display load addr
    address = T32Extract (entry + off_dl_load_log_entry_addr, 4);
    T32EventDef ("d.l %x", address); /***** TBD...Ramesh */
    T32Printf   ("0x%8x", address);
    T32DisplaySeparator();

    // get symbol path
    T32GetSymbolPath(path, address, 256);
    path[255] = 0;
    if(path) {
        GetProgModGlobFromPath (path, &program, &module, &global);
    }

    /* If the path is not valid, set addr to 0 to prevent future displays */
    if(strlen(path) <=3 ) {
      address=0;
    }

    //display symbol
    if( global && address ) {
        T32EventDef ("d.l %x", T32SymbolAddrGet(global)); /**** TBD...Ramesh */
        T32Printf   ("%s", global);
    } else {
        T32Printf   ("None");
    }
    T32DisplaySeparator();

    // display module
    if( module && address ) {
        T32Printf   ("%s", module);
    } else {
        T32Printf   ("None");
    }
    T32DisplaySeparator();

    // display thread
    tmp = T32Extract (entry + off_dl_load_log_entry_thread, 4);
#if 0
    T32Printf   ("0x%8x", tmp);
    T32DisplaySeparator();
#endif

    // display thread name
    GetNameFromThreadID(tmp, name);  /*********** TBD...Ramesh  */
    T32Printf   ("%s", name);
    T32DisplaySeparator();

    // diplay load page
#if 0
    T32EventDef ("y.LIST.MODULE %x", address & 0xFFFFF000);
    T32Printf   ("0x%8x", address & 0xFFFFF000);
    T32DisplaySeparator();
#endif

    // display evicted addr
    tmp = T32Extract (entry + off_dl_load_log_entry_evic, 4);
    if(tmp) {
      T32EventDef ("d.l %x", tmp); /*** TBD...Ramesh */
      T32Printf   ("0x%8x", tmp);
    }else{
      T32Printf   ("None");
    }
    T32DisplaySeparator();
   // get symbol path of evicted guy
    T32GetSymbolPath(path, tmp, 256); /*** TBD...Ramesh ***/
    path[255] = 0;

    if(path) {
        GetProgModGlobFromPath (path, &program, &module, &global);
    }

    /* If the path is not valid, set evict addr to 0 to prevent future displays */
    if(strlen(path) <=3 ) {
      tmp=0;
    }

    //display evicted symbol
    if( global && tmp) {
        T32EventDef ("d.l %x", T32SymbolAddrGet(global));  /*** TBD...Ramesh */
        T32Printf   ("%s", global);
    } else {
        T32Printf   ("None");
    }
    T32DisplaySeparator();

    // display evicted module
    if( module && tmp) {
        T32Printf   ("%s", module);
    } else {
        T32Printf   ("None");
    }
    T32DisplaySeparator();

    T32Printf ("\n");

}

/*******************************************************/

ulong get_id(ulong tag)
{
    return (0x000000ff &  (tag >> 16));
}

ulong get_len(ulong tag)
{
    return (0x0000007f &  (tag >> 25));
}

ulong get_pcycle_hi(ulong tag)
{
    return 0x0000ffff &  (tag >> 0 );
}

ulong print_trace(ulong tnum, ulong curr_trace, ulong start, ulong tcm_trace_flag)
{
    ulong tag, id, len, w0, w1;
    unsigned long long prev_pcycles, prev_hi;
    unsigned long pcycles_delta, pcycles;
    union phv_t phv;
    Trace_Record *prev_trace_ptr;
    unsigned long *prev_context_pcycles_ptr;

    if (start == -1)
    {
        //oldest trace
        pcycles = 0;
        tag = 0;
        w0 = 0;
        w1 = 0;
        len = 0;
    }else {
        pcycles = DreadLong(curr_trace+start);
        tag = DreadLong(curr_trace+start+4);
        w0 = DreadLong(curr_trace+start+8);
        w1 = DreadLong(curr_trace+start+12);
        len = get_len(tag)*4;
        if (len == 0) return len;
    }

    if( tcm_trace_flag ) {
        prev_trace_ptr = &prev_tcm_trace[tnum];
        prev_context_pcycles_ptr = &prev_context_pcycles_tcm[tnum];
    }
    else {
        prev_trace_ptr = &prev_trace[tnum];
        prev_context_pcycles_ptr = &prev_context_pcycles[tnum];
    }

    if (prev_trace_ptr->tag != 0)
    {
        // print previous trace record
        // first time call of this function doesn't print anything
        id = get_id(prev_trace_ptr->tag);

        // use current pcycles to calcuate the schedule time for previous event
        prev_hi = get_pcycle_hi(prev_trace_ptr->tag);
        prev_pcycles = ((prev_hi<<32) + prev_trace_ptr->pcycles);

        if( tcm_trace_flag ) {
            T32Printf("%16x  %d  * ",prev_pcycles, tnum);
        }
        else {
            T32Printf("%16x  %d    ",prev_pcycles, tnum);
        }

        switch (id)
        {
            case 10:
                    // verbose logs of context switching
                    // fall-thru
            case 0:
                    if( id == 0 ){
                        T32Printf("Context Switch      ");  // context switch
                    }
                    if( id == 10 ){
                        T32Printf("VB Context Switch   ");  // VB context switch
                    }
                    if ( *prev_context_pcycles_ptr == -1UL){
                        pcycles_delta = -1UL;
                    }
                    else{
                        pcycles_delta = *prev_context_pcycles_ptr - prev_trace_ptr->pcycles;
                    }
                    *prev_context_pcycles_ptr = prev_trace_ptr->pcycles;
                    //T32Printf("prev contxt pcls %x   ", *prev_context_pcycles_ptr);
                    //print if not the last trace
                    if (pcycles_delta == -1UL){
                        T32Printf("N/A         ");
                    }
                    else{
                        T32Printf("%8x    ",pcycles_delta);
                    }
                    if (prev_trace_ptr->w1 == 0)
                    {
                        T32Printf("\"Idle thread\"\n");
                    }
                    else
                    {
                        //if config_priority_inheritance=y
                        if ((DreadLong(par_configs) & 0x00400000) != 0)
                        {
                            T32Printf("\"%s\" (prio=%2x)", MagicToName(prev_trace_ptr->w1), (prev_trace_ptr->w0));
                            T32Printf("\n");
                        }
                        else
                        {
                            T32Printf("\"%s\" (prio=",MagicToName(prev_trace_ptr->w1));
                            phv.raw = DreadLong(prev_trace_ptr->w1 + CONTEXT_prio);
                            T32Printf("%2x)\n", (phv.info.prio));
                        }
                    }
                break;
            case 1: T32Printf("Incoming Interrupt              num=0x%x \n", prev_trace_ptr->w0);break;
            case 2: T32Printf("Futex Wait                      futex_ID=0x%8x, obj_addr=0x%8x\n", prev_trace_ptr->w0, prev_trace_ptr->w1);break;
            case 3: T32Printf("Futex Resume                    futex_ID=0x%8x, ",prev_trace_ptr->w0);
                    if ( (prev_trace_ptr->w1 > max_threads_ddr) && (prev_trace_ptr->w1 != 0x7fff))
                    {
                        //wake up single, "w1" means the tcb of the woken thread
                        T32Printf("wake up tcb=%8x \"%s\"\n",prev_trace_ptr->w1, MagicToName(prev_trace_ptr->w1));
                    } else {
                        //wake up multiple, "w1" means number of waiting threads
                        T32Printf("wake up %2d threads\n", prev_trace_ptr->w1);
                    }
                    break;
            case 4: T32Printf("Page Fault                      \"%s\"", MagicToName(prev_trace_ptr->w0));
                    T32Printf(" faulting addr=0x%8x\n", prev_trace_ptr->w1); break;
            case 5: if (prev_trace_ptr->w0 == 0)
                        T32Printf("Event Wait                    eventID=%d\n", prev_trace_ptr->w1);
                    else
                        T32Printf("Thread Resume                   tcb=%8x \"%s\"\n",prev_trace_ptr->w0, MagicToName(prev_trace_ptr->w0));
                    break;

            case 6:T32Printf("Pi-Futex Wait                   futex_ID=0x%8x  obj_addr=0x%8x \n", prev_trace_ptr->w0, prev_trace_ptr->w1); break;
            case 7:T32Printf("Pi-Futex Resume                 futex_ID=0x%8x, ", prev_trace_ptr->w0);
                    if (prev_trace_ptr->w1 > max_threads_ddr)
                    {
                        //wake up single, "w1" means the tcb of the woken thread
                        T32Printf("wake up tcb=%8x \"%s\"\n",prev_trace_ptr->w1, MagicToName(prev_trace_ptr->w1));
                    } else {
                         //wake up multiple, "w1" means number of waiting threads
                         T32Printf("wake up %2d threads\n", prev_trace_ptr->w1);
                    }
                    break;

            case 8:
                   switch (prev_trace_ptr->w0)
                   {
                       // Power Collapse messages
                       case 1: T32Printf("power collapse prepared successfully\n"); break;
                       case 2: T32Printf("power collapse shutdown failed interrupt pending\n"); break;
                       case 3: T32Printf("power collapse shutdown successfully\n"); break;
                       case 4: T32Printf("power collapse warm boot\n"); break;
                       case 5: T32Printf("power collapse/APCR exit\n"); break;
                       // APCR messages
                       case 8: T32Printf("APCR entering successfully\n"); break;
                   }
                   break;

            case 9: T32Printf("Kernel Interrupt                num=0x%x \n", prev_trace_ptr->w0);break;

            case 11:
                   switch (prev_trace_ptr->w0)
                   {
                       case 1:T32Printf("Register IST Interrupt          intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 2:T32Printf("De-Register IST Interrupt       intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 3:T32Printf("Enable Interrupt with API       intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 4:T32Printf("Disable Interrupt with API      intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 5:T32Printf("Clear Interrupt with API        intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 6:T32Printf("Register ISR Interrupt          intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 7:T32Printf("De-Register ISR Interrupt       intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 8:T32Printf("Register Fastint Interrupt      intNo=0x%x \n", prev_trace_ptr->w1); break;
                       case 9:T32Printf("De-Register Fastint Interrupt   intNo=0x%x \n", prev_trace_ptr->w1); break;
                   }
                   break;

            case 12:
                   switch (prev_trace_ptr->w1)
                   {
                       case 1:T32Printf("Enable IST Interrupt in Ack     intNo=0x%x \n", prev_trace_ptr->w0); break;
                       case 2:T32Printf("Disable IST Interrupt in Ack    intNo=0x%x \n", prev_trace_ptr->w0); break;
                       case 3:T32Printf("Clear IST PendingInt in Ack     intNo=0x%x \n", prev_trace_ptr->w0); break;
                       case 4:T32Printf("Enable Fast int in sleep path   intNo=0x%x \n", prev_trace_ptr->w0); break;
                       case 5:T32Printf("Enable Fast int                 intNo=0x%x \n", prev_trace_ptr->w0); break;
                       case 6:T32Printf("Enable ISR int                  intNo=0x%x \n", prev_trace_ptr->w0); break;
                   }
                   break;

           case 13:
                   switch (prev_trace[tnum].w0)
                   {
                       case 1:T32Printf("Thread Create with allocation   TCB=0x%x \n", prev_trace[tnum].w1); break;
                       case 2:T32Printf("Thread Exit with deallocation   TCB=0x%x \n", prev_trace[tnum].w1); break;
                   }
                   break;
            case 14:
                   //kernel write word
                   T32Printf("Kernel write word      address 0x%x data 0x%x\n", prev_trace[tnum].w0, prev_trace[tnum].w1);
                   break;
            case 25:T32Printf("Priority Set                    thread=\"%s\", prio=%2x\n", MagicToName(prev_trace_ptr->w0), prev_trace_ptr->w1); break;
            case 26:T32Printf("Reschedule from WAIT\n"); break;
            case 27:T32Printf("Reschedule from LOW PRIO\n"); break;
            case 28:T32Printf("Schedule Highest\n"); break;
            case 29:T32Printf("Schedule New                    thread=\"%s\" new prio=%2x me prio=%2x\n", MagicToName(prev_trace_ptr->w0),
                                                                (prev_trace_ptr->w1)>>16, (prev_trace_ptr->w1)&0x0000FFFF);
                    break;
            case 30:T32Printf("Schedule New from Sleep         tcb to schedule=0x%8x thread=\"%s\"\n", prev_trace_ptr->w0, MagicToName(prev_trace_ptr->w0));
                    break;
            case 31:T32Printf("Reschedule bitmask              thread=\"%s\"\n", MagicToName(prev_trace_ptr->w0)); break;
            default:break;
        }
    }
    prev_trace_ptr->pcycles = (unsigned long)pcycles;
    //T32Printf("pcycles read at end: %x\n", pcycles);
    prev_trace_ptr->tag = tag;
    prev_trace_ptr->w0 = w0;
    prev_trace_ptr->w1 = w1;
    return len;
}

unsigned long print_cur_return_next(unsigned long tnum, unsigned long index, unsigned long tcm_trace_flag)
{
    unsigned long cur_buf, len, buf_size;

    if(tcm_trace_flag) {
        cur_buf = DreadLong(trace_tcm_buf+tnum*8+4);
        buf_size = tcm_buffer_size;
    }
    else {
        cur_buf = DreadLong(trace_buf+tnum*8+4);
        buf_size = trace_buf_size;
    }

    if (index == -1)
    {
        // flush the prev_trace in buffer, the return value will used by merged buffer processing
        print_trace(tnum, cur_buf, -1, tcm_trace_flag);
        return -1;
    }

    len = print_trace(tnum, cur_buf, index, tcm_trace_flag);
    if (len == 0) {
        // empty record, two possibility:
        // a) end of buffer (an empty record to indicate end of buffer), in this case, wrap around and keep going
        // b) end of trace, in this case, flush last trace record and return
        if (index == (buf_size - 0x10))
        {
            // assume case a
            index = index - 0x10;
            return index;
        } else {
            //case b
            DEBUG_MSG("invalid trace record at buffer %x, index %x\n");
            return -1;
        }
    }

    index = (index < 0x10)?(buf_size-0x10):(index-0x10);
    return index;
}

/* getting index is different depending which trace buffer is used:
 * in regular case, index is intact, regular case: qurtk_trace_buffers.trace_addr != 0;
 * in case *_bk buffer is used, index has to be calculated based on roll over pcycle
 * that is, assume first record that is non-sequential chronologically is at address x
 * assume the very first record of this hw thread buffer is at address y, then index = x-y;
 */
unsigned long get_initial_index_bk (unsigned long tnum, unsigned long tcm_trace_flag)
{
    /* find rollover idx by traversing from bottom of a hw thread trace, all records
     * should have a decreasing pcycles the first pcycle that is greater than previos
     * is the roll over boundary
     */

    unsigned long long cur_pcycle, prev_pcycle, cur_pcycle_hi, prev_pcycle_hi;
    unsigned long buf_size, cur_record, cur_buf, index=-1;
    int i;
    Index_Pcycles *idxp;

    if(tcm_trace_flag)
    {
        cur_buf = DreadLong(trace_tcm_buf + tnum*8 +4);
        if((cur_buf == 0) || (DreadLong(cur_buf) ==0 )) {
            return -1;
        }
        //T32Printf("curr buf: %x\n", cur_buf);
        idxp = &ip_tcm[tnum];
        buf_size = tcm_buffer_size;
    }
    else
    {
        cur_buf = DreadLong(trace_buf + tnum*8 +4);
        if((cur_buf == 0) || (DreadLong(cur_buf) ==0 )) {
            return -1;
        }
        //T32Printf("curr buf: %x\n", cur_buf);
        idxp = &ip[tnum];
        buf_size = trace_buf_size;
    }

    idxp->original_index = 0;
    idxp->cur_index = 0;
    for (i =1; i < (buf_size >> 3); i++)
    {
        cur_record = cur_buf + buf_size - 0x10*i;
        //T32Printf("cur record : %x\n", cur_record);

        cur_pcycle_hi = (DreadLong(cur_record + 4)) & 0x0000ffff;        //tag of current record
        prev_pcycle_hi = (DreadLong(cur_record -0x10 + 4)) & 0x0000ffff; //tag of the prev record
        //T32Printf("whats in prev tag %x\n", DreadLong(cur_record + 4 - 0x10));

        cur_pcycle = DreadLong(cur_record) + (cur_pcycle_hi << 32);
        prev_pcycle = DreadLong(cur_record - 0x10) + (prev_pcycle_hi << 32);

        if (prev_pcycle > cur_pcycle)
        {
            index = cur_record - cur_buf;
            idxp->original_index = index;
            idxp->cur_index = index - 0x10;
            index -= 0x10;
            break;
        }
    }
    //case of buffer being full with no roll over:
    if (index == -1)
    {
        idxp->original_index= 0;
        index = buf_size-0x10;
        idxp->cur_index = index;
    }

    return index;
}

unsigned long get_initial_index(unsigned long tnum, unsigned long tcm_trace_flag)
{
    unsigned long index, cur_buf, buf_size;
    Index_Pcycles *idxp;

    /* check to see if need to call bk */
    if(tcm_trace_flag)
    {
        if( DreadLong(trace_tcm_buf) == 0 ) {
            index = get_initial_index_bk(tnum, tcm_trace_flag);
            return index;
        }
        index = DreadLong(trace_tcm_buf+tnum*8);
        cur_buf = DreadLong(trace_tcm_buf+tnum*8+4);
        idxp = &ip_tcm[tnum];
        buf_size = tcm_buffer_size;
    }
    else
    {
        if( DreadLong(trace_buf) == 0 ) {
            index = get_initial_index_bk(tnum, tcm_trace_flag);
            return index;
        }
        index = DreadLong(trace_buf+tnum*8);
        cur_buf = DreadLong(trace_buf+tnum*8+4);
        idxp = &ip[tnum];
        buf_size = trace_buf_size;
    }

    // 4W each trace record
    if ( (DreadLong(cur_buf+index) == 0) && (DreadLong(cur_buf+index+4) == 0) )
    {
        // buffer is NOT filled up yet
        if (index == 0)
            // but index is zero, implying empty buffer
            return -1;
    }
    // index could be 0 if buffer was full and restarted
    idxp->original_index = index;

    index = (index < 0x10)?(buf_size-0x10):(index-0x10);
    idxp->cur_index = index;
    //T32Printf("index %x for tnum %d\n", index, tnum);

    return index;

}

unsigned long find_latest(void)
{
    unsigned long long max=0, pcycles, pcycle_hi;
    unsigned long index = -1, i;

    for (i=0; i<=max_hthread; i++)
    {
        pcycle_hi = get_pcycle_hi(prev_trace[i].tag);
        pcycles = ((pcycle_hi<<32) + prev_trace[i].pcycles);
        if (pcycles > max)
        {
            max = pcycles;
            index = i;
        }
    }

    if( tcm_trace_buffer_enabled )
    {
        for (i=0; i<=max_hthread; i++)
        {
            pcycle_hi = get_pcycle_hi(prev_tcm_trace[i].tag);
            pcycles = ((pcycle_hi<<32) + prev_tcm_trace[i].pcycles);
            if (pcycles > max)
            {
                max = pcycles;
                index = i+10;
            }
        }
    }

    return index;
}

unsigned long find_latest_for_tnum(unsigned long tnum)
{
    unsigned long long max=0, pcycles, pcycle_hi;
    unsigned long index = -1;

    pcycle_hi = get_pcycle_hi(prev_trace[tnum].tag);
    pcycles = ((pcycle_hi<<32) + prev_trace[tnum].pcycles);
    if (pcycles > max)
    {
        max = pcycles;
        index = tnum;
    }

    if( tcm_trace_buffer_enabled )
    {
        pcycle_hi = get_pcycle_hi(prev_tcm_trace[tnum].tag);
        pcycles = ((pcycle_hi<<32) + prev_tcm_trace[tnum].pcycles);
        if (pcycles > max)
        {
            max = pcycles;
            index = tnum+10;
        }
    }

    return index;
}

void TraceDisplay  (void)
{
    ulong tnum  = argvalue[0]-1;
    ulong cur_buf, index ;
    ulong next, hw_mask, i, latest;
    ulong buffer_size_addr, tcm_buffer_size_addr, ddr_buffer_size_addr;

    hw_mask = DreadLong(DgetSymbolAddress(VALID_HWT_MASK));

    trace_buf = DgetSymbolAddress(TRACE_BUFFERS);
    trace_ddr_buf = DgetSymbolAddress(TRACE_BUFFERS_DDR);
    trace_tcm_buf = DgetSymbolAddress(TRACE_BUFFERS_TCM);

    buffer_size_addr = DgetSymbolAddress(TRACE_BUFFER_SIZE);
    tcm_buffer_size_addr = DgetSymbolAddress(TRACE_BUFFER_TCM_SIZE);
    ddr_buffer_size_addr = DgetSymbolAddress(TRACE_BUFFER_DDR_SIZE);

    if (trace_buf == -1) {
        T32Printf("Please rebuild QURT with CONFIG_TRACEBUFFER=y to enable kernel traces\n");
        return;
    }

    if (trace_ddr_buf == -1) {
        T32Printf("There is DDR trace buffer. \n");
        return;
    }

    //If buffers in TRACE_BUFFERS is zero, switch to using TRACE_BUFFERS_DDR
    if( DreadLong(trace_buf+4) == 0 )
    {
        trace_buf = trace_ddr_buf;
    }

    // Check trace buffer
    if (buffer_size_addr == -1)
    {
        trace_buf_size = DreadLong(trace_buf+12) - DreadLong(trace_buf+4);
    }
    else
    {
        trace_buf_size = DreadLong(buffer_size_addr);
    }

    // Check DDR trace buffer
    if (ddr_buffer_size_addr == -1)
    {
        ddr_buffer_size = DreadLong(trace_ddr_buf+12) - DreadLong(trace_ddr_buf+4);
    }
    else
    {
        ddr_buffer_size = DreadLong(ddr_buffer_size_addr);
    }

    // Check TCM trace buffer
    if(tcm_buffer_size_addr == -1)
    {
        tcm_buffer_size = 0;
        tcm_trace_buffer_enabled = 0;
    }
    else
    {
        tcm_buffer_size = DreadLong(tcm_buffer_size_addr);
        if( tcm_buffer_size == 0 ) {
            tcm_trace_buffer_enabled = 0;
        }
        else
        {
            if (trace_tcm_buf == -1)
            {
                T32Printf("The TCM trace buffer size is assigned, but no TCM trace buffer is allocated. \n");
                return;
            }

            tcm_trace_buffer_enabled = 1;
            if( DreadLong(trace_buf+4) == DreadLong(trace_tcm_buf+4) )
            {
                trace_tcm_buf = trace_buf;    // directly use trace_buffer as tcm buffer
                trace_buf = trace_ddr_buf;    // always switch trace_buffer to ddr buffer
                trace_buf_size = ddr_buffer_size;
            }
        }
    }

    for (i = 0; i < 6; i++)
    {
        if ( ((1<<i) & hw_mask) && (max_hthread < i) )
        {
            max_hthread = i;
        }
        prev_trace[i].pcycles = prev_trace[i].tag = prev_trace[i].w0 = prev_trace[i].w1 = 0;
        prev_context_pcycles[i] = -1;

        if(tcm_trace_buffer_enabled)  // set TCM trace buffer
        {
            prev_tcm_trace[i].pcycles = prev_tcm_trace[i].tag = prev_tcm_trace[i].w0 = prev_tcm_trace[i].w1 = 0;
            prev_context_pcycles_tcm[i] = -1;
        }
    }

    if (tnum > 5)
    {
        // print merged trace buffers
        // populate the prev_trace buffer for comparision
        for (i = 0; i <= max_hthread; i++)
        {
            index = get_initial_index(i, 0);
            if (index != -1)
            {
                //buffer is not empty
                next = print_cur_return_next(i, index, 0);
                ip[i].cur_index = next;
            }
        }

        if(tcm_trace_buffer_enabled)  // check TCM trace buffer
        {
            for (i = 0; i <= max_hthread; i++)
            {
                index = get_initial_index(i, 1);
                if (index != -1)
                {
                    //buffer is not empty
                    next = print_cur_return_next(i, index, 1);
                    ip_tcm[i].cur_index = next;
                }
            }
        }

        while (1)
        {
            if ( (latest = find_latest()) == -1)
            {
                //all traces are done
                return;
            }

            if(latest >=10)  // TCM trace
            {
                latest -= 10;
                ip_tcm[latest].cur_index = print_cur_return_next(latest, ip_tcm[latest].cur_index, 1);
                if (ip_tcm[latest].original_index == -2)
                {
                    // force flushing prev_trace buffer
                    ip_tcm[latest].cur_index = -1;
                    continue;
                }
                else if (ip_tcm[latest].cur_index == ip_tcm[latest].original_index)
                {
                    // last trace record
                    ip_tcm[latest].original_index = -2;
                }
            }

            else  // DDR trace
            {
                ip[latest].cur_index = print_cur_return_next(latest, ip[latest].cur_index, 0);
                if (ip[latest].original_index == -2)
                {
                    // force flushing prev_trace buffer
                    ip[latest].cur_index = -1;
                    continue;
                }
                else if (ip[latest].cur_index == ip[latest].original_index)
                {
                    // last trace record
                    ip[latest].original_index = -2;
                }
            }
        }
    }

    else  // individual tnum
    {
        if ( ((1<<tnum) & hw_mask) == 0)
            // invalid HW thread
            return;

        index = get_initial_index(tnum, 0);
        if (index != -1)
        {
            //buffer is not empty
            next = print_cur_return_next(tnum, index, 0);
            ip[tnum].cur_index = next;
        }

        if(tcm_trace_buffer_enabled)  // check TCM trace buffer
        {
            index = get_initial_index(tnum, 1);
            if (index != -1)
            {
                //buffer is not empty
                next = print_cur_return_next(tnum, index, 1);
                ip_tcm[tnum].cur_index = next;
            }
        }

        while (1)
        {
            if ( (latest = find_latest_for_tnum(tnum)) == -1)
            {
                //all traces are done
                return;
            }

            if(latest >=10)  // TCM trace
            {
                latest -= 10;
                ip_tcm[latest].cur_index = print_cur_return_next(latest, ip_tcm[latest].cur_index, 1);
                if (ip_tcm[latest].original_index == -2)
                {
                    // force flushing prev_trace buffer
                    ip_tcm[latest].cur_index = -1;
                    continue;
                }
                else if (ip_tcm[latest].cur_index == ip_tcm[latest].original_index)
                {
                    // last trace record
                    ip_tcm[latest].original_index = -2;
                }
            }

            else  // DDR trace
            {
                ip[latest].cur_index = print_cur_return_next(latest, ip[latest].cur_index, 0);
                if (ip[latest].original_index == -2)
                {
                    // force flushing prev_trace buffer
                    ip[latest].cur_index = -1;
                    continue;
                }
                else if (ip[latest].cur_index == ip[latest].original_index)
                {
                    // last trace record
                    ip[latest].original_index = -2;
                }
            }
        }
    }
}

/*********** command for displaying bitmask of configurations *********/

info_cmd_init_t ConfigParser (void)
{
   return ConfigInit;
}

cmd_display_t* ConfigInit (void)
{
   static const char header[]="Build configurations        ";
   static cmd_display_t display = {WINDOW, 0, 60, header, ConfigDisplay};

   OS_Initialized ();
   return &display;

}
void ConfigDisplay (void)
{

   DISP_MSG (  "BUILD CONFIGURATIONS BITMASK   %8x  ", DreadLong(par_configs));
   DISP_MSG ("\n");

   DISP_MSG ("==================================================\n");
   DISP_MSG ("CONFIG_ETM_PROFILING     %d\n", DreadLong(par_configs)  & 0x00000001);
   DISP_MSG ("CONFIG_POSIX             %d\n", (DreadLong(par_configs) & 0x00000002) >> 1);
   DISP_MSG ("CONFIG_DYNAMIC_MEMORY    %d\n", (DreadLong(par_configs) & 0x00000004) >> 2);
   DISP_MSG ("CONFIG_QTIMERS           %d\n", (DreadLong(par_configs) & 0x00000008) >> 3);
   DISP_MSG ("CONFIG_QUBE              %d\n", (DreadLong(par_configs) & 0x00000010) >> 4);
   DISP_MSG ("CONFIG_HW_BITMASK        %d\n", (DreadLong(par_configs) & 0x00000020) >> 5);
   DISP_MSG ("CONFIG_SANITY_CHECK      %d\n", (DreadLong(par_configs) & 0x00000040) >> 6);
   DISP_MSG ("CONFIG_SANITY_FIXUP      %d\n", (DreadLong(par_configs) & 0x00000080) >> 7);
   DISP_MSG ("CONFIG_POWER_MGMT        %d\n", (DreadLong(par_configs) & 0x00000100) >> 8);
   DISP_MSG ("CONFIG_PROFILING         %d\n", (DreadLong(par_configs) & 0x00000200) >> 9);
   DISP_MSG ("CONFIG_TIMETEST          %d\n", (DreadLong(par_configs) & 0x00000400) >> 10);
   //DISP_MSG ("CONFIG_TID_TIMETEST      %d\n", (DreadLong(par_configs) & 0x00000800) >> 11);
   //DISP_MSG ("CONFIG_G0                %d\n", (DreadLong(par_configs) & 0x00001000) >> 12);
   DISP_MSG ("CONFIG_C_SCHEDULER       %d\n", (DreadLong(par_configs) & 0x00002000) >> 13);
   DISP_MSG ("CONFIG_SECTIONS          %d\n", (DreadLong(par_configs) & 0x00004000) >> 14);
   DISP_MSG ("CONFIG_TRACEBUFFER       %d\n", (DreadLong(par_configs) & 0x00008000) >> 15);
   //DISP_MSG ("CONFIG_CHICKEN_BITS      %d\n", (DreadLong(par_configs) & 0x00010000) >> 16);
   //DISP_MSG ("CONFIG_MEM_UART          %d\n", (DreadLong(par_configs) & 0x00020000) >> 17);
   DISP_MSG ("CONFIG_DEBUG             %d\n", (DreadLong(par_configs) & 0x00040000) >> 18);
   DISP_MSG ("CONFIG_RELOC_IMAGE       %d\n", (DreadLong(par_configs) & 0x00080000) >> 19);
   DISP_MSG ("CONFIG_DEMAND_LOADING    %d\n", (DreadLong(par_configs) & 0x00100000) >> 20);
   DISP_MSG ("CONFIG_MP               %d\n", (DreadLong(par_configs) & 0x00200000) >> 21);
   DISP_MSG ("CONFIG_PRIO_INH          %d\n", (DreadLong(par_configs) & 0x00400000) >> 22);
   DISP_MSG ("CONFIG_PRIORITY_SET      %d\n", (DreadLong(par_configs) & 0x00800000) >> 23);
}

/*********************************************************************/

//DEPRECATED!...
/****************** TASK.HEAP ************/
info_cmd_init_t AppHeapParser (void)
{
  argvalue[0] = T32ParseValue("<process>");
  return AppHeapInit;
}

cmd_display_t* AppHeapInit(void)
{
  static const char header[] = "Heap Usage      ";
  static cmd_display_t display = {WINDOW, 0, 60, header, AppHeapDisplay};

  static cmd_display_t errdisp =
  {
    MESSAGE, 0, 0,
     "Sorry: Couldn't get heap info", 0
  };

  if (OS_Initialized () != 1)
    T32DebugPrintf("OS not initialized...\n");

  if (Process_Initialized() != 1)
    return &errdisp;

  return &display;
}

void AppHeapDisplay(void)
{
    //FIXME: heap size needs to be displayed per process
    ulong total_heap_size, actual_heap = 0;
    ulong process = 0;
    ulong off_proc_heapbase, off_proc_heaplimit, asid_in= 0;

    char *process_name;
    const char symbol[32]="\\Global\\_Aldata";
    char slash[32] = "\\\\";
    char full_name[64];

    asid_in = argvalue[0]; //assume valid asid
    //check if process is valid/initialized:

    process_name = NameFromAsid(asid_in);

    T32Printf ("PROCESS: %s\n", process_name);
    memcpy(full_name, process_name, 64);

    strcat(full_name, "_reloc");  //builder attaches _reloc to each elf
    strcat(full_name, symbol);
    //T32Printf("full_name %s\n", full_name);

    if (process_name == "")
    {
       slash[0] = '\0';
    }
    strcat(slash, full_name);
    //T32Printf("symbol path: %s\n", slash);

    if (par_kheap_size != -1)
    {
       T32Printf ("\nKernel Heap Initial Size: %x\n", DreadLong (par_kheap_size));

      DISP_MSG ("Max Kernel Heap Used: %d kbytes \n", DreadLong(par_kernel_heap)*4 );
      DISP_MSG ("\n");
    }

    process = DgetSymbolAddress(PROCESS_TABLE);
    T32SymbolTypeLink("struct space");
    off_proc_heapbase = T32SymbolTypeOffsetGet(".heapBase");
    off_proc_heaplimit = T32SymbolTypeOffsetGet(".heapLimit");

    //T32DebugPrintf ("QURTK_app_heap_sise: %8x\n", DreadLong(par_app_heap_size));

    //T32Printf ("heapBase: %x\n", DreadLong(process + off_proc_heapbase));
    //T32Printf ("heapLimit: %x\n", DreadLong(process + off_proc_heaplimit));

    actual_heap = (ulong) (DreadLong(par_image_vend) - DreadLong(par_heap_base));

    par_app_heap = DgetSymbolAddress( slash );
    //T32Printf ("add of head symbol: %x\n", par_app_heap);

    head_addr = DreadLong(par_app_heap + 4);
    //T32Printf ("head_addr %x\n", head_addr);

    DISP_MSG ("\n");
    DISP_MSG ("free application heap blocks: \n");
    curr_block = head_addr;
    total_heap_size = 0;

    do
    {
       DISP_MSG ("=========================\n");
       DISP_MSG ("curr_block: %x\n", curr_block);
       size_block = DreadLong(curr_block);
       DISP_MSG ("size_block: %x\n", size_block);
       curr_block = DreadLong(curr_block + 4);
       DISP_MSG ("next_ptr: %x\n", curr_block);
       total_heap_size += size_block;
       DISP_MSG ("=========================\n");

    }while (curr_block);

    DISP_MSG ("total free heap: %x\n", total_heap_size);

}
//heap display deprecated

/**********************************************************************/

/***************** TASK.STACKOVF *************************************/
info_cmd_init_t CmdStackOvfParser (void)
{
   return StackOvfInit;
}
cmd_display_t* StackOvfInit(void)
{
   static const char *header[] = {"tcbp      ","corrupt marker", "remaining/total(marker bytes)", "overflow  ", "stack addr", "stack size", "thread name ", 0};
   static cmd_display_t display = {WINDOW2, 0, 60, (char *)header, StackOvfDisplay};

   static cmd_display_t errdisp =
   {
        MESSAGE, 0, 0,
         "Sorry: Couldn't get thread stack", 0
   };

   if (OS_Initialized() != 1)
       return &errdisp;
   return &display;
}

void StackOvfDisplay(void)
{
  ulong tcbptr ,context_size, stack_ovf, index=0;
  size4u_t ugp =0;
  union phv_t phv;
  unsigned int len_utcb, len_thread_attr, off_stack_ptr, off_stack_size, off_thread_attr, stack_ptr, stack_size;

  context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));
  if (!island_mode){

      tcbptr = par_tcblist;
      //T32DebugPrintf("tcb head: %x\n", tcbptr);
      do {
            //T32DebugPrintf("tcbptr: %x\n", tcbptr);
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            //T32Printf("tcbptr: %x, valid: %x\n", tcbptr, phv.info.valid);
            if(phv.info.valid == 1 )
            {
               T32Printf("%8x            ", tcbptr);
               T32DisplaySeparator();
               ugp = DreadLong(tcbptr + CONTEXT_ugpgp + 4);
               //T32Printf("%8x        ", tcbptr);
               if (ugp)
               {
                    detect_stack_ovf(ugp); //does display as well
                    len_utcb = T32SymbolTypeLink("struct QURT_utcb");
                    off_thread_attr = T32SymbolTypeOffsetGet(".attr");

                    len_thread_attr = T32SymbolTypeLink("struct _qurt_thread_attr");
                    off_stack_ptr = T32SymbolTypeOffsetGet(".stack_addr");
                    off_stack_size = T32SymbolTypeOffsetGet (".stack_size");

                    stack_ptr = DreadLong(ugp + off_thread_attr + off_stack_ptr);
                    stack_size = DreadLong(ugp + off_thread_attr +off_stack_size);

                    //display stack addr
                    T32Printf("%8x      ", stack_ptr);
                    T32DisplaySeparator();

                    //display stack size
                    T32Printf("%8x      ", stack_size);
                    T32DisplaySeparator();
               }
               else
               {
                    T32Printf ("unknown     ");
                    T32DisplaySeparator();
               }
               //display thread name
               T32DisplayTaskName (tcbptr, 16);
               T32Printf("\n");
            }

            tcbptr = tcbptr + context_size;
            index++;
      }while(index < max_threads_ddr);
  }

  if (max_threads_tcm)
  {
    index = 0;
    tcbptr = DgetSymbolAddress(TCB_LIST_TCM);
    do {
         phv.raw = DreadLong(tcbptr + CONTEXT_prio);
         if(phv.info.valid == 1 )
         {
            T32Printf("%8x        ", tcbptr);
            T32DisplaySeparator();

            ugp = DreadLong(tcbptr + CONTEXT_ugpgp + 4);
            if (ugp)
            {
                detect_stack_ovf(ugp);
            }
            else
            {
                T32Printf ("unknown   ");
                T32DisplaySeparator();
            }
            //display this task's name
            T32DisplayTaskName (tcbptr, 16);
            T32Printf ("\n");
           }
           tcbptr = tcbptr + context_size;
           index++;
        }while(index < max_threads_tcm);
   }

}

void  detect_stack_ovf(ulong ugp)
{
   unsigned int stack_ptr, stack_size, magic, corrupt_addr, fill_cnt_symbol, fill_cnt;
   unsigned int off_stack_ptr, off_stack_size, len_utcb, off_thread_attr, len_thread_attr;
   unsigned int stack_limit, stack_fill_index, stack_fill_limit;
   int i =0;

   corrupt_addr = 0;
   stack_size = 0;
   stack_ptr = 0;
   len_utcb = T32SymbolTypeLink("struct QURT_utcb");
   off_thread_attr = T32SymbolTypeOffsetGet(".attr");

   len_thread_attr = T32SymbolTypeLink("struct _qurt_thread_attr");
   off_stack_ptr = T32SymbolTypeOffsetGet(".stack_addr");
   off_stack_size = T32SymbolTypeOffsetGet (".stack_size");

   stack_ptr = DreadLong(ugp + off_thread_attr + off_stack_ptr);
   stack_size = DreadLong(ugp + off_thread_attr +off_stack_size);

   fill_cnt_symbol = DgetSymbolAddress(STACK_FILL_CNT);
   fill_cnt = STACK_MAGIC_COUNT;

   if (fill_cnt_symbol != (unsigned int)(-1))
      fill_cnt = DreadLong(fill_cnt_symbol);

   if (fill_cnt == 0)
   {
      corrupt_addr = 0;
      //return corrupt_addr;
      goto display;
   }
   fill_cnt = fill_cnt < STACK_MAGIC_COUNT?
                            fill_cnt:
                            STACK_MAGIC_COUNT;
   /* iterate through stack space looking for markers: if overwritten, signal */
   //stack_magic_len 8
   if ((stack_size == 0) || (stack_ptr == 0))
   {
       corrupt_addr = -1; //print unknown
       //return corrupt_addr;
       goto display;
   }

   stack_fill_index = (unsigned int)stack_ptr;
   /*stack_fill_limit = stack_size > STACK_MAGIC_COUNT?
      (unsigned int)stack_fill_index + STACK_MAGIC_COUNT:
      (unsigned int)stack_fill_index + stack_size; */
   stack_fill_limit = stack_size > fill_cnt?
      (unsigned int)stack_fill_index + fill_cnt:
      (unsigned int)stack_fill_index + stack_size;

   //T32DebugPrintf("ugp: %x: stack fill limit %x, stack_fill_index %x, stack_ptr %x, stack_size %x\n",
           //ugp, stack_fill_limit, stack_fill_index, stack_ptr, stack_size);
   for (; stack_fill_index < stack_fill_limit; stack_fill_index+=4)
   {
      if (DreadLong(stack_fill_index) != STACK_MAGIC)
      {
          corrupt_addr = (unsigned int) stack_fill_index;
          break;
      }
   }

   //T32DebugPrintf("corrupt addr: %x\n", corrupt_addr);
   //display 2 column values: corrupted marker if any and remaining/total bytes of intact markers
display:
    if (corrupt_addr == -1)
    {
        T32Printf("unknown  ");
        T32DisplaySeparator();
        T32Printf("unknown  ");
        T32DisplaySeparator();
        T32Printf("unknown  ");
        T32DisplaySeparator();
    }
    else if (corrupt_addr == 0)
    {
        T32Printf("none   ");
        T32DisplaySeparator();
        T32Printf("%4d / %4d ", fill_cnt, fill_cnt);
        T32DisplaySeparator();
        T32Printf("none ");
        T32DisplaySeparator();
    }
    else
    {
        T32Printf("%x ", corrupt_addr);
        T32DisplaySeparator();
        T32Printf("%4d / %4d   ",  ((unsigned int)(fill_cnt - (stack_fill_limit - corrupt_addr))), fill_cnt);
        T32DisplaySeparator();
        if (((unsigned int)(fill_cnt - (stack_fill_limit - corrupt_addr))) <= 4)
            T32Printf("%x   ", corrupt_addr);
        else
            T32Printf("likely   ");
        T32DisplaySeparator();
    }
   //return corrupt_addr;
}

/********************************************************************/

/************************ TASK.GSTACK *******************************/
info_cmd_init_t CmdGStackParser (void)
{
   //take tcb
   argvalue[0] = T32ParseValue("<tcbptr>");
   return GStackInit;
}

cmd_display_t* GStackInit (void)
{
  static const char header[] = "thread stack      ";
  static cmd_display_t display = {WINDOW, 0, 60, header, GStackDisplay};
  if (OS_Initialized() != 1)
    T32DebugPrintf("OS not initialized... \n");
  return &display;
}

void GStackDisplay (void)
{
  ulong tcbptr = argvalue[0];
  //GetContext(tcbptr);
  //pass in tcbptrs to display function names and addresses:
}

/********************************************************************/

/************************ TASK.SYSINFO ******************************/
info_cmd_init_t CmdProcessParser (void)
{
    argvalue[0] = T32ParseValue("<process>");
    return ProcessInit;
}

cmd_display_t* ProcessInit (void)
{
    static const char header[] = "Process Display      ";
    static cmd_display_t display = {WINDOW, 0, 60, header, ProcessDisplay};

    static cmd_display_t errdisp =
    {
            MESSAGE, 0, 0,
            "Sorry: Couldn't get process list", 0
    };

    if (OS_Initialized() != 1)
        T32DebugPrintf("OS not initialized... \n");

   if (Process_Initialized() != 1)
      return &errdisp;

  return &display;

}

void ProcessDisplay(void)
{
     //displaying available resources:
     //1. threads (tcbs) for this asid
     //2. futexes
     //3.
     //4.

     displayTCB();

}

void displayTCB(void)
{
    ulong tcbptr ,context_size, index=0;
    union phv_t phv;
    unsigned int asid, asid_in, ssr;

    //char *process_name;
    //const char symbol[32]="\\Global\\_Aldata";
    //char full_name[32];

    asid_in = argvalue[0];
    context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));

    /* read first space pointer */
    if (!island_mode){

        tcbptr = par_tcblist;
    #if 0
        //as a test, try displaying name:
        ssr = DreadLong(tcbptr + CONTEXT_ssrelr + 4);
        asid = ASID_FROM_SSR(ssr);
        process_name = NameFromAsid(asid);
        //append to path:
        memcpy(full_name, process_name, 32);
        strcat(full_name, symbol);
        //strlcat (full_name, symbol, sizeof(full_name));
        T32Printf("full_name: %s\n", full_name);

        //end test
    #endif
        do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            ssr = DreadLong(tcbptr + CONTEXT_ssrelr + 4);
        asid = ASID_FROM_SSR(ssr);
        //display state:

        if((phv.info.valid == 1 ) && (asid_in == asid))  //and asid match
            {
                 //tldisplayline (tcbptr);
             T32Printf ("tcb: %x, asid %x\n", tcbptr, asid);
            }
            tcbptr = tcbptr + context_size;
            index++;
       } while(index < max_threads_ddr);
    }

}

/************************ TASK.PGT ************************************/
info_cmd_init_t PgtParser (void)
{
    argvalue[0] = T32ParseValue ("<task>");
    return PgtInit;
}

cmd_display_t* PgtInit(void)
{
  static const char *header[] = {"VPage    ", "PPage    ", "Size     ", "RWX    ", "Global User ", "Cacheability                      ","Raw      ", 0};
  static cmd_display_t display = {WINDOW2, 0, 160, (char *)header, PgtDisplay};
  static cmd_display_t displayone = {WINDOW, 0, 160, header, PgtDisplayOne};

  if (OS_Initialized () != 1)
          T32DebugPrintf("OS not initialized.. ...\n");

 if (argvalue[0])
    return &displayone;
 else
    return &display;   //kernel pagetable: superset of all others
}

//assuming only one ASID, second arg = 0
//this function takes vaddr and returns appropriate pg.table entry to display

u32_t QURT_getR ( u32_t pg)
{
    union pgent_t pge;
    pge.raw = pg;
    if (pge.info.perm & 0x1)
        return 1;
    else
        return 0;
}

u32_t QURT_getW
(
 /* Pagetable entry */
 u32_t        pg
)
{
    union pgent_t pge;
    pge.raw = pg;
    if (pge.info.perm & 0x2)
        return 1;
    else
        return 0;
}

u32_t QURT_getGlobal (u32_t pg)
{
    union pgent_t pge;
    pge.raw = pg;
    if (pge.info.global &0x1)
        return 1;
    else
        return 0;
}

u32_t QURT_getUser (u32_t pg)
{
    union pgent_t pge;
    pge.raw = pg;
    if(pge.info.usr &0x1)
        return 1;
   else
        return 0;
}

u32_t QURT_getX
(
 /* Pagetable entry */
 u32_t        pg
)
{
    union pgent_t pge;
    pge.raw = pg;
    if (pge.info.perm & 0x4)
        return 1;
    else
        return 0;
}

unsigned int ct0(unsigned int d){
    unsigned int i;
    for( i = 0; i < 32; i++ ){
         if( (d & (1<<i)) != 0 ){
              return i;
          }
    }
    return 32;
}

unsigned int clrbit(unsigned int d, unsigned int bit){
    return (d & (~(1<<bit)));
}

pgsize_t QURT_getSize
(
 /* Pagetable entry */
 u32_t        pg
)
{
    union pgent_t pge;
    pge.raw = pg;
    //return (1<<QURT_hw_pgshifts[pge.info.size]);//v2-v3
    return (pgsize_t)(1 << (ct0((unsigned int)pg)));
}

u32_t QURT_getPhysAddr( u32_t pg)
{
   //DEBUG_MSG("pg.info.phys_addr: %x\n", pg.info.phys_addr);
   return ( (clrbit( (unsigned int)pg, (unsigned int)(ct0( (unsigned int)pg & 0x7f)) ) >> 1)& 0xfffff);
}

unsigned int QURT_getNextIndex ( u32_t pg)
{
    union pgent_t pge;
    pgsize_t size;
    pge.raw = pg;
    //DEBUG_MSG("from getIndex: size %d\n", pge.info.size);
    /* Convert TLB page size into multiples of 4K pages */
    size = QURT_getSize(pg);
    switch(size)
    {
        case size_4k:
            return 1;
        case size_16k:
            return 4;
        case size_64k:
            return 16;
        case size_256k:
            return 64;
        case size_1m:
            return 256;
        case size_4m:
            return 1; // must be L1 page table index
        case size_16m:
            return 4; // must be L1 page table index
        default:
            DEBUG_MSG("Fatal: invalid page size\n");
            break;
    }
    /* Should never reach here, default size */
    return 1;
}

char * QURT_printCacheFields (u32_t pg)
{
    union pgent_t pge;
    pge.raw = pg;
    /* Convert cache fields into text */
    switch(pge.info.cache)
    {
        case 0:
            return "Cachable, write-back, non-shared, non-L2-cacheable";
        case 1:
            return "Cachable, write-through, non-shared, non-L2-cacheable";
        case 2:
            return "RESERVED";
        case 3:
            return "RESERVED";
        case 4:
            return "Device-type";
        case 5:
            return "Cachable, write-through, non-shared, L2-cacheable";
        case 6:
            return "Uncached, shared";
        case 7:
            return "Cacheable, write-back, non-shared, L2-cacheable";
        case 8:
            return "Cacheable, write-back";
        case 9:
            return "Cacheable, write-through";
        case 10:
            return "Cacheable, write-back";
        case 11:
            return "Cacheable, write-through";
        default:
            return "Unknown";
    }
    return "Unknown";
}

void PrintOnePagetable(unsigned int space_addr)
{

   unsigned int virt_addr, patch, phys_addr;
   size4u_t paddr, result;
   size4u_t offset;
   size4u_t pgSizeStruct, l2_tbladdr;
   pgsize_t size;

    union pgent_t pg;

   // char l1tbl [1024 * 4];
   // char l2tbl [1024 * 4];

    int index, index1;
    offset = 0;
    index = 0;
    index1 = 0;

   //DEBUG_MSG("spaceaddr %x\n", space_addr);
// #if 0
    //printf("sqiao: pdir at first level is %x\n",pdir);

   DreadBuffer(l1tbl, space_addr, 1024 * 4);

    /* Walk through the top-level entries in the address space */

    for(index = 0; index < 1024; index+=offset)
    {
       // T32Printf("entering the loop: index: %x\n", index);
        /* Read One entry */
        // sq 0407 pgent_t entry((pgent_t*)(pdir+index));
        //pgent_t entry(this, pdir+index*CLASS_PGENT_T_SIZE);
 // #if 0
        //T32Printf("&l1tbl[index] = %8x\n", T32Endian(*(u32_t *)&l1tbl[index*4]));
    //#if 0
        pg.raw = T32Endian(*(u32_t *)&l1tbl[index*4]);
        size = QURT_getSize(pg.raw);
        phys_addr = QURT_getPhysAddr(pg.raw);
    //T32Printf ("pg.raw: %x\n", pg.raw);
        //T32Printf("phys addr of this page: %x\n", phys_addr);
        //T32Printf("size of this page: %x size_4k %x\n", size, size_4k);
        /* If the entry is valid and has 2nd level page table */

        if (size >= size_4m && size < size_invalid) //it's a valid L1 page table entry
        {
            //T32Printf("printing large page! size %x, physaddr: %x\n", size, phys_addr);
           // T32Printf("pg.raw: %8x, pg info.perm: %x\n", pg.raw, pg.info.perm);
            /* Print Mapping */
            virt_addr = index << 22;
            T32Printf("0x%5x    ",(virt_addr>>12));
            T32DisplaySeparator();
            T32Printf("0x%5x    ",  phys_addr);
            T32DisplaySeparator();
            T32Printf("0x%x    ", QURT_pgsize_encode_to_size[ct0(pg.raw)]);
            T32DisplaySeparator();

            /* Format the output */
            T32Printf("  %d  ", QURT_getR( pg.raw));
            T32Printf("%d  %d  ", QURT_getW( pg.raw), QURT_getX( pg.raw));
            T32DisplaySeparator();

            T32Printf("%d  ", QURT_getGlobal(pg.raw));
            T32Printf("%d   ", QURT_getUser(pg.raw));
            T32DisplaySeparator();

            T32Printf("%s ", QURT_printCacheFields( pg.raw));
            T32DisplaySeparator();
            T32Printf("%x       \n", pg.raw);

            offset = QURT_getNextIndex( pg.raw);
            //T32Printf("offset: %x\n", offset);
        }
    else if ((pg.raw != INVALID_PAGE))
        {
            // T32Printf("in the 4k loop \n");
            // T32Printf("pg.info.physaddr: %8x\n", pg.info.phys_addr);
            /* ... initialize size to 12 i.e 4K page */
            //l2_tbladdr = phys_addr;
            l2_tbladdr = pg.raw;  //address of l2pgt is l1entry if not actual entry of >4Mb
            //T32Printf("l2_tbladdr: %x\n", l2_tbladdr);
            DreadBuffer(l2tbl, l2_tbladdr, 1024 * 4);

            /* Walk through the pagetable entries based on pagesize */
              //change upper bound back to 1024 after debugging
            for(index1 = 0; index1 < 1024 ; index1+=offset)
            {
                /* Virtual address to be converted ( Base + Offset ) */
                virt_addr= (index<<22) + (index1<<12);

                pg.raw = T32Endian(*(u32_t *)&l2tbl[index1*4]);
                size = QURT_getSize(pg.raw);
                phys_addr = QURT_getPhysAddr(pg.raw);

                //T32Printf("4k pg.raw: %x\n", pg.raw);
                if ((size < size_4m) && (pg.raw != INVALID_PAGE))
                {
                //T32Printf("4k pg.raw: %x\n", pg.raw);
                    /* Print Mapping */
                    T32Printf("0x%5x    ",(virt_addr>>12));
                    T32DisplaySeparator();
                    T32Printf("0x%5x    ",phys_addr);
                    T32DisplaySeparator();
                    T32Printf("0x%x     ", QURT_pgsize_encode_to_size[ct0(pg.raw)]);
                    T32DisplaySeparator();

                    /* Format the output */
                    if (size < size_1m)
                    {
                        T32Printf("   %d  ", QURT_getR( pg.raw));
                    }
                    else
                    {
                        T32Printf("  %d  ", QURT_getR( pg.raw));
                    }

                    T32Printf("%d  %d   ", QURT_getW(pg.raw), QURT_getX( pg.raw));
                    T32DisplaySeparator();
                    T32Printf("%d  ", QURT_getGlobal(pg.raw));
                    T32Printf("%d   ", QURT_getUser(pg.raw));
                    T32DisplaySeparator();
                    T32Printf("%s      ", QURT_printCacheFields( pg.raw));
                    T32DisplaySeparator();
                    T32Printf("%x       \n", pg.raw);

            offset = QURT_getNextIndex( pg.raw);

                }//end if
                else offset = 1; // Invalid entry, L2 page table default page size in 4k

            }//end for
            offset = 1; // L1 page table default page size in 4m

        }//endif size4k
        else
        {
            // invalid L1 entry
            offset = 1;
            //T32Printf("invalid entry???\n");
        }
   // #endif
    }
//#endif
    T32Printf("\n");

}

void PgtDisplayOne (void)
{
   size4u_t current_space;
   unsigned int asid = argvalue[0];
   T32Printf ("pagetable of asid %x\n", asid);
   //add error check for allowable asids

   if( (current_space = (size4u_t)DgetSymbolAddress(SPACE_LIST)) == (size4u_t)-1)
        T32Printf("error in current space!\n");
   current_space = DreadLong(current_space + 4*asid);
   //T32Printf("VPage     PPage     Size      RWX      Global   User   Cacheability         \n");
   //if (T32IsLineDisplayed(1))
       PrintOnePagetable(current_space);
   //else
   //    T32Printf("\n");
}

void PgtDisplay(void)
{
   size4u_t current_space;
   if( (current_space = (size4u_t)DgetSymbolAddress(SPACE_LIST)) == (size4u_t)-1)
     T32Printf("error in current space!\n");
   current_space = DreadLong(current_space);

   //T32Printf("current space: %x\n", current_space);
   //T32Printf("VPage     PPage     Size      RWX      Global   User  Cacheability              \n");
   //if (T32IsLineDisplayed(1))
        PrintOnePagetable(current_space);
   //else
   //     T32Printf("\n");

}
/**********************************************************************/

/*********************  Command for Task Test  *********************/

/* Parser: returns initialization routine */

info_cmd_init_t TestParser (void)
{
  return TestInit;
}

/* Initialization of test */

cmd_display_t* TestInit (void)
{
  static const char header[] = "argument         value     content   symbol";
  static cmd_display_t display = {WINDOW, 0, 60, header, TestDisplay};

  OS_Initialized ();
  return &display;
}

void TestDisplay (void)
{
//FIXME: not sure how to do if defined:
//#if defined(m68k)
//    const char* cpu = "M68K";
//#endif
//#if defined(QDSP6)
    const char* cpu = "QDSP6";
//#endif

  ulong tmp;

  DISP_MSG (  "magic            %8x   %8x  ", par_magic, DreadLong(par_magic));
//  T32DisplaySymbol (par_magic, 32);
  DISP_MSG ("\nTask List        %8x   %8x  ", par_tcblist, DreadLong(par_tcblist));
//  T32DisplaySymbol (par_tcblist, 32);
  DISP_MSG ("\nSpace List     %8x     %8x  ", par_slist, DreadLong(par_slist));
//  T32DisplaySymbol (par_slist, 32);
  DISP_MSG ("\nMTD Version      %s  %s  ", cpu, __DATE__);
  DISP_MSG ("\nQURT_OSAM_VERSION  %s  %s  ", OSAM_QURT_VERSION, __DATE__);
  DISP_MSG ("\nQ6ZPATH %s ", q6zip_path);

  T32CopyFromGlobal ((byte *)&q6zip_on, sizeof(byte));
  T32CopyFromGlobal ((byte *)&q6zip_rw, sizeof(byte));
  DISP_MSG ("\nq6zip_rx %d ", q6zip_on);
  DISP_MSG ("\nq6zip_rw %d ", q6zip_rw);
  DISP_MSG ("\n");
}

/********************************** TASK.OPTION *********************************/
/* Purpose of this command is to pass arguments to the extension,
 * should not depend on os initialzation, can be called righ after load, even with .cmm;
 * enumeration type determines a set that OSAM recognizes, set can be extended;
 * current set: Q6VER and Q6ZPATH, format: task.Option <enum type> <string>
 * usage: task.Option Q6ZPATH OS.ENV("PATH_TOQ6ZIP") or task.Option Q6ZPATH "X:\abc"
 *
 */
info_cmd_init_t CmdOption(void)
{
    static T32ParseElement envvars[] = {{"Q6VER", 1}, {"Q6ZPATH", 2} , {0,0}};
    argvalue[0] = T32ParseEnumeration (envvars, "<envvar>");
    argvalue[1] = T32ParseValueOrString(argbuffer, 200, "<string>");
    return OptionInit;
}

cmd_display_t* OptionInit(void)
{
    static const char header[] = "argument         value                ";
    static cmd_display_t display = {WINDOW, 0, 60, header, OptionDisplay};

    //static cmd_display_t display = {MESSAGE, 0, 0, 0, OptionDisplay};
    static cmd_display_t noarg = {MESSAGE, 0, 0, "no args", 0};
    if(argvalue[0]) return &display;
    else    return &noarg;
}

void OptionDisplay(void)
{
    switch (argvalue[0])
    {
        case 1:
            if(argbuffer[0])
                T32Printf("Q6VERSION %s\n",argbuffer);
            break;
        case 2:
            if (argbuffer[0])
            {
                T32Printf("Q6ZPATH %s\n", argbuffer);
                //memcpy((void *)q6zip_path, (const void*)argbuffer, 100);
                memcpy ((void *)q6zip_path, (const void *)argbuffer, 199);
                q6zip_path[199]=0;
                T32CopyToGlobal((uint8*)q6zip_path, sizeof(byte)*199);
            }
            //T32Printf(" whats in path %s\n", q6zip_path);
            break;
    }//end switch
}

/************************************* TASK.RQZ *****************************************************/
/* This hidden command allows resetting certain q6zip osam state variables; this
 *  command should never be exposed to the user, following can be reset:
 *  q6zip_on (text)
 *  q6zip_rw (rw)
 *  load_flag[idx]
 *  load_flag_rx[idx]
 *  Use-cases where this command can be handy include scenario where you expect a page
 *  to be decompressed but something goes wrong with decompression and while the page is
 *  loaded, contents are 0 (since osam does not error check file contents of decompressedx.bin),
 *  in which case you would want to rerun decompressor after you fix python or some external issue
 *  that caused "bad" contents to be loaded. Other option is to reload osam, but this command
 *  eliminates the need for that, i.e. this is convenient way to trigger failed decompression;
 *  Prior to resetting q6zip_on and q6zip_rw, make sure you close opended data window, otherwise
 *  you will be caught in infinite decompression loop if you have this command current on your screen
 *  at the same time as you have data window open, effectively you may incur conflicting state;
 *
 *  only for experinced advanced users debugging q6zip osam issues, usage:
 *  task.rqz qrx 0/1
 *  task.rqz qrw 0/1
 *  task.rqz lfrx 0xc  -> will clear the load flag.. note: idx specified in HEX!
 *  FIXME: perhaps do a toggle on the load_flag arrays... might be more useful
 *  DO NOT USE THE COMMAND IF IN DOUBT WHAT IT DOES!
 *
 */
info_cmd_init_t CmdResetQ6Z(void)
{
    static T32ParseElement types[] = {{"QRX", 1}, {"QRW", 2} , {"LFRX", 3}, {"LFRW", 4}, {0,0}};
    argvalue[0] = T32ParseEnumeration (types, "<type>");
    argvalue[1] = T32ParseValue("<value>");
    return ResetQ6ZInit;
}

cmd_display_t* ResetQ6ZInit(void)
{
    static const char header[] = "changing q6zip state                  ";
    static cmd_display_t display = {WINDOW, 0, 60, header, ResetQ6ZDisplay};
    static cmd_display_t noarg = {MESSAGE, 0, 0, "no args", 0};
    if(argvalue[0]) return &display;
    else    return &noarg;
}

void ResetQ6ZDisplay(void)
{
    int value;
    value = argvalue[1];
    if (value >= 4096) return;
    switch (argvalue[0])
    {
        case 1:
            T32Printf("setting q6zip_on to %d\n",value );
            T32CopyFromGlobal((byte *)&q6zip_on, sizeof(byte));
            q6zip_on = value;
            T32CopyToGlobal((byte *)&q6zip_on, sizeof(byte));
            break;
       case 2:
            T32Printf("setting q6zip_rw to %d\n",value );
            T32CopyFromGlobal((byte *)&q6zip_rw, sizeof(byte));
            q6zip_rw = value;
            T32CopyToGlobal((byte *)&q6zip_rw, sizeof(byte));
            break;
      case 3:
           T32Printf("setting loadflag_rx[%d] to 0\n",value );
           T32CopyFromGlobal((byte*)(&load_flag_rx[value]), sizeof(byte));
           load_flag_rx[value] = 0;
           T32CopyToGlobal((byte*)(&load_flag_rx[value]), sizeof(byte));
           break;
      case 4:
           T32Printf("setting loadflag_rw[%d] to 0\n",value );
           T32CopyFromGlobal((byte*)(&load_flag[value]), sizeof(byte));
           load_flag[value] = 0;
           T32CopyToGlobal((byte*)(&load_flag[value]), sizeof(byte));
           break;
    }//end switch
}

/****************************************************************************************/

ulong ThreadNo(ulong myself_global)
{
    return myself_global;
}

int isRunning (size4u_t tcb_ptr)
{
    union phv_t phv;
    phv.raw = DreadLong(tcb_ptr + CONTEXT_prio);

    //DEBUG_MSG("hthread = %x in RUNNING state check\n", phv.info.hthread);
    // if (phv.info.hthread <= 5 && phv.info.hthread >= 0)
    if (phv.info.hthread <= 5)
        return 1;

    return 0;
}

int isWaiting (size4u_t tcb_ptr)
{
    unsigned int futex_ptr, pending;

    futex_ptr = DreadLong(tcb_ptr + CONTEXT_futex_ptr);
    pending = DreadLong(tcb_ptr + CONTEXT_pending);

    if (futex_ptr == 0 && pending == 0)
        return 0;

    return 1;
}

int isSuspended (size4u_t tcb_ptr)
{
    unsigned int tmp, error_list, off_users_err,ent, err_info, users_err, off_entry_array, off_err_tcb, len_entry;
    unsigned int i = 0;

    /* suspended threads are tracked here:
     * QURT_error_info.users_errors.entry[count].error_tcb
     */

    error_list = DgetSymbolAddress("QURT_error_info");
    err_info = T32SymbolTypeLink("struct QURT_error");
    off_users_err = T32SymbolTypeOffsetGet(".users_errors");

    if (error_list == 0)
    {
        T32DebugPrintf("no QURT_error_info \n");
        return 0;
    }
    T32SymbolTypeLink("struct QURT_users_errors");
    off_entry_array = T32SymbolTypeOffsetGet(".entry");

    T32SymbolTypeLink("struct QURT_user_error_entry");
    off_err_tcb = T32SymbolTypeOffsetGet(".error_tcb");
    len_entry = T32SymbolTypeSizeGet("struct QURT_user_error_entry");

    tmp = DreadLong(error_list + off_users_err + off_entry_array + off_err_tcb);
    while (tmp != 0)
    {
        if (tmp == tcb_ptr)
            return 1;
        i++;
        tmp = DreadLong(error_list + off_users_err + off_entry_array + off_err_tcb + len_entry*i);
    }

    return 0;
}

void SlDisplay (void)
{

    /* Virtual Address of the tcb */
    unsigned int tcbptr, idle_counter, index=0, ssr, i;
    unsigned int asid_index[MAX_PDS]={0}, cur_asid;
    union phv_t phv;
    ulong context_size;
    unsigned long long pcycles_start, pcycles, pcycles_hi;

    context_size = DreadLong(DgetSymbolAddress(CONTEXT_SIZE));
    if (!island_mode)
    {
        tcbptr = par_tcblist;
        clearAsidArray();
        //walk through thread list
        do {
            phv.raw = DreadLong(tcbptr + CONTEXT_prio);
            if (phv.info.valid == 1)
            {
                /*Get the SSR from thread context and Read asid */
                ssr = DreadLong(tcbptr + CONTEXT_ssrelr + 4);
                cur_asid = ASID_FROM_SSR(ssr);
                if (cur_asid >= 8)
                {
                    DISP_MSG("FATAL error in OSAM library: don't support more than 8 address spaces\n");
                    return;
                };
                asid_info[cur_asid][asid_index[cur_asid]] = tcbptr;
                asid_index[cur_asid]++;
            }
            tcbptr = tcbptr + context_size;
            index++;
        } while(index < max_threads_ddr);

    //walk through space list
    for (index = 0; index < MAX_PDS; index++)
    {
        if ((tcbptr = asid_info[index][0]) != 0)
        {
            DISP_MSG ("%d     {", index);
            i = 1;
            /* Walk through the thread list and print the thread info */
            do
            {
                /* display task name */
                T32EventDef ("v.v %%hex %%tree.open (QURTK_thread_context *)%x", tcbptr);
                T32Printf("%s ",MagicToName(tcbptr));
                tcbptr = asid_info[index][i++];

            } while(tcbptr && i < max_threads_ddr);
            T32Printf ("} Thread Counts: 0x%x\n", i-1);
        }
    }
    }
    DISP_MSG("\n");
    idle_counter = DgetSymbolAddress(IDLE_COUNTER);
    for (index = 0; index < MAX_HTHREADS; index++)
    {
        pcycles_start = DreadLong(idle_counter + index*16);
        pcycles = DreadLong(idle_counter + index*16 + 8);
        if (pcycles_start && !pcycles)
        {
            DISP_MSG("HW thread %d is still in inital idle state\n",index);
        }
        else
        {
            pcycles_hi = DreadLong(idle_counter + index*16 + 12);
            pcycles = ((pcycles_hi<<32) + pcycles);
            DISP_MSG("HW thread %d idle pcycles: %16x\n",index, pcycles);
        }
    }
    DISP_MSG("\n");
}
