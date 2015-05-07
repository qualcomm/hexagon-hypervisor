/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Programmer.
    (C) Copyright Lauterbach GmbH, Germany
    System Interface Definition.
    
    Version: 25.07.2013  HAP  EDK4.0 - refactoring
*/

/* basic type defines */
typedef unsigned char  uchar;
typedef unsigned char  byte;
typedef unsigned char  uint8;
typedef   signed char  sbyte;
typedef   signed char  sint8;

typedef unsigned short ushort;
typedef unsigned short uint16;
typedef   signed short sint16;

typedef unsigned long  uint;
typedef unsigned long  ulong;
typedef unsigned long  uint32;
typedef   signed long  sint32;

typedef unsigned long long uint64;
typedef   signed long long sint64;

/* semantic types */
typedef uint32 addr32_t;
typedef uint64 addr64_t;
typedef uint32 magic32_t;
typedef uint64 magic64_t;

/* shorthand */
typedef uint8  U8;
typedef uint16 U16;
typedef uint32 U32;
typedef uint64 U64;

#define EXT_ERROR   0xFFFFFFFF
#define EXT_EOL     0xFFFFFFFF

enum {  /*  GetMode 'supermagic' Definitions*/
        GETMOD_ALL  = 0x00000000,
        GETMOD_LIB  = 0xFFFFFFFE,
        GETMOD_PROC = 0xFFFFFFFF
};

enum {  /* Parameter Types */  
        PAR_NONE          =  0, // no parameter
        PAR_HEX           =  3, // hexadecimal number
        PAR_STRING        =  7, // string in quotes
        PAR_STRING_DIRECT = 23  // string without interpretation
};

enum {  /* Breakpoint Definitions */
        BP_P        = 0x0001,   // Program
        BP_H        = 0x0002,   // HLL
        BP_S        = 0x0004,   // Spot
        BP_R        = 0x0008,   // Read
        BP_W        = 0x0010,   // Write
        BP_A        = 0x0020,   // Alpha
        BP_B        = 0x0040,   // Bravo
        BP_C        = 0x0080,   // Charly
        /* only for Siemens C166 Bondout family */
        BP_C166_OAW = 0x0100    // Operand address write
};

enum {  /* Memory Access Definitions */
        MEM_ACCESS_DEFAULT      =  0,   // default access 
        MEM_ACCESS_DATA         =  0,   // D:  data access
        MEM_ACCESS_ABS_DATA     =  1,   // AD: absolute data
        MEM_ACCESS_PROGRAM      =  8,   // P:  program access
        MEM_ACCESS_ABS_PROGRAM  =  9,   // AP: absolute program
        /* only for Intel 386/486 family */
        MEM_ACCESS_REAL_DATA    =  2,   // RD: real data
        MEM_ACCESS_PROT_DATA    =  3,   // PD: protected data
        MEM_ACCESS_REAL_PROGRAM = 10,   // RP: real program
        MEM_ACCESS_PROT_PROGRAM = 11,   // PP: protected prog
        /* only for PowerPC family */
        MEM_ACCESS_SPR          = 16,   // SPR registers
        MEM_ACCESS_DCR          = 17    // DCR registers (4xx)
};

enum {  /* Analyzer Item Definitions */
        ANA_ACCESS         =  0,   // bus access type
        ANA_ADDRESS        =  1,   // record address
        ANA_WIDTH          =  2,   // bus access width in bytes
        ANA_DATA           =  3,   // data of this cycle (byte, word or long)
        ANA_DATA32         =  4,   // record data, 32 bit (one or more cycles)
        ANA_DATA16         =  5,   // record data, 16 bit (one or two cycles)
        ANA_MARKER         =  7,   // marker information (0..3 -> A..D)
        /* only for Siemens C166 Bondout family */
        ANA_C166_IBVALID   = 16,   // internal bus valid
        ANA_C166_IBOPDATA  = 17,   // internal bus operand data
        ANA_C166_IBOPADDRW = 18,   // internal bus operand address write
        /* Analyzer Access Types */
        ANA_ACCESS_READ    =  1,   // bus access type: read
        ANA_ACCESS_WRITE   =  2    // bus access type: write
};

enum {  /* Task States in Analyzer Taskstate Check */
        TS_UNDEFINED       = 0,   // unknown state
        TS_RUNNING         = 1,   // task is currently running
        TS_READY           = 2,   // task is ready to run
        TS_WAITING         = 3,   // task waits for something
        TS_SUSPENDED       = 4    // task is suspended
};

