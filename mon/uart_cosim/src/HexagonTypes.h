/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*****************************************************************
# Copyright (c) $Date: 2011/09/07 17:15:49 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2011/09/07 17:15:49 $
*****************************************************************/
#ifndef _HEXAGONTYPES_H_
#define _HEXAGONTYPES_H_

typedef unsigned char       HEX_1u_t;
typedef char                HEX_1s_t;
typedef unsigned short int  HEX_2u_t;
typedef short               HEX_2s_t;
typedef unsigned int        HEX_4u_t;
typedef int                 HEX_4s_t;

#ifdef _WIN32
typedef unsigned _int64     HEX_8u_t;
typedef _int64              HEX_8s_t;
#else
typedef unsigned long long int  HEX_8u_t;
typedef long long int           HEX_8s_t;
#endif

typedef unsigned int        HEX_PA_t;  // for now ...

typedef unsigned int        HEX_VA_t;

typedef enum hexagon_statistics_t
{
    HEX_TOTAL_PCYCLES,
    HEX_THREAD_INSTRUCTIONS,
    HEX_THREAD_STALL_CYCLES,
    HEX_POWER_RESET_STATS,
    HEX_POWER_ALL_WAIT_PERCENT,
    HEX_POWER_AVG_ACTIVE_THREADS,
    HEX_POWER_AVG_PKT_DENSITY
} HEX_StatisticsType;

typedef enum hexagon_status_t
{
    HEX_STAT_ERROR = -1,
    HEX_STAT_SUCCESS,
    HEX_STAT_CANNOT_CONFIG,
    HEX_STAT_INVALID_ARGS,
    HEX_STAT_RANGE_ERROR,
    HEX_STAT_FILE_ACCESS_ERROR,
    HEX_STAT_DEVICE_NOT_FOUND,
    HEX_STAT_MEM_ACCESS_ERROR,
    HEX_STAT_CANNOT_TRANSLATE,
    HEX_STAT_NO_ACTIVE_THREADS
} HEXAPI_Status;

typedef enum hexagon_abi_t
{
    HEX_V2_ABI,
    HEX_V3_ABI
} HEXAPI_ABI;

#define HEX_CPU_ID_V2         0x102

#define HEX_CPU_ID_V3L        0x003
#define HEX_CPU_ID_V3M        0x103
#define HEX_CPU_ID_V3C        0x303

#define HEX_CPU_ID_V4M        0x004
#define HEX_CPU_ID_V4C        0x104
#define HEX_CPU_ID_V4L        0x304

#define HEX_CPU_ID_V5H		  0x005		
#define HEX_CPU_ID_V5A		  0x105
#define HEX_CPU_ID_V5L		  0x205

typedef enum hexagon_cpu_t
{
    HEX_CPU_V2        = HEX_CPU_ID_V2,
    HEX_CPU_V3L       = HEX_CPU_ID_V3L,
    HEX_CPU_V3M       = HEX_CPU_ID_V3M,
    HEX_CPU_V3C       = HEX_CPU_ID_V3C,
    HEX_CPU_V3        = HEX_CPU_V3M,
    HEX_CPU_V4M       = HEX_CPU_ID_V4M,
    HEX_CPU_V4C       = HEX_CPU_ID_V4C,
    HEX_CPU_V4L       = HEX_CPU_ID_V4L,
    HEX_CPU_V4        = HEX_CPU_V4M,
	HEX_CPU_V5H		  = HEX_CPU_ID_V5H,
	HEX_CPU_V5A		  = HEX_CPU_ID_V5A,
	HEX_CPU_V5L		  = HEX_CPU_ID_V5L,
	HEX_CPU_V5		  = HEX_CPU_V5H
} HEXAPI_Cpu;

typedef enum hexagon_interval_t
{
    HEX_MILLISEC = 1,
    HEX_MICROSEC,
    HEX_NANOSEC,
    HEX_PICOSEC
} HEXAPI_Interval;

typedef enum hexgon_timing_mode_t
{
    HEX_NOTIMING = 0,
    HEX_TIMING_NODBC,
    HEX_TIMING,          // timing mode with data backed caches
    HEX_TIMING_COHERENCY // timing mode with data-backed caches and coherency checking
} HEXAPI_TimingMode;

