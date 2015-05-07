/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef T32_MAP_H
#define T32_MAP_H

#define DEBUG_MSG       T32DebugPrintf

#define info_command_t  T32CmdDef
#define cmd_display_t   T32CmdDisplayDef
#define arch_context_t  T32ContextDef
#define info_cmd_init_t T32CmdInitRtn
#define os_obj_handle_t       T32MagicDef

#define DregisterCommand        T32DefineCommand
#define DregisterTogetCurrTask  T32DefineCurrentTask
#define DregisterTogetDebugName T32DefineTaskName
#define DregisterTogetASID      T32DefineTaskSpaceId
#define DregisterTogetSwtid     T32DefineTaskId
#define DregisterTogetContext   T32DefineGetContext
#define DisplaySeparator        T32DisplaySeparator
#define DgetCurrHwThread        T32GetActualHwThread
#define DregisterTogetTaskList  T32DefineTaskList
#define DregisterTogetTranslationList T32DefineTranslationList
#define DregisterTogetStack     T32DefineTaskStackEx
#define DgetSymbolAddress       T32SymbolAddrGet
#define DregisterTogetTraceId   T32DefineTaskTraceId

#define DISP_MSG(format, ...)  T32Printf(format, ## __VA_ARGS__)

#define DreadBuffer              T32ReadBuffer
#define DreadLong                T32ReadLong
#define DExtract                 T32Extract

static unsigned int qc_mcd_convert_to_register_tbl_offset( unsigned int register_num )
{
    unsigned int tbl_index = register_num ; // default 

    if( register_num <= 29 ) tbl_index = register_num  ;
    else if( register_num <= 31 ) tbl_index = register_num + 2 ;
    else if( register_num <= 36 ) tbl_index = register_num + 3 ;
    else if( register_num <= 43 ) tbl_index = register_num + 2 ;
    else if( register_num == 64 ) tbl_index = register_num - 16 ;
    else if( register_num <= 70 ) tbl_index = register_num - 19 ; 

    return( tbl_index ) ;
}

unsigned long DreadThreadRegister(unsigned int tnum, unsigned int regnum)
{
	ulong value, adjusted_regnum;
    int current_tnum = T32GetActualHwThread();
    if(current_tnum != tnum)
    	T32SelectHwThread(tnum);
    adjusted_regnum = qc_mcd_convert_to_register_tbl_offset(regnum);
    value = T32ReadRegister(adjusted_regnum);
    if(current_tnum != tnum)
    	T32SelectHwThread(current_tnum);
	return value;
}
#endif /* T32_MAP_H */