enum {  /* Debugger States from T32GetStatus() */
        SYSTEM_DOWN        = 0,   // debugger system is down
        SYSTEM_HALTED      = 1,   // debugger is up, CPU is down
        SYSTEM_STOPPED     = 2,   // target is stopped (e.g. on breakpoint)
        SYSTEM_RUNNING     = 3    // target is running
};

enum {  /* Display Attributes */
        ATTR_DEFAULT       = 0x00,
        ATTR_ERROR         = 0x78,
        ATTR_GREY          = 0x7E,
        ATTR_GREY_UL       = 0x7A,
        ATTR_BOLD          = 0x92
};

enum {  /* IDs for System Info Query */
        SYSINFO_LICENSESN   = 0,   // license serial number
        SYSINFO_IPADDR      = 1,   // Host IP address
        SYSINFO_MACADDR     = 2,   // Host MAC address
        SYSINFO_CURRENTDATE = 3,   // Current date
        SYSINFO_CPUFAMILY   = 4,   // cpu family name
        SYSINFO_CPU         = 5,   // cpu name
        SYSINFO_SYSDIR      = 6,   // TRACE32 system directory
        SYSINFO_EXTFILE     = 7,   // path and file name of extension
        SYSINFO_CORENAME    = 8,   // core name
        SYSINFO_EXTDIR      = 9,    // ext directory
        SYSINFO_BUILDNUMBER = 10   // TRACE32 build number
};

enum {  /* IDs for object types in T32Set/GetCurrent */
        CURRENT_CORE        = 1,   // current core in multicore environments
        CURRENT_TASK        = 2    // current task
};

enum {  /* IDs and errors for T32DefineExtMemoryAccess */
        T32EXTMEMORYACCESS_READ     =  1,   // read access
        T32EXTMEMORYACCESS_WRITE    =  2,   // write access
        T32EXTMEMORYACCESS_ERRMODE  = -1,   // unknown mode
        T32EXTMEMORYACCESS_ERRACCESS= -2,   // error while accessing
        T32EXTMEMORYACCESS_OK       = 0     // no error
};

/* Size definitions for T32DefineTaskList and T32ReadTaskList */
#define TASK_NUMBER_MAX  3072     // maximum number of tasks
#define TASK_NAMELEN_SZ  64       // max size of task names

typedef enum { /* Display Types */
        WINDOW   = 1,       // display new window
        MESSAGE  = 2,       // display a message in the message line
        WINDOW2  = 3        // display new window with scalable columns
} T32DisplayType;

typedef const struct { /* Display Definition for Command Extensions */
        T32DisplayType type;          // display type
        unsigned char  height, width; // window sizes
        const    char *headline;      // window:  headline text / message: static message text
        void     (*display) (void);   // dynamic display function
} T32CmdDisplayDef;

typedef struct { /* Check Result Definition for Analyzer List */
        uint32 lines;       // number of lines to display
        uint32 value;       // extra value to pass
} T32ALCheckResult;

typedef const struct { /* Display Definition for Analyzer List */
        T32ALCheckResult* (*check)   (void);                 // check lines function
        void              (*display)(T32ALCheckResult *res); // display function
} T32ALDisplayDef;

typedef struct { /* Check Structure Definition for Analyzer Statistics */
        struct {
          uint32   ignoresize;      // size of ignore area
          addr32_t ignoreaddr;      // start address of ignore area
          addr32_t checkaddr1;      // 1st address to check if preview
          addr32_t checkaddr2;      // 2nd address to check if preview
        } preview;
        magic32_t (*check)(void);   // analyzer statistic check routine
} T32ASCheckDef;

typedef struct { /* Check Result Structure for Analyzer Task State */
  magic32_t magic;     // task magic number
  uint32    state;     // detected task state
} T32ATCheckResult;

/* Type Definition for return parameter of Parser Routine.
   Parser routine returns pointer to initialization routine,
   Initialization routine returns display definition.
*/
typedef T32CmdDisplayDef* (*T32CmdInitRtn) (void);

/* Type Definition for return parameter of Ana Taskstate Init.
   Initialization routine returns pointer to check routine,
   Check routine returns Check Result.
*/
typedef T32ATCheckResult* (*T32ATCheckRtn) (void);