typedef enum hexagon_core_state_t
{
    HEX_CORE_SUCCESS = 0,
    HEX_CORE_FINISHED,
    HEX_CORE_RESET,
    HEX_CORE_BREAKPOINT,
    HEX_CORE_ASYNCHRONOUS_BREAK,
    HEX_CORE_ERROR
} HEXAPI_CoreState;

typedef enum hexagon_trace_t
{
    HEX_TRACE_ICACHE = 0,
    HEX_TRACE_DCACHE,
    HEX_TRACE_L2CACHE,
    HEX_TRACE_MEM,
    HEX_TRACE_BUS,
    HEX_TRACE_PC,
    HEX_TRACE_STALL
} HEXAPI_TracingType;

typedef enum null_ptr_t
{
    HEX_NULLPTR_IGNORE = 0,
    HEX_NULLPTR_WARN,
    HEX_NULLPTR_FATAL
} HEXAPI_Nullptr;

typedef enum bus_access_t
{
    HEX_INSTRUCTION_FETCH = 0,
    HEX_DATA_READ,
    HEX_DATA_WRITE,
    HEX_DATA_CASTOUT,
    HEX_DATA_READ_LOCKED,
    HEX_DATA_WRITE_LOCKED,
    HEX_SYNCH,
    HEX_BARRIER,
    HEX_DATA_READ_PREFETCH,
    HEX_INSTRUCTION_PREFETCH,
    HEX_DEBUG_READ,
    HEX_DEBUG_WRITE
} HEXAPI_BusAccessType;

typedef enum verbose_mode_t
{
    HEX_QUIET = 0,
    HEX_NORMAL,
    HEX_VERBOSE,
    HEX_REALLY_VERBOSE
} HEXAPI_VerboseMode;

typedef enum bus_burst_t
{
    HEX_BURST_FIXED = 0,
    HEX_BURST_INCREMENTAL,
    HEX_BURST_WRAPPED
} HEXAPI_BusBurstType;

/* ATTENTION USER: These next 2 register structures contain enumerations for all thread and    */
/*   System registers for all supported cores. This means it is up to the user to take care    */
/*   and only use registers appropriate to the core under use. Functions that take these       */
/*   enumerations as parameter arguments will return error (HEX_STAT_INVALID_ARGS) if the      */
/*   register in question is not supported by the core in use. Users should also be aware      */
/*   that the same register may have different bit definitions for different cores.            */
/*   Please check the Hexagon Architecture references.                                         */

typedef enum thread_register_t
{
    // V2, V3, and V4

    TH_REG_R0,
    TH_REG_R1,
    TH_REG_R2,
    TH_REG_R3,
    TH_REG_R4,
    TH_REG_R5,
    TH_REG_R6,
    TH_REG_R7,
    TH_REG_R8,
    TH_REG_R9,
    TH_REG_R10,
    TH_REG_R11,
    TH_REG_R12,
    TH_REG_R13,
    TH_REG_R14,
    TH_REG_R15,
    TH_REG_R16,
    TH_REG_R17,
    TH_REG_R18,
    TH_REG_R19,
    TH_REG_R20,
    TH_REG_R21,
    TH_REG_R22,
    TH_REG_R23,
    TH_REG_R24,
    TH_REG_R25,
    TH_REG_R26,
    TH_REG_R27,
    TH_REG_R28,
    TH_REG_R29,
    TH_REG_R30,
    TH_REG_R31,
    TH_REG_SA0,
    TH_REG_LC0,
    TH_REG_SA1,
    TH_REG_LC1,
    TH_REG_P3_P0,
    TH_REG_M0,
    TH_REG_M1,
    TH_REG_USR,
    TH_REG_PC,
    TH_REG_UGP,
    TH_REG_GP,
    TH_REG_ELR,
    TH_REG_SSR,
    TH_REG_IMASK,

    // V2 and V3 - Not V4
    TH_REG_SGP,
    TH_REG_BADVA,
    TH_REG_TID,

    // V4 only

    TH_REG_CS0,
    TH_REG_CS1,
    TH_REG_UPCYCLELO,
    TH_REG_UPCYCLEHI,
    TH_REG_UPMUCNT0,
    TH_REG_UPMUCNT1,
    TH_REG_UPMUCNT2,
    TH_REG_UPMUCNT3,
    TH_REG_SGP0,
    TH_REG_SGP1,
    TH_REG_STID,
    TH_REG_BADVA0,
    TH_REG_BADVA1,
    TH_REG_CCR,
    TH_REG_HTID,
    TH_REG_G0,
    TH_REG_G1,
    TH_REG_G2,
    TH_REG_G3

} HEXAPI_TH_REG;

