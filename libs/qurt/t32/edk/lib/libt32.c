/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** TRACE32 Extension Programmer.
    (C) Copyright Lauterbach GmbH, Germany
    System Interface Implementation.
    
    Version: 25.07.2013  HAP  EDK4.0 - refactoring
*/

/* include System Interface Definition */
#include "t32ext.h"

/* Implementation.

   r2 = service number for trap call interface
   r3 = first  argument
   r4 = second argument
   
   result: in r3/r4
 */

#ifdef T32EXT101
void T32DefineCommand(T32CmdDef *cmddef)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x101");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT102
void T32DefineFunction(T32FctDef *fundef)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x102");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT103
void T32DefineAnalyzerList(T32AnaListDef *analist)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x103");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT104
void T32DefineAnalyzerStat(T32AnaStatDef *anastat)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x104");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT105
void T32DefineAnalyzerTaskstate(T32AnaTaskstateDef *anataskst)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x105");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT106
void T32DefineTaskId(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x106");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT107
void T32DefineTaskName(char*(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x107");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT108
void T32DefineTaskList(uint32*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x108");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT109
void T32DefineTaskStack(T32StackDef*(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x109");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10a
void T32DefineTaskStackEx(T32StackExDef* stackdef)
{
  /* r3 = ptr to data structure */
  asm("li   %r2,0x10a");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10b
void T32DefineCurrentTask(magic32_t(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x10b");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10c
void T32DefineGetContext(T32ContextDef*(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x10c");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10d
void T32DefineTranslationList(uint32*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x10d");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10e
void T32DefineGetMagic(T32MagicDef*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x10e");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT10f
void T32DefineSpacePageList(uint32*(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x10f");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT110
void T32DefineGetSpaceId(T32MagicDef*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x110");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT111
void T32DefineTaskSpaceId(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x111");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT112
void T32DefineGetModules(T32ModuleDef*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x112");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT113
void T32DefineTaskTraceId(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x113");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT114
void T32DefineTaskVMABase(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x114");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT115
void T32DefineTaskHasStack(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x115");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT116
void T32DefineMemoryModificationRequest(T32MemModReqDef* memModReqDef)
{
  /* r3 = ptr to data structure */
  asm("li   %r2,0x116");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT117
void T32DefineTaskHwThread(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x117");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT118
void T32DefineTaskTTB(uint32(*func)(magic32_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x118");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT119
void T32DefineGetModules64(T32ModuleDef64*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x119");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11a
void T32DefineFunction64(T32FctDef64* fctdef)
{
  /* r3 = ptr to structure */
  asm("li   %r2,0x11a");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11b
void T32DefineRelocInfo(T32RelocInfo*(*func)(uint32, char*, uint32))
{ /* EXPERIMENTAL */
  /* r3 = ptr to function */
  asm("li   %r2,0x11b");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11c
void T32DefineGetMagic64(T32MagicDef64*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x11c");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11d
void T32DefineRelocInfo64(T32RelocInfo64*(*func)(uint32, char*, uint32))
{ /* EXPERIMENTAL */
  /* r3 = ptr to function */
  asm("li   %r2,0x11d");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11e
void T32DefineGetContext64(T32ContextDef64*(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x11e");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT11f
void T32DefineGetCurrentMachineId(uint32(*func)(void))
{ /* EXPERIMENTAL */
  /* r3 = ptr to function */
  asm("li   %r2,0x11f");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT120
void T32DefineTaskList64(uint64*(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x120");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT121
void T32DefineTaskName64(char*(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x121");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT122
void T32DefineTaskId64(uint32(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x122");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT123
void T32DefineTaskSpaceId64(uint32(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x123");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT124
void T32DefineTaskTTB64(uint64(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x124");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT125
void T32DefineTaskStack64(T32StackDef64*(*func)(magic64_t magic))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x125");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT126
void T32DefineGetMachineContext(T32ContextDef*(*func)(uint32 machid))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x126");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT127
void T32DefineGetCurrentGuestId(uint32(*func)(void))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x127");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT128
void T32DefineExtMemoryAccess (sint32 (*func)(T32AddressHandle ahdl, T32BufferHandle bhdl, uint32 mode))
{
  /* r3 = ptr to function */
  asm("li   %r2,0x128");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT140
void T32DefineVMSingleStep(uint32(*func)(uint32))
{   /* EXPERIMENTAL */
    asm("li   %r2,0x140");
    asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT141
void T32RegisterGetRunState(void(*CBGetRunState)(uint32 state))
{   /* EXPERIMENTAL */
    asm("li   %r2,0x141");
    asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT160
uint32 T32GetAHAddress32 (T32AddressHandle ahdl)
{
    asm("li   %r2,0x160");  // trap id
    asm("mr   %r4,%r3");    // address handle
    asm("li   %r3,0x01");   // GetAddress32
    asm("tweq %r2,%r2");
}
uint32 T32GetAHSpaceId (T32AddressHandle ahdl)
{
    asm("li   %r2,0x160");  // trap id
    asm("mr   %r4,%r3");    // address handle
    asm("li   %r3,0x02");   // GetSpaceId
    asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT161
uint32 T32GetBHContent (T32BufferHandle bhdl, uint32 offset, byte* buffer, uint32 size)
{
    asm("li   %r2,0x161");  // trap id
    asm("mr   %r7,%r6");    // size
    asm("mr   %r6,%r5");    // buffer address
    asm("mr   %r5,%r4");    // offset
    asm("mr   %r4,%r3");    // buffer handle
    asm("li   %r3,0x01");   // GetContent
    asm("tweq %r2,%r2");
}
uint32 T32SetBHContent (T32BufferHandle bhdl, uint32 offset, byte* buffer, uint32 size)
{
    asm("li   %r2,0x161");  // trap id
    asm("mr   %r7,%r6");    // size
    asm("mr   %r6,%r5");    // buffer address
    asm("mr   %r5,%r4");    // offset
    asm("mr   %r4,%r3");    // buffer handle
    asm("li   %r3,0x02");   // SetContent
    asm("tweq %r2,%r2");
}
uint32 T32GetBHSize (T32BufferHandle bhdl)
{
    asm("li   %r2,0x161");  // trap id
    asm("mr   %r4,%r3");    // buffer handle
    asm("li   %r3,0x03");   // GetSize
    asm("tweq %r2,%r2");
}
uint32 T32SetBHSize (T32BufferHandle bhdl, uint32 size)
{
    asm("li   %r2,0x161");  // trap id
    asm("mr   %r5,%r4");    // size
    asm("mr   %r4,%r3");    // buffer handle
    asm("li   %r3,0x04");   // SetSize
    asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT180
void T32RegisterCallback(void(*func)(), uint32 type)
{   /* EXPERIMENTAL */
    asm("li   %r2,0x180");
    asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT201
addr32_t T32SymbolAddrGet(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x201");
  asm("tweq %r2,%r2");
  /* r3 = symbol address */
}
#endif

#ifdef T32EXT202
sint32 T32SymbolSizeGet(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x202");
  asm("tweq %r2,%r2");
  /* r3 = symbol address */
}
#endif

#ifdef T32EXT203
sint32 T32SymbolTypeSizeGet(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x203");
  asm("tweq %r2,%r2");
  /* r3 = type size */
}
#endif

#ifdef T32EXT204
sint32 T32SymbolTypeOffsetGet(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x204");
  asm("tweq %r2,%r2");
  /* r3 = type offset */
}
#endif

#ifdef T32EXT205
uint32 T32SymbolBrowse(char* symbolname, char *symboltypename, void(callback)(uint32 addr, char *name), char *name, uint32 maxlen)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x205");
  asm("tweq %r2,%r2");
  /* r3 = number of symbols */
}
#endif

#ifdef T32EXT206
sint32 T32SymbolTypeLink(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x206");
  asm("tweq %r2,%r2");
  /* r3 = type offset */
}
#endif

#ifdef T32EXT207
addr32_t T32SymbolSectionAddrGet(char* sectionname)
{
  /* r3 = ptr to sectionname */
  asm("li   %r2,0x207");
  asm("tweq %r2,%r2");
  /* r3 = section address */
}
#endif

#ifdef T32EXT208
sint32 T32SymbolSectionSpaceIdGet(char* sectionname)
{
  /* r3 = ptr to sectionname */
  asm("li   %r2,0x208");
  asm("tweq %r2,%r2");
  /* r3 = section space id */
}
#endif

#ifdef T32EXT209
sint32 T32GetEnumString(char *dest, uint32 maxlen, char *enumtype, uint32 enumvalue)
{
  /* r3 = ptr to destination */
  /* r4 = max length of destination */
  /* r5 = enumeration type */
  /* r6 = requested enumeration value */
  asm("li   %r2,0x209");
  asm("tweq %r2,%r2");
  /* r3 = string length */
}
#endif

#ifdef T32EXT20a
addr64_t T32SymbolAddrGet64(char* symbolname)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x20a");
  asm("tweq %r2,%r2");
  /* r3, r4 = symbol address */
}
#endif

#ifdef T32EXT20b
uint32 T32SymbolBrowse64(char* symbolname, char *symboltypename, void(callback)(uint64 addr, char *name), char *name, uint32 maxlen)
{
  /* r3 = ptr to symbolname */
  asm("li   %r2,0x20b");
  asm("tweq %r2,%r2");
  /* r3 = number of symbols */
}
#endif

#ifdef T32EXT301
byte T32ReadByte(addr32_t addr)
{
  /* r3 = target address */
  asm("li   %r2,0x301");
  asm("tweq %r2,%r2");
  /* r3 = read byte data */
}
#endif

#ifdef T32EXT302
uint16 T32ReadWord(addr32_t addr)
{
  /*  in: r3 = target address */
  asm("li   %r2,0x302");
  asm("tweq %r2,%r2");
  /* out: r3 = read uint16 data */
}
#endif

#ifdef T32EXT303
uint32 T32ReadLong(addr32_t addr)
{
  /* r3 = target address */
  asm("li   %r2,0x303");
  asm("tweq %r2,%r2");
  /* r3 = read uint32 data */
}
#endif

#ifdef T32EXT304
sint32 T32WriteByte(addr32_t addr, byte value)
{
  /* r3 = target address */
  /* r4 = value to write */
  asm("li   %r2,0x304");
  asm("tweq %r2,%r2");
  /* r3 = exit status */
}
#endif

#ifdef T32EXT305
sint32 T32WriteWord(addr32_t addr, uint16 value)
{
  /* r3 = target address */
  /* r4 = value to write */
  asm("li   %r2,0x305");
  asm("tweq %r2,%r2");
  /* r3 = exit status */
}
#endif

#ifdef T32EXT306
sint32 T32WriteLong(addr32_t addr, uint32 value)
{
  /* r3 = target address */
  /* r4 = value to write */
  asm("li   %r2,0x306");
  asm("tweq %r2,%r2");
  /* r3 = exit status */
}
#endif

#ifdef T32EXT307
uint32 T32ReadMemory(addr32_t addr, uint32 size)
{
  /* r3 = target address */
  /* r4 = size */
  asm("li   %r2,0x307");
  asm("tweq %r2,%r2");
  /* r3 = read byte/word/sint32 data */
}
#endif

#ifdef T32EXT311
uint32 T32ReadBuffer(byte *dest, addr32_t source, uint32 length)
{
  /* r3 = ptr to destination */
  /* r4 = source target address */
  /* r5 = number of bytes to read */
  asm("li   %r2,0x311");
  asm("tweq %r2,%r2");
  /* r3 = number of read bytes */
}
#endif

#ifdef T32EXT312
uint32 T32WriteBuffer(byte *source, addr32_t dest, uint32 length)
{
  /* r3 = ptr to source */
  /* r4 = destination target address */
  /* r5 = number of bytes to write */
  asm("li   %r2,0x312");
  asm("tweq %r2,%r2");
  /* r3 = number of written bytes */
}
#endif

#ifdef T32EXT313
uint32 T32ReadBuffer64(byte *dest, addr64_t source, uint32 length)
{
  /* r3 = ptr to destination */
  /* r5 = source target address MSW */
  /* r6 = source target address LSW */
  /* r7 = number of bytes to read */
  asm("li   %r2,0x313");
  asm("tweq %r2,%r2");
  /* r3 = number of read bytes */
}
#endif

#ifdef T32EXT315
sint32 T32CheckMemory(addr32_t addr, uint32 length)
{
  /* r3 = target address */
  /* r4 = number of bytes to check(read) */
  asm("li   %r2,0x315");
  asm("tweq %r2,%r2");
  /* r3 = error code or 0 if no error */
}
#endif

#ifdef T32EXT316
sint32 T32CheckMemory64(addr64_t addr, uint32 length)
{
  /* r3/r4 = target address */
  /* r5 = number of bytes to check(read) */
  asm("li   %r2,0x316");
  asm("tweq %r2,%r2");
  /* r3 = error code or 0 if no error */
}
#endif

#ifdef T32EXT321
void T32SetMemoryAccessClassId(uint32 access)
{
  /* r3 = access class ID */
  asm("li   %r2,0x321");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT322
void T32SetMemoryAccessClassString(char *access)
{
  /* r3 = ptr to access class string */
  asm("li   %r2,0x322");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT323
void T32SetMemoryAccessSpaceId(uint32 spaceid)
{
  /* r3 = space id */
  asm("li   %r2,0x323");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT331
void T32TargetCall(addr32_t addr)
{
  /* r3 = target address to start */
  asm("li   %r2,0x331");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT340
uint32 T32ReadRegister(uint32 no)
{
  /* r3 = reg offset - see emu.h */
  asm("li   %r2,0x340");
  asm("tweq %r2,%r2");
  /* r3 = read 32bit data */
}
#endif

#ifdef T32EXT341
sint32 T32WriteRegister(uint32 no, uint32 value)
{
  /* r3 = reg offset - see emu.h */
  /* r4 = value to set           */
  asm("li   %r2,0x341");
  asm("tweq %r2,%r2");
  /* r3 = error */
}
#endif

#ifdef T32EXT342
uint64 T32ReadRegister64(uint32 no)
{
  /* r3 = reg offset - see emu.h */
  asm("li   %r2,0x342");
  asm("tweq %r2,%r2");
  /* r3, r4 = read 64bit data */
}
#endif

#ifdef T32EXT351
uint8 T32ReadByte64(addr64_t addr)
{
  /* r3 = target address MSW */
  /* r4 = target address LSW */
  asm("li   %r2,0x351");
  asm("tweq %r2,%r2");
  /* r3 = read byte data */
}
#endif

#ifdef T32EXT352
uint16 T32ReadWord64(addr64_t addr)
{
  /* r3 = target address MSW */
  /* r4 = target address LSW */
  asm("li   %r2,0x352");
  asm("tweq %r2,%r2");
  /* r3 = read word data */
}
#endif

#ifdef T32EXT353
uint32 T32ReadLong64(addr64_t addr)
{
  /* r3 = target address MSW */
  /* r4 = target address LSW */
  asm("li   %r2,0x353");
  asm("tweq %r2,%r2");

  /* r3 = read sint32 data */
}
#endif

#ifdef T32EXT354
uint64 T32ReadQuad64(addr64_t addr)
{
  /* r3 = target address MSW */
  /* r4 = target address LSW */
  asm("li   %r2,0x354");
  asm("tweq %r2,%r2");
  /* r3 = read quad data MSW */
  /* r4 = read quad data LSW */
}
#endif

#ifdef T32EXT355
uint64 T32ReadMemory64(addr64_t addr, uint32 size)
{
  /* r3 = target address MSW */
  /* r4 = target address LSW */
  /* r5 = size */
  asm("li   %r2,0x355");
  asm("tweq %r2,%r2");
  /* r3 = read quad MSW data */
  /* r4 = read byte/word/sint32/quad LSW data */
}
#endif

#ifdef T32EXT360
uint32 T32SelectHwThread(uint32 no)
{ /* DEPRECATED */
  /* r3 = HW thread number */
  asm("li   %r2,0x360");
  asm("tweq %r2,%r2");
  /* r3 = new HW thread set */
}
#endif

#ifdef T32EXT361
uint32 T32GetActualHwThread(void)
{ /* DEPRECATED */
  asm("li   %r2,0x361");
  asm("tweq %r2,%r2");
  /* r3 = actual HW thread */
}
#endif

#ifdef T32EXT362
uint32 T32SetCurrent(uint32 objtype, uint32 no)
{
  /* r4 = object number/magic */
  /* r3 = object type */
  asm("li   %r2,0x362");
  asm("tweq %r2,%r2");
  /* r3 = new object id set */
}
#endif

#ifdef T32EXT363
uint32 T32GetCurrent(uint32 objtype)
{
  /* r3 = object type */
  asm("li   %r2,0x363");
  asm("tweq %r2,%r2");
  /* r3 = current object id */
}
#endif

#ifdef T32EXT400
void T32Printf(char  *format, ...)
{
  /* r3 = ptr to format string */
  asm("li   %r2,0x400");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT401
void T32DisplayString(char *string)
{
  /* r3 = ptr to string */
  asm("li   %r2,0x401");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT402
void T32DisplayLf(sint32 number)
{
  /* r3 = number of line feeds */
  asm("li   %r2,0x402");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT403
void T32DisplayStringFix(char *string, uint32 length)
{
  /* r3 = ptr to string */
  /* r4 = length to display */
  asm("li   %r2,0x403");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT40f
void T32Sprintf(char *dest, char *format, ...)
{
  /* r3 = ptr to destination */
  /* r4 = ptr to format string */
  asm("li   %r2,0x40f");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT411
void T32DisplayHex8(uint32 data)
{
  /* r3 = data to display */
  asm("li   %r2,0x411");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT412
void T32DisplayHex4(uint32 data)
{
  /* r3 = data to display */
  asm("li   %r2,0x412");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT413
void T32DisplayHex2(uint32 data)
{
  /* r3 = data to display */
  asm("li   %r2,0x413");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT421
void T32DisplayDec5(uint32 data)
{
  /* r3 = data to display */
  asm("li   %r2,0x421");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT422
void T32DisplayDec3(uint32 data)
{
  /* r3 = data to display */
  asm("li   %r2,0x422");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT431
void T32DisplaySymbol(addr32_t addr, uint32 length)
{
  /* r3 = symbol address */
  /* r4 = length to display */
  asm("li   %r2,0x431");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT432
void T32DisplayStringTarget(addr32_t addr, uint32 length)
{
  /* r3 = string address */
  /* r4 = length to display */
  asm("li   %r2,0x432");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT433
uint32 T32DisplayAscii(byte *buffer, uint32 length)
{
  /* r3 = buffer address */
  /* r4 = length to display */
  asm("li   %r2,0x433");
  asm("tweq %r2,%r2");
  /* r3 = # of chars displayed */
}
#endif

#ifdef T32EXT434
void T32GetSymbol(char *name, addr32_t addr, uint32 length)
{
  /* r3 = return string */
  /* r4 = symbol address */
  /* r5 = max length */
  asm("li   %r2,0x434");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT435
void T32GetSymbolPath(char *name, addr32_t addr, uint32 length)
{
  /* r3 = return string */
  /* r4 = symbol address */
  /* r5 = max length */
  asm("li   %r2,0x435");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT436
void T32DisplayStringTarget64(addr64_t addr, uint32 length)
{
  /* r3/r4 = string address */
  /* r5 = length to display */
  asm("li   %r2,0x436");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT437
void T32DisplaySymbol64(addr64_t addr, uint32 length)
{
  /* r3, r4 = symbol address */
  /* r5 = length to display */
  asm("li   %r2,0x437");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT441
void T32DisplayTaskName(magic32_t magic, uint32 length)
{
  /* r3 = task magic */
  /* r4 = length to display */
  asm("li   %r2,0x441");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT451
uint32 T32IsLineDisplayed(uint32 lines)
{
  /* r3 = number of lines to check */
  asm("li   %r2,0x451");
  asm("tweq %r2,%r2");
  /* r3 = 1 if lines are displayed, 0 else */
}
#endif

#ifdef T32EXT452
void T32DisplayAttribute(uint32 attr)
{
  /* r3 = display attribute */
  asm("li   %r2,0x452");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT453
void T32DisplaySeparator(void)
{
  asm("li   %r2,0x453");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT454
void T32SortedListInit(T32SortItem* (*func) (uint64))
{
  /* r3 = sort function */
  asm("li   %r2,0x454");    // trap id
  asm("mr   %r4,%r3");      // callback function
  asm("li   %r3,0x01");     // "...Init"
  asm("tweq %r2,%r2");
}

uint32 T32SortedListSize(void)
{
  asm("li   %r2,0x454");    // trap id
  asm("li   %r3,0x02");     // "...Size"
  asm("tweq %r2,%r2");
  /* r3 = size of sorted list */
}

uint64 T32SortedListMagic(uint32 pos)
{
  /* r3 = list position */
  asm("li   %r2,0x454");    // trap id
  asm("mr   %r4,%r3");      // position
  asm("li   %r3,0x03");     // "...Magic"
  asm("tweq %r2,%r2");
  /* r3/r4 = magic at position */
}

uint32 T32SortedListColumn(void)
{
  asm("li   %r2,0x454");    // trap id
  asm("li   %r3,0x04");     // "...Column"
  asm("tweq %r2,%r2");
  /* r3 = column to sort */
}

uint32 T32SortedListSet(uint32 key, uint32 column)
{
  /* r3 = no/up/down, r4 = column */
  asm("li   %r2,0x454");    // trap id
  asm("mr   %r5,%r4");      // column
  asm("mr   %r4,%r3");      // key
  asm("li   %r3,0x05");     // "...Set"
  asm("tweq %r2,%r2");
  /* r3/r4 = magic at position */
}
#endif

#ifdef T32EXT501
sint32 T32ArgumentGet(uint32 argidx)
{
  /* r3 = number of argument to read */
  asm("li   %r2,0x501");
  asm("tweq %r2,%r2");
  /* r3 = passed argument value */
}
#endif

#ifdef T32EXT502
sint64 T32ArgumentGet64(uint32 argidx)
{
  /* r3 = number of argument to read */
  asm("li   %r2,0x502");
  asm("tweq %r2,%r2");
  /* r3/r4 = passed argument value */
}
#endif

#ifdef T32EXT511
uint32 T32ParseValue(char *softkey)
{
  /* r3 = ptr to softkey string */
  asm("li   %r2,0x511");
  asm("tweq %r2,%r2");
  /* r3 = parsed argument value */
}
#endif

#ifdef T32EXT512
uint32 T32ParseString(char *dest, uint32 maxlen, char *softkey)
{
  /* r3 = ptr to destination string */
  /* r4 = max length of parsed string */
  /* r5 = ptr to softkey string */
  asm("li   %r2,0x512");
  asm("tweq %r2,%r2");
  /* r3 = parsed string length */
}
#endif

#ifdef T32EXT513
uint32 T32ParseValueOrString(char *dest, uint32 maxlen, char *softkey)
{
  /* r3 = ptr to destination string */
  /* r4 = max length of parsed string */
  /* r5 = ptr to softkey string */
  asm("li   %r2,0x513");
  asm("tweq %r2,%r2");
  /* r3 = parsed value if value or 0 if string */
}
#endif

#ifdef T32EXT514
uint32 T32ParseEnumeration(T32ParseElement *keys, char *softkey)
{
  /* r3 = ptr to array of keys */
  /* r4 = ptr to softkey string */
  asm("li   %r2,0x514");
  asm("tweq %r2,%r2");
  /* r3 = parsed argument value */
}
#endif

#ifdef T32EXT515
magic32_t T32ParseTask(void)
{
  asm("li   %r2,0x515");
  asm("tweq %r2,%r2");
  /* r3 = parsed task magic */
}
#endif

#ifdef T32EXT516
uint32 T32ParseAddress(char *dest, uint32 maxlen, char *softkey)
{
  /* r3 = ptr to destination string */
  /* r4 = max length of parsed string */
  /* r5 = ptr to softkey string */
  asm("li   %r2,0x516");
  asm("tweq %r2,%r2");
  /* r3 = parsed address offset */
}
#endif

#ifdef T32EXT517
uint64 T32ParseValueOrString64(char *dest, uint32 maxlen, char *softkey)
{
  /* r3 = ptr to destination string */
  /* r4 = max length of parsed string */
  /* r5 = ptr to softkey string */
  asm("li   %r2,0x517");
  asm("tweq %r2,%r2");
  /* r3 = parsed value if value or 0 if string */
}
#endif

#ifdef T32EXT518
uint64 T32ParseValue64(char *softkey)
{
  /* r3 = ptr to softkey string */
  asm("li   %r2,0x518");
  asm("tweq %r2,%r2");
  /* r3 = parsed argument value */
}
#endif

#ifdef T32EXT521
void T32FunctionError(char *errortext)
{
  /* r3 = ptr to error string */
  asm("li   %r2,0x521");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT601
uint32 T32Endian(uint32 data)
{
  /* r3 = data to reformat */
  asm("li   %r2,0x601");
  asm("tweq %r2,%r2");
  /* r3 = formatted value */
}
#endif

#ifdef T32EXT602
uint16 T32EndianWord(uint16 data)
{
  /* r3 = data to reformat */
  asm("li   %r2,0x602");
  asm("tweq %r2,%r2");
  /* r3 = formatted value */
}
#endif

#ifdef T32EXT603
uint32 T32Extract(byte *buffer, uint32 size)
{
  /* r3 = ptr to buffer */
  /* r4 = number of bytes to extract */
  asm("li   %r2,0x603");
  asm("tweq %r2,%r2");
  /* r3 = extracted value */
}
#endif

#ifdef T32EXT604
addr32_t T32PointerToAddress(uint32 ptr, uint32 size)
{
  /* r3 = data to reformat */
  asm("li   %r2,0x604");
  asm("tweq %r2,%r2");
  /* r3 = formatted value */
}
#endif

#ifdef T32EXT605
addr32_t T32LogicalToPhysical(addr32_t addr)
{
  /* r3 = data to reformat */
  asm("li   %r2,0x605");
  asm("tweq %r2,%r2");
  /* r3 = formatted value */
}
#endif

#ifdef T32EXT606
void T32SetDefaultMagics(magic32_t other, magic32_t kernel)
{
  /* r3 = magic of "other"      */
  /* r4 = magic of kernel       */
  asm("li   %r2,0x606");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT607
uint64 T32Extract64(byte *buffer, uint32 size)
{
  /* r3 = ptr to buffer */
  /* r4 = number of bytes to extract */
  asm("li   %r2,0x607");
  asm("tweq %r2,%r2");
  /* r3/r4 = extracted value */
}
#endif

#ifdef T32EXT608
addr64_t T32LogicalToPhysical64(addr64_t addr)
{
  /* r3 = data to reformat */
  asm("li   %r2,0x608");
  asm("tweq %r2,%r2");
  /* r3 = formatted value */
}
#endif

#ifdef T32EXT701
void T32EventDef(char *command, uint32 value)
{
  /* r3 = command string */
  /* r4 = address to use */
  asm("li   %r2,0x701");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT702
void T32ButtonDef(char *command, uint32 value)
{
  /* r3 = command string */
  /* r4 = address to use */
  asm("li   %r2,0x702");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT703
void T32EventStringDef(char *command, uint32 value, char *string)
{
  /* r3 = command string */
  /* r4 = address to use */
  /* r5 = string */
  asm("li   %r2,0x703");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT704
void T32MenuDef(char *searchname)
{
  /* r3 = search string */
  asm("li   %r2,0x704");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT705
byte T32TreeDef(uint32 id)
{
  /* r3 = id of tree */
  asm("li   %r2,0x705");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT706
void T32EventDef64(char *command, uint64 value)
{
  /* r3 = command string */
  /* r4, r5 = address to use */
  asm("li   %r2,0x706");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT707
void T32EventStringDef64(char *command, uint64 value, char *string)
{
  /* r3 = command string */
  /* r4,r5 = address to use */
  /* r6 = string */
  asm("li   %r2,0x707");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT711
void T32WindowSetWorkHandler(void (*handler)(void))
{
  /* r3 = ptr to handler */
  asm("li   %r2,0x711");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT712
void T32WindowSetCloseHandler(void (*handler)(void))
{
  /* r3 = ptr to handler */
  asm("li   %r2,0x712");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT713
void T32WindowSetButtons(T32WindowButton *buttons)
{
  /* r3 = ptr to button definitions */
  asm("li   %r2,0x713");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT714
void T32WindowSetMaxVSize(uint32 size)
{
  /* r3 = vertical size in lines */
  asm("li   %r2,0x714");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT715
void T32WindowSetHeader(const char *header)
{
  /* r3 = pointer to header definition */
  asm("li   %r2,0x715");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT718
void T32WindowGetLimit(uint32 * right, uint32 * left, uint32 * down, uint32 * up)
{
  asm("li   %r2,0x718");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT801
uint32 T32AnaGetCurrItem(uint32 item)
{
  /* r3 = item number */
  asm("li   %r2,0x801");
  asm("tweq %r2,%r2");
  /* r3 = item value */
}
#endif

#ifdef T32EXT802
void T32AnaSkipRecord(uint32 dummy)
{
  asm("li   %r2,0x802");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT901
void T32BreakpointSet(addr32_t addr, uint32 length, uint32 bps)
{
  /* r3 = breakpoint address */
  /* r4 = length of area to set */
  /* r4 = breakpoints to set */
  asm("li   %r2,0x901");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXT902
void T32BreakpointDelete(addr32_t addr, uint32 length, uint32 bps)
{
  /* r3 = breakpoint address */
  /* r4 = length of area to delete */
  /* r4 = breakpoints to delete */
  asm("li   %r2,0x902");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa01
void T32DebugPrintf(char *format, ...)
{
  /* r3 = ptr to format string */
  asm("li   %r2,0xa01");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa02
void T32Warning(char *format, ...)
{
  /* r3 = ptr to format string */
  asm("li   %r2,0xa02");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa11
void T32Execute(char *format, ...)
{
  /* r3 = ptr to format string */
  asm("li   %r2,0xa11");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa12
uint32 T32GetStatus(void)
{
  asm("li   %r2,0xa12");
  asm("tweq %r2,%r2");
  /* r3 = status */
}
#endif

#ifdef T32EXTa13
sint32 T32GetSystemInfo(char *info, uint32 id)
{
  /* r3 = ptr to string space */
  /* r4 = what to get */
  asm("li   %r2,0xa13");
  asm("tweq %r2,%r2");
  /* r3 = 0 if ok */
}
#endif

#ifdef T32EXTa14
void T32SetVersionInfo(char **versionstrings)
{
  /* r3 = ptr to array of version strings */
  asm("li   %r2,0xa14");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa21
void T32EvaluationSet(uint32 value)
{
  /* r3 = value to set */
  asm("li   %r2,0xa21");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa22
uint32 T32EvaluationGet(void)
{
  asm("li   %r2,0xa22");
  asm("tweq %r2,%r2");
  /* r3 = evaluation value */
}
#endif

#ifdef T32EXTa31
void T32CopyToGlobal(uint8 *address, uint32 size)
{
  /* r3 = address to copy */
  /* r4 = size to copy */
  asm("li   %r2,0xa31");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTa32
void T32CopyFromGlobal(uint8 *address, uint32 size)
{
  /* r3 = address to copy */
  /* r4 = size to copy */
  asm("li   %r2,0xa32");
  asm("tweq %r2,%r2");
}
#endif

#ifdef T32EXTb01
uint32 T32TaskNameGet(char *name, magic32_t magic)
{
  /* r3 = ptr to name space */
  /* r4 = task magic */
  asm("li   %r2,0xb01");
  asm("tweq %r2,%r2");
  /* r3 = length of task name */
}
#endif

#ifdef T32EXTb02
uint32 T32IsTaskMagic(magic32_t magic)
{
  /* r3 = task magic to check*/
  asm("li   %r2,0xb02");
  asm("tweq %r2,%r2");
  /* r3 = true if task is magic */
}
#endif

#ifdef T32EXTb03
uint32 T32AnaTrackTaskState(uint32 offset, uint32 size, T32ATCheckResult *pstate)
{
  /* r3 = task magic to check*/
  asm("li   %r2,0xb03");
  asm("tweq %r2,%r2");
  /* r3 = true if analyzer has accessed taskstate */
}
#endif

#ifdef T32EXTb04
uint32 T32TaskNameGet64(char *name, magic64_t magic)
{
  /* r3 = ptr to name space */
  /* r4 = task magic */
  asm("li   %r2,0xb04");
  asm("tweq %r2,%r2");
  /* r3 = length of task name */
}
#endif

/*(+) VM awareness additions */
#ifdef T32EXT1000
uint32 T32ReadTaskList(T32TaskEntry *tlst, uint32 tcnt)
{
  /* r3 = ptr to destination */
  /* r4 = number of entries to read */
  asm("li   %r2,0x1000");
  asm("tweq %r2,%r2");
  /* r3 = number of read entries(if r3 == tcnt+1, then more data is available) */
}
#endif

#ifdef T32EXT1010
uint32 T32TaskStackGetCaller(magic32_t magic, uint32 level, char *name, addr32_t *address)
{
  /* r3 = [in]  task magic */
  /* r4 = [in]  stack level */
  /* r5 = [out] pointer to caller name buffer */
  /* r6 = [out] pointer to caller address */
  asm("li   %r2,0x1010");
  asm("tweq %r2,%r2");
  /* r3 = status */
}
#endif

#ifdef T32EXT1011
uint32 T32TaskStackGetPointer(magic32_t magic, uint32 level, char *varname, addr32_t *address)
{
  /* r3 = [in]  task magic */
  /* r4 = [in]  stack level */
  /* r5 = [in]  pointer to varpath buffer */
  /* r6 = [out] pointer to symbol memory address */
  asm("li   %r2,0x1011");
  asm("tweq %r2,%r2");
  /* r3 = status */
}
#endif

#ifdef T32EXT1020
uint32 T32VmAddToMap(T32VmMapEntry *mlist, uint32 mcnt)
{ /* EXPERIMENTAL */
  /* r3 = ptr to VM map entry list */
  /* r4 = number of entries to read */
  asm("li   %r2,0x1020");
  asm("tweq %r2,%r2");
  /* r3 = status */
}
#endif

/*(-) VM awareness additions */