/* New Command Definition */
typedef const struct {
        char          *longname;         // command long form
        char          *shortname;        // command short form
        char          *softkey;          // softkey representation
        char          *help;             // help specification
        T32CmdInitRtn (*parser)(void);   // Parser routine
} T32CmdDef;

/* New Function Definition */
typedef const struct {
  char   *name;                         // function name
  uint32  result_type;                  // type of function result
  uint32  arg1_type;                    // type of first argument
  uint32  arg2_type;                    // type of second argument
  char   *help;                         // help specification
  uint32 (*function)(uint32, uint32);   // Function calculation routine
} T32FctDef;

typedef const struct {
  char   *name;                         // function name
  uint32  result_type;                  // type of function result
  uint32  arg1_type;                    // type of first argument
  uint32  arg2_type;                    // type of second argument
  char   *help;                         // help specification
  uint64 (*function)(uint64, uint64);   // Function calculation routine
} T32FctDef64;

/* Analyzer List Definition */
typedef const struct {
  char             *help;               // help specification
  T32ALDisplayDef* (*init)(void);       // Initialization routine
} T32AnaListDef;

/* Analyzer Statistics Definition */
typedef const struct {
  char           *help;                 // help specification
  T32ASCheckDef* (*init)(void);         // Initialization routine
} T32AnaStatDef;

/* Analyzer Task State Definition */
typedef const struct {
  char           *help;                 // help specification
  T32ATCheckRtn  (*init)(void);         // Initialization routine
} T32AnaTaskstateDef;

/* Task Stack Calculation Definition */
typedef struct {
  magic32_t magic;   // task magic
  addr32_t  bottom;  // Bottom (high address) of stack
  addr32_t  top;     // Top    (low  address) of stack
  addr32_t  sp;      // Stack pointer reported by OS
} T32StackDef;       // this is an OLD definition, new follows

typedef struct {
  magic64_t magic;   // task magic
  addr64_t  bottom;  // Bottom (high address) of stack
  addr64_t  top;     // Top    (low  address) of stack
  addr64_t  sp;      // Stack pointer reported by OS
} T32StackDef64;       // this is an OLD definition, new follows

typedef struct {
  magic32_t magic;   // task magic
  addr32_t  low;     // low address of stack
  addr32_t  high;    // high address of stack
  addr32_t  sp;      // Stack pointer reported by OS
} T32StackExResult;

typedef T32StackExResult * (*T32StackExInitRtn)(magic32_t magic, uint32 type);

typedef const struct {
  T32StackExInitRtn (*init)(void);     // Initialization routine
  uint32              patternsize;     // length of fill pattern
  const byte         *pattern;         // pointer to fill pattern
  uint32          * (*reserved)(void); // reserved for future use
} T32StackExDef;

/* Context Calculation Definition */
typedef struct {
  const char *name;           // register name
  addr32_t    address;        // address in memory
  uint32      size;           // size in bytes, or zero if address is the value
} T32ContextDef;

typedef struct {
  const char *name;           // register name
  addr64_t    address;        // address in memory
  uint32      size;           // size in bytes, or zero if address is the value
} T32ContextDef64;

/* Magic Information Definition */
typedef struct {
  addr32_t address;           // Address in memory
  uint32   size;              // size in bytes
  uint32   page;              // MMU/CPU/Banking page
  uint32   flags;             // access flags
} T32MagicDef;

typedef struct {
  addr64_t address;           // Address in memory
  uint32   size;              // size in bytes
  uint32   page;              // MMU/CPU/Banking page
  uint32   flags;             // access flags
} T32MagicDef64;

/* Memory Modification Request Definition */
typedef struct {
  uint8    action;            // in:      action to take
  uint8    length;            // in:      length of modification
  uint8    pagedout;          // in/out:  page is not in memory
  uint8    reserved;
  uint32   spaceid;           // in:      space id of address
  addr32_t address;           // in:      address to change
  uint8    newdata[4];        // in:      modification data
  uint8    origdata[4];       // out:     original data (valid if pagedout == 0)
} T32MemModReqInfo;

typedef const struct {
  sint32 (*getStatus)  (void); // get status of table
  sint32 (*readTable)  (void); // read table from target
  sint32 (*writeTable) (void); // write table to target
  T32MemModReqInfo  *table;    // table with modification requests
  uint32        maxEntries;    // max num of requests
} T32MemModReqDef;

/* Type Definition for parsing predefined key items */
typedef struct {
  char   *key;                // keyword to parse
  uint32  value;              // value to return
} T32ParseElement;