typedef enum global_register_t
{
    // V2, V3, and V4

    G_REG_EVB,
    G_REG_IPEND,
    G_REG_SYSCFG,
    G_REG_MODECTL,
    G_REG_REV,
    G_REG_DIAG,
    G_REG_IAD,
    G_REG_IEL,
    G_REG_IAHL,
    G_REG_PCYCLEHI,
    G_REG_PCYCLELO,
    G_REG_ISDBST,
    G_REG_ISDBCFG0,
    G_REG_ISDBCFG1,
    G_REG_BRKPTPC0,
    G_REG_BRKPTCFG0,
    G_REG_BRKPTPC1,
    G_REG_BRKPTCFG1,
    G_REG_ISDBMBXIN,
    G_REG_ISDBMBXOUT,
    G_REG_ISDBEN,
    G_REG_ISDBGPR,
    G_REG_BRKPTINFO,
    G_REG_ISDBCMD,
    G_REG_ISDBVER,
    G_REG_RGDR,
    G_REG_ACC0,
    G_REG_ACC1,
    G_REG_CHICKEN,
    G_REG_STFINST,
    G_REG_PMUCNT0,
    G_REG_PMUCNT1,
    G_REG_PMUCNT2,
    G_REG_PMUCNT3,
    G_REG_PMUEVTCFG,
    G_REG_PMUCFG,
    G_REG_AVS,

    // V2 and V3 (Not in V4)

    G_REG_TLBHI,
    G_REG_TLBLO,
    G_REG_TLBIDX,

    // V4 only

    G_REG_CFGBASE,
    G_REG_VID

} HEXAPI_G_REG;

typedef enum interrupt_state_t
{
    INT_PIN_DEASSERT = 0,
    INT_PIN_ASSERT
} HEXAPI_InterruptPinState;

typedef enum core_ready_state_t
{
    CORE_NOT_READY = 0,
    CORE_READY    
} HEXAPI_CoreReadyState;

typedef enum transaction_state_t
{
    TRANSACTION_REPLAY = 0,
    TRANSACTION_SUCCESS,
    TRANSACTION_FAIL,
    TRANSACTION_LOCKED
} HEXAPI_TransactionStatus;

typedef enum openmode_t
{
    HEX_MODE_READ,          // "r"
    HEX_MODE_READBINARY,    // "rb"
    HEX_MODE_WRITE,         // "w"
    HEX_MODE_WRITEBINARY,   // "wb"
    HEX_MODE_APPEND,        // "a+"
    HEX_MODE_APPENDBINARY   // "ab+"
} HEXAPI_OpenMode;

typedef HEXAPI_TransactionStatus(*bus_transaction_request_callback)(void*                  /*deviceHandle*/
                                                                   , HEX_PA_t              /*physical address*/
                                                                   , HEX_4u_t              /*lengthInBytes*/
                                                                   , HEX_1u_t *            /*data*/
                                                                   , HEX_4u_t              /*requestID*/
                                                                   , HEXAPI_BusAccessType  /*type*/
                                                                   , HEX_4u_t              /*threadNum*/
                                                                   , HEXAPI_BusBurstType   /*burst*/
                                                                   );

typedef void (*memory_read_callback)(void*      /*deviceHandle*/
                                    , HEX_PA_t  /*physical address*/
                                    , HEX_8u_t  /*value*/
                                    , HEX_4u_t  /*sizeInBytes*/
                                    );

typedef void (*memory_written_callback)(void*      /*deviceHandle*/
                                       , HEX_PA_t  /*physical address*/
                                       , HEX_8u_t  /*value*/
                                       , HEX_4u_t  /*sizeInBytes*/
                                       );

typedef void (*cosim_callback)(void* /*deviceHandle*/);

typedef void (*pc_callback)(void* /*deviceHandle*/);

typedef void (*timed_callback)(void* /*deviceHandle*/);

typedef void (*frequency_change_callback)(void*      /*deviceHandle*/
                                         , HEX_8u_t  /*changed freq*/
                                         );

typedef void (*core_ready_callback)(void * /*deviceHandle*/, HEXAPI_CoreReadyState polarity );

#endif