/* Type Definition for window buttons */
typedef struct {
  char  *label;               // label of button
  void  (*handler) (void);    // button press handler
} T32WindowButton;

/* Type Definition for module information */
typedef struct {
  magic32_t magic;            // magic value (id) of module
  uint32    type;             // type of module (not used by T32)
  addr32_t  codeaddr;         // code address of module
  uint32    codesize;         // code size of module
  addr32_t  dataaddr;         // data address of module
  uint32    datasize;         // data size of module
  uint32    spaceid;          // space id of module; 0 if kernel, ~0 if none
  char     *name;             // name of module
} T32ModuleInfo;

typedef struct {
  magic64_t   magic;          // magic value (id) of module */
  uint64    type;             // type of module (not used by T32)
  addr64_t  codeaddr;         // code address of module
  uint64    codesize;         // code size of module
  addr64_t  dataaddr;         // data address of module
  uint64    datasize;         // data size of module
  uint32    spaceid;          // space id of module; 0 if kernel, ~0 if none
  char     *name;             // name of module
} T32ModuleInfo64;

/* Type Definition for module definition */
typedef const struct {
  uint32*           (*list)(uint32 supermagic);            // module list function
  T32ModuleInfo*    (*info)(magic32_t magic, uint32 type); // module info function
} T32ModuleDef;

typedef const struct {
  uint64*           (*list)(uint64 supermagic);            // module list function
  T32ModuleInfo64*  (*info)(magic64_t magic, uint64 type); // module info function
} T32ModuleDef64;

/* Type Definition for relocation information */
typedef struct {
  char*     name;       // section name
  addr32_t  address;    // section address
} T32RelocInfo;

typedef struct {
  char*     name;       // section name
  addr64_t  address;    // section address
} T32RelocInfo64;

/* Type Definition for window sorting */
typedef struct T32SortItemStruct {
    uint64 magic;
    char*  item;
    uint32 itemsize;
} T32SortItem;

/* Type definitions for handles */
typedef uint32 T32AddressHandle;    // handle for address object
typedef uint32 T32BufferHandle;     // handle for buffer object

/* Prototypes of System Interface Routines ************************/

/* interface to TRACE32 and other APIs */
void       T32DefineCommand    (T32CmdDef   *cmddef);
void       T32DefineFunction   (T32FctDef   *fundef);
void       T32DefineFunction64 (T32FctDef64 *fundef);
uint32     T32EvaluationGet    (void);
void       T32EvaluationSet    (uint32 value);
void       T32Execute          (char *format, ...);    // varargs
uint32     T32GetCurrent       (uint32 objtype);            // objtype - see CURRENT_*
uint32     T32SetCurrent       (uint32 objtype, uint32 no); // objtype - see CURRENT_*
sint32     T32GetSystemInfo    (char *info, uint32 id);
void       T32DebugPrintf      (char *format, ...);    // varargs
void       T32FunctionError    (char *errortext);
void       T32Warning          (char *format, ...);    // varargs

/* extension instance memory control */
void       T32CopyFromGlobal   (uint8 *address, uint32 size);
void       T32CopyToGlobal     (uint8 *address, uint32 size);

/* handle access functions */
// Address handle
uint32      T32GetAHAddress32   (T32AddressHandle ahdl);
uint32      T32GetAHSpaceId     (T32AddressHandle ahdl);
uint32      T32GetBHSize        (T32BufferHandle bhdl);
uint32      T32SetBHSize        (T32BufferHandle bhdl, uint32 size);
uint32      T32GetBHContent     (T32BufferHandle bhdl, uint32 offset, byte* buffer, uint32 size);
uint32      T32SetBHContent     (T32BufferHandle bhdl, uint32 offset, byte* buffer, uint32 size);

/* parsing extension command and window arguments and options */
sint32     T32ArgumentGet      (uint32 argidx);
sint64     T32ArgumentGet64    (uint32 argidx);
uint32     T32ParseAddress     (char *dest, uint32 maxlen, char *softkey);
uint32     T32ParseEnumeration (T32ParseElement *keys, char *softkey);
uint32     T32ParseString      (char *dest, uint32 maxlen, char *softkey);
uint32     T32ParseValue       (char *softkey);
uint32     T32ParseValueOrString (char *dest, uint32 maxlen, char *softkey);

uint64     T32ParseValue64     (char *softkey);
uint64     T32ParseValueOrString64 (char *dest, uint32 maxlen, char *softkey);

/* display functions */
uint32     T32DisplayAscii     (byte  *buffer, uint32 length);
void       T32DisplayAttribute (uint32 attr);
void       T32DisplayDec3      (uint32 data);
void       T32DisplayDec5      (uint32 data);
void       T32DisplayHex2      (uint32 data);
void       T32DisplayHex4      (uint32 data);
void       T32DisplayHex8      (uint32 data);
void       T32DisplayLf        (sint32 number);
void       T32DisplaySeparator (void);     // display '|' and skip field
void       T32DisplayString    (char  *string);
void       T32DisplayStringFix (char  *string, uint32 length);
uint32     T32IsLineDisplayed  (uint32 lines);
void       T32Printf           (char  *format, ...);   // varargs
void       T32Sprintf          (char  *dest, char *format, ...); // varargs
/* see below: T32DisplayStringTarget/64(), T32DisplaySymbol/64(), T32DisplayTaskName() */

/* data field control elements */
void       T32ButtonDef        (char *command, uint32 value);
void       T32EventDef         (char *command, uint32 value);
void       T32EventStringDef   (char *command, uint32 value, char *string);
void       T32EventStringDef64 (char *command, uint64 value, char *string);
void       T32MenuDef          (char *searchname);
byte       T32TreeDef          (uint32 id);

void       T32EventDef64       (char *command, uint64 value);

/* window control */
void       T32WindowSetButtons      (T32WindowButton *buttons);
void       T32WindowSetCloseHandler (void (*handler) (void));
void       T32WindowSetHeader       (const char *header);
void       T32WindowSetMaxVSize     (uint32 size);
void       T32WindowSetWorkHandler  (void (*handler) (void));
void       T32WindowGetLimit        (uint32 * right, uint32 * left, uint32 * down, uint32 * up);

/* window sorting utilities */
void       T32SortedListInit   (T32SortItem* (*func) (uint64));
uint32     T32SortedListSize   (void);
uint64     T32SortedListMagic  (uint32 pos);
uint32     T32SortedListColumn (void);
uint32     T32SortedListSet    (uint32 key, uint32 column);

/* utility functions */
uint16     T32EndianWord       (uint16 data);
uint32     T32Endian           (uint32 data);
uint32     T32Extract          (byte *buffer, uint32 size);

uint64     T32Extract64        (byte *buffer, uint32 size);

/* symbol database operations */
sint32     T32GetEnumString        (char *dest, uint32 maxlen, char *enumtype, uint32 enumvalue);
void       T32GetSymbol            (char *name, addr32_t addr, uint32 length);
void       T32GetSymbolPath        (char *name, addr32_t addr, uint32 length);
void       T32DisplaySymbol        (addr32_t addr, uint32 length);
addr32_t   T32SymbolAddrGet        (char *symbolname);
uint32     T32SymbolBrowse         (char *symbolname, char *symboltypename, void (callback)(addr32_t addr, char *name), char *name, uint32 maxlen);
uint32     T32SymbolBrowse64       (char *symbolname, char *symboltypename, void (callback)(addr64_t addr, char *name), char *name, uint32 maxlen);
addr32_t   T32SymbolSectionAddrGet (char *sectionname);
sint32     T32SymbolSectionSpaceIdGet (char *sectionname);
sint32     T32SymbolSizeGet        (char *symbolname);
sint32     T32SymbolTypeLink       (char *symbolname);
sint32     T32SymbolTypeOffsetGet  (char *symbolname);
sint32     T32SymbolTypeSizeGet    (char *symbolname);

void       T32DisplaySymbol64      (addr64_t addr, uint32 length);
addr64_t   T32SymbolAddrGet64      (char *symbolname);

/* Target Access  *************************************************************** */

void        T32DefineExtMemoryAccess    (sint32 (*func)(T32AddressHandle ahdl, T32BufferHandle bhdl, uint32 mode));

/* target memory and register operations - read */
void       T32DisplayStringTarget  (addr32_t addr, uint32 length);
uint32     T32ReadBuffer           (byte  *dest, addr32_t source, uint32 length);
byte       T32ReadByte             (addr32_t addr);
uint16     T32ReadWord             (addr32_t addr);
uint32     T32ReadLong             (addr32_t addr);
uint32     T32ReadMemory           (addr32_t addr, uint32 size);
uint32     T32ReadRegister         (uint32 no);

void       T32DisplayStringTarget64 (addr64_t addr, uint32 length);
uint32     T32ReadBuffer64         (byte  *dest, addr64_t source, uint32 length);
uint8      T32ReadByte64           (addr64_t addr);
uint16     T32ReadWord64           (addr64_t addr);
uint32     T32ReadLong64           (addr64_t addr);
uint64     T32ReadQuad64           (addr64_t addr);
uint64     T32ReadMemory64         (addr64_t addr, uint32 size);
uint64     T32ReadRegister64       (uint32 no);

/* target memory and register operations - write */
uint32     T32WriteBuffer          (byte  *source, addr32_t dest, uint32 length);
sint32     T32WriteByte            (addr32_t addr, byte value);
sint32     T32WriteWord            (addr32_t addr, uint16 value);
sint32     T32WriteLong            (addr32_t addr, uint32 value);
sint32     T32WriteRegister        (uint32 no, uint32 value);

/* target memory and register operations - check */
sint32     T32CheckMemory          (addr32_t addr, uint32 length);
sint32     T32CheckMemory64        (addr64_t addr, uint32 length);

/* target memory access classes */
void       T32SetMemoryAccessClassId (uint32 access);
void       T32SetMemoryAccessClassString (char *access);
void       T32SetMemoryAccessSpaceId (uint32 spaceId);

/* target execution control */
uint32     T32GetStatus        (void);
void       T32TargetCall       (addr32_t addr);

/* Breakpoint Access ************************************************************ */
void       T32BreakpointSet    (addr32_t addr, uint32 length, uint32 bps);
void       T32BreakpointDelete (addr32_t addr, uint32 length, uint32 bps);

/* Analyzer Functions - Trace Access ******************************************** */
void       T32DefineAnalyzerList       (T32AnaListDef *analist);
void       T32DefineAnalyzerStat       (T32AnaStatDef *anastat);
void       T32DefineAnalyzerTaskstate  (T32AnaTaskstateDef *anataskst);
           
uint32     T32AnaGetCurrItem    (uint32 item);
void       T32AnaSkipRecord     (uint32 dummy);
uint32     T32AnaTrackTaskState (uint32 offset, uint32 size, T32ATCheckResult *pstate);

/* Operating System Awareness *************************************************** */
void       T32DefineCurrentTask    (magic32_t (*func)(void));
void       T32DefineGetContext     (T32ContextDef* (*func)(magic32_t magic));
void       T32DefineGetMagic       (T32MagicDef* (*func)(void));
void       T32DefineGetModules     (T32ModuleDef* (*func)(void));
void       T32DefineGetSpaceId     (T32MagicDef* (*func)(void));
void       T32DefineTaskId         (uint32 (*func)(magic32_t magic));
void       T32DefineTaskList       (magic32_t * (*func)(void));
void       T32DefineTaskName       (char* (*func)(magic32_t magic));
void       T32DefineTaskSpaceId    (uint32 (*func)(magic32_t magic));
void       T32DefineTaskStack      (T32StackDef* (*func)(magic32_t magic));
void       T32DefineTaskStackEx    (T32StackExDef* stackdef);
void       T32DefineTaskTraceId    (uint32 (*func)(magic32_t magic));
void       T32DefineTranslationList (uint32* (*func)(void));
void       T32DisplayTaskName      (magic32_t magic, uint32 length);
uint32     T32IsTaskMagic          (magic32_t magic);
addr32_t   T32LogicalToPhysical    (addr32_t addr);
magic32_t  T32ParseTask            (void);
addr32_t   T32PointerToAddress     (uint32 ptr, uint32 size);
void       T32SetDefaultMagics     (magic32_t other, magic32_t kernel);
uint32     T32TaskNameGet          (char *name, magic32_t magic);

void       T32DefineGetContext64   (T32ContextDef64* (*func)(magic64_t magic));
void       T32DefineGetMagic64     (T32MagicDef64*   (*func)(void));
void       T32DefineGetModules64   (T32ModuleDef64*  (*func)(void));
void       T32DefineTaskId64       (uint32  (*func)(magic64_t magic));
void       T32DefineTaskList64     (magic64_t * (*func)(void));
void       T32DefineTaskName64     (char*   (*func)(magic64_t magic));
void       T32DefineTaskSpaceId64  (uint32  (*func)(magic64_t magic));
void       T32DefineTaskStack64    (T32StackDef64* (*func)(magic64_t magic));
addr64_t   T32LogicalToPhysical64  (addr64_t addr);
uint32     T32TaskNameGet64        (char *name, magic64_t magic);

/* Virtual Machine Awareness **************************************************** */
typedef struct {
    magic32_t  magic;
    uint32     pid;
    uint32     spaceid;
    uint32     flags;    ///< [31:16]core, [4]virtualvalid, [3]virtualid, [2]stopped, [1]current, [0]exists
    char       name[TASK_NAMELEN_SZ];
} T32TaskEntry;

uint32     T32ReadTaskList(T32TaskEntry *tlst, uint32 tcnt);
uint32     T32TaskStackGetCaller(magic32_t magic, uint32 level, char *name, addr32_t *address);
uint32     T32TaskStackGetPointer(magic32_t magic, uint32 level, char *varname, addr32_t *address);

/* compatibility definitions for older EDK versions ***************************** */
#define T32MemoryAccess         T32SetMemoryAccessClassId
#define T32ReadReg              T32ReadRegister
#define T32ReadReg64            T32ReadRegister64
#define T32WriteReg             T32WriteRegister

///+//WORK/IN/PROGRESS/////////WORK/IN/PROGRESS/////////WORK/IN/PROGRESS/////////WORK/IN/PROGRESS////// 

/* Operating System Awareness *** MAY CHANGE WITHOUT NOTICE *** EXPERIMENTAL **** */
void T32DefineGetCurrentGuestId(uint32 (*func)(void));
void T32DefineGetCurrentMachineId(uint32 (*func)(void));
void T32DefineGetMachineContext(T32ContextDef* (*func)(uint32 machid));
void T32DefineMemoryModificationRequest(T32MemModReqDef* memModReqDef);
void T32DefineRelocInfo(T32RelocInfo*  (*func)(uint32, char*, uint32));
void T32DefineSpacePageList(uint32* (*func)(magic32_t magic));
void T32DefineTaskHasStack(uint32  (*func)(magic32_t magic));
void T32DefineTaskHwThread(uint32  (*func)(magic32_t magic));
void T32DefineTaskTTB(uint32  (*func)(magic32_t magic));
void T32DefineTaskTTB64(uint64  (*func)(magic64_t magic));
void T32DefineTaskVMABase(uint32  (*func)(magic32_t magic));
void T32DefineRelocInfo64(T32RelocInfo64*  (*func)(uint32, char*, uint32));
void T32SetVersionInfo(char **versionstrings);
uint32 T32GetActualHwThread(void);   // deprecated - use T32GetCurrent(CORE)
uint32 T32SelectHwThread(uint32 no); // deprecated - use T32SetCurrent(CORE,x)

/* Virtual Machine Awareness **** MAY CHANGE WITHOUT NOTICE *** EXPERIMENTAL **** */
void T32DefineVMSingleStep(uint32 (*func)(uint32));
void T32RegisterGetRunState(void(*CBGetRunState)(uint32 state));
void T32RegisterCallback(void(*CBFunc)(), uint32 type); /* CBFunc signature depends on type */
enum T32VmMapperAttributes_e {
    VM_MAP_ATTRIBUTE_VM       = 0x0001, VM_MAP_ATTRIBUTE_VMSTOP = 0x0002,
    VM_MAP_ATTRIBUTE_BYTECODE = 0x0004, VM_MAP_ATTRIBUTE_IP     = 0x0010,
    VM_MAP_ATTRIBUTE_SP       = 0x0020, VM_MAP_ATTRIBUTE_CP     = 0x0040,
    VM_MAP_ATTRIBUTE_LP       = 0x0080, VM_MAP_ATTRIBUTE_FP     = 0x0100,
    VM_MAP_ATTRIBUTE_IPBASE   = 0x0200, VM_MAP_ATTRIBUTE_MB     = 0x0400,
    VM_MAP_ATTRIBUTE_CB       = 0x0800
};
typedef struct {
    uint32 attrmask,     attr; 
    uint32 from_address, from_address_hi, from_access;
    uint32 to_address,   to_address_hi,   to_access;
} T32VmMapEntry;
uint32 T32VmAddToMap(T32VmMapEntry *mlist, uint32 mcnt);

///-//WORK/IN/PROGRESS/////////WORK/IN/PROGRESS/////////WORK/IN/PROGRESS/////////WORK/IN/PROGRESS////// 

