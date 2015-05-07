/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  ELF to PowerPC Hyperprocessor converter.

    History:
    2014-08-12 DIE  changed version number to 1.1
    2013-06-06 HAP  refactoring
    2010-12-06 DIE  max. ElfDataSize extended from 0x2000 to 0x4000
    2010-07-19 DIE  max. ElfDataSize extended from 0x1000 to 0x2000
    2002-07-01 DIE  Heap Start added (= end of last section)
    
    indent -linux -i4 -ts8 -l132 -cd41 elf2t32.c
*/

#define DBGINFO if(0)printf

#ifndef UNIX
# include <windows.h>
# include <io.h>
# include <memory.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#ifdef UNIX
# define O_BINARY	0
#endif

#define ELF_HEADER_SIZE		52
#define ELF_SECTION_SIZE	48
#define SZ_T32HEADER            0x68
#define SZ_MAX_BUFFER		0x100
#define DEFAULT_TEXT_ADDRESS	0x0
#define DEFAULT_TEXT_SZ_MAX	0x40000
#define DEFAULT_DATA_ADDRESS	0x10000
#define DEFAULT_DATA_SZ_MAX	0x4000

#define OUT_PUTBLOCK(buf,size)		{if(write(OutHandle,buf,size)!=size)OutErrorFatal();}
#define ELF_GETBLOCK(buffer,size)	{if(read(ElfHandle,buffer,size)!=size)ElfErrorFatal();}
#define ELF_SEEK(pos)			{if(lseek(ElfHandle,pos,0)!=pos)ElfErrorFatal();}
#define ELF_TELL()			lseek(ElfHandle,0L,SEEK_CUR)
#define ELF_WORD(A,B) {(A)=(((B)[0])<<8)|(((B)[1]));}
#define ELF_LONG(A,B) {(A)=(((B)[0])<<24)|(((B)[1])<<16)|(((B)[2])<<8)|(((B)[3]));}

typedef unsigned char uchar;
typedef unsigned long uint32;
typedef unsigned short uint16;

static int OutHandle;
static int ElfHandle;

static uint16 ElfProgramMax;		/* number of Program Headers in file */
static uint16 ElfProgramEntrySize;	/* length of one Program Header */
static uint32 ElfProgramFptr;		/* file pointer to section data */
static uint16 ElfSectionIndex;		/* current ELF section */
static uint16 ElfSectionMax;		/* number of sections in file */
static uint32 ElfSectionAddress;	/* current address of section */
static uint32 ElfSectionSize;		/* current size    of sectionn */
static uint32 ElfSectionFptr;		/* file pointer to section data */
static uint32 ElfStartAddress;
static uint32 ElfDataAddress;
static uint32 ElfTextSize;
static uint32 ElfDataSize;
static uint32 ElfHeaderFptr;		/* file pointer to next header */
static uchar *ElfStringTable;
static uint32 ElfStringFptr;		/* file pointer to strings in file */
static uint32 ElfStringSize;		/* length of ELF string table */
static uint32 ElfStringMax;
static uint16 ElfSectionEntrySize;	/* length of one section header */
static uint16 ElfSectionStrings;	/* section string table index */
static uchar *ElfSectionStringTable;	/* section names */
static uint32 ElfSectionMaxAddress;	/* highest address occupied */

enum e_elf_section_header_type { SHT_NULL, SHT_PROGBITS, SHT_SYMTAB, SHT_STRTAB };
enum e_elf_program_type { PT_NULL, PT_LOAD, PT_DYNAMIC, PT_INTERP, PT_NOTE, PT_SHLIB, PT_PHDR };

static uchar buffer[SZ_MAX_BUFFER];
static uchar *textbuffer;
static uchar *databuffer;

/* prototypes */
static void ElfReadHeader(void);
static void CopySegments(void);
static void OutErrorFatal(void);
static void ElfErrorFatal(void);
static void ElfError(char *msg);
static void ElfWarning(char *msg);

/** Write section descriptor (2 chars) and section size (4 bytes, big endian). */
static void writeinfo(uchar * ptr, uchar * name, long size)
{
    *ptr++ = name[0];
    *ptr++ = name[1];
    *ptr++ = (uchar) (size >> 24);
    *ptr++ = (uchar) (size >> 16);
    *ptr++ = (uchar) (size >> 8);
    *ptr++ = (uchar) (size);
}

int main(int argc, char *argv[])
{
    uchar t32hdr[SZ_T32HEADER];

    printf("TRACE32 TASK File Generator V1.1\n");

    if ((argc < 3) || (*argv[1] == '?')) {
	printf("\nUSAGE: elf2t32 <infile> <outfile>\n");
	exit(2);
    }

    { /*+ varblock */
	char *sname, *dname;
	int i;
	sname = argv[1];
	dname = argv[2];
	if ((ElfHandle = open(sname, O_RDONLY | O_BINARY)) == -1) {
	    printf("\ncannot open input file %s\n", sname);
	    exit(1);
	}
	if ((OutHandle = open(dname, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, 0666)) == -1) {
	    printf("\ncannot create output file\n");
	    exit(1);
	}
	printf("converting %s to %s ...", sname, dname);

	strncpy(t32hdr, "T32M_003.001@(#)TRACE32 configuration file V003.001 : ", SZ_T32HEADER);
	t32hdr[4] = 0;
	for (i = 0; i < 16; i++) {
	    if (sname[i] == '.')
		break;
	    t32hdr[0x36 + i] = sname[i];
	}
	t32hdr[0x46] = 0;	/* 0x46: alignment (2 bytes) */
	t32hdr[0x47] = 0;
    } /*- varblock */

    ElfDataAddress = DEFAULT_DATA_ADDRESS;

    ElfReadHeader();		/* ELF-Header/Section Header lesen */
    CopySegments();		/* Segmente lesen */

    writeinfo(&t32hdr[0x48], "tl", ElfTextSize);		/* 0x48: text length */
    writeinfo(&t32hdr[0x4E], "dl", ElfDataSize);		/* 0x4E: data length */
    writeinfo(&t32hdr[0x54], "da", ElfDataAddress);		/* 0x54: data address */
    writeinfo(&t32hdr[0x5A], "ep", ElfStartAddress);		/* 0x5A: entry point */
    writeinfo(&t32hdr[0x60], "hs", ElfSectionMaxAddress);	/* 0x60: heap start */
    t32hdr[0x66] = 0;		/* 0x66: alignment (2 bytes) */
    t32hdr[0x67] = 0;

    OUT_PUTBLOCK(t32hdr, SZ_T32HEADER);

    OUT_PUTBLOCK("TEXT", 4);
    if (ElfTextSize) {
	OUT_PUTBLOCK(textbuffer, ElfTextSize);
	free(textbuffer);
    }

    OUT_PUTBLOCK("DATA", 4);
    if (ElfDataSize) {
	OUT_PUTBLOCK(databuffer, ElfDataSize);
	free(databuffer);
    }

    OUT_PUTBLOCK("T32E", 4);

    close(ElfHandle);
    close(OutHandle);

    printf("\n");
    return 0;
}

void ElfReadHeader(void)
{
    uint32 tmp;

    if (ELF_HEADER_SIZE > SZ_MAX_BUFFER)
	ElfError("internal buffer too small for ELF header");

    ELF_GETBLOCK(buffer, ELF_HEADER_SIZE);	/* get ELF header */
    if (buffer[0] != 0x7F 
	|| buffer[1] != 'E' || buffer[2] != 'L' || buffer[3] != 'F' 
	|| buffer[4] != 1	/* EI_CLASS   must be ELFCLASS32  (32bit file) */
	|| buffer[5] != 2	/* EI_DATA    must be ELFDATA2MSB (big endian) */
	|| buffer[6] > 1	/* EI_VERSION must be EV_CURRENT  (revision 1) */ )
	ElfError("bad magic");

    ELF_LONG(tmp, buffer + 20);
    if (tmp != SHT_PROGBITS)
	ElfError("bad ELF Header type");

    ELF_LONG(ElfStartAddress, buffer + 24);	/* Program Entry Point */
    ELF_LONG(ElfProgramFptr, buffer + 28);	/* Program Headers Table */
    ELF_LONG(ElfSectionFptr, buffer + 32);	/* Section Headers Table */
    ELF_WORD(ElfProgramEntrySize, buffer + 42);	/* Program Header Size */
    ELF_WORD(ElfProgramMax, buffer + 44);	/* Program Header Number */
    ELF_WORD(ElfSectionEntrySize, buffer + 46);	/* Section Header Size */
    ELF_WORD(ElfSectionMax, buffer + 48);	/* Section Header Number */
    ELF_WORD(ElfSectionStrings, buffer + 50);	/* Strings Section Index */

    if (ElfSectionEntrySize > SZ_MAX_BUFFER)
	ElfError("internal buffer too small for ELF section headers");

    /* calculate position and size of string table, then read it */
    tmp = ElfSectionFptr + (ElfSectionStrings * ElfSectionEntrySize);
    ELF_SEEK(tmp);
    ELF_GETBLOCK(buffer, ElfSectionEntrySize);	/* get string table header */
    ELF_LONG(tmp, buffer + 4);
    if (tmp != SHT_STRTAB)
	ElfError("bad string section header type");
    ELF_LONG(ElfStringFptr, buffer + 16);
    ELF_LONG(ElfStringSize, buffer + 20);
    ELF_SEEK(ElfStringFptr);

    ElfSectionStringTable = (char *)malloc(ElfStringSize);
    if (!ElfSectionStringTable)
	ElfError("error: cannot allocate string section buffer");
    ELF_GETBLOCK(ElfSectionStringTable, ElfStringSize);

    /* read other sections to get last data section */
    ELF_SEEK(ElfSectionFptr);
    ElfSectionMaxAddress = 0;
    for (ElfSectionIndex = 0; ElfSectionIndex < ElfSectionMax; ElfSectionIndex++) {
	char *nameptr;
	ELF_GETBLOCK(buffer, ElfSectionEntrySize);
	ELF_LONG(tmp, buffer);
	nameptr = ElfSectionStringTable + tmp;
	ELF_LONG(ElfSectionAddress, buffer + 12);
	ELF_LONG(ElfSectionSize, buffer + 20);
	/* if (!strcmp(nameptr, ".bss")) ... - not used */
	if ((ElfSectionAddress + ElfSectionSize) > ElfSectionMaxAddress)
	    ElfSectionMaxAddress = ElfSectionAddress + ElfSectionSize;
	DBGINFO("\ninfo: [#%2d] 0x%08lx %s", ElfSectionIndex, ElfSectionMaxAddress, nameptr);
    }

    free(ElfSectionStringTable);
}

void CopySegments(void)
{
    uint32 next_entry;
    int i;

    if (ElfProgramMax > 2)
	ElfError("error: more then two load segments in file");

    next_entry = ElfProgramFptr;
    for (i = 0; i < ElfProgramMax; i++) {
	uint32 tmp;

	ELF_SEEK(next_entry);
	ELF_GETBLOCK(buffer, ElfProgramEntrySize);
	next_entry = ELF_TELL();

	ELF_LONG(tmp, buffer + 0);		/* p_type */
	if (tmp != PT_LOAD)
	    ElfError("program segment type not PT_LOAD (0x01)");

	ELF_LONG(ElfSectionAddress, buffer + 8);/* p_vaddr - virtual load address */

	/* if available, use physical address as load address */
	ELF_LONG(tmp, buffer + 12);		/* p_paddr - segment physical address */
	if (tmp)
	    ElfSectionAddress = tmp;

	ELF_LONG(ElfSectionSize, buffer + 16);	/* p_filesz - file size of this segment */
	ELF_LONG(tmp, buffer + 20);		/* p_memsz - memory size of this segment */
	if (tmp != ElfSectionSize)
	    DBGINFO("\ninfo: [%s] p_filesz=0x%08lx p_memsz=0x%08lx",
	            (i == 0) ? "text" : "data", ElfSectionSize, tmp);

	if (i == 0) {		/* Text segment (.text, .rodata, .rodata.str1.4) */
	    if (ElfSectionAddress != DEFAULT_TEXT_ADDRESS)
		ElfError("error: text segment starts not at zero");
	    ElfTextSize = ElfSectionSize;
	    if (ElfTextSize) {
		if (ElfTextSize > DEFAULT_TEXT_SZ_MAX)
		    ElfWarning("warning: text segment larger than default (0x40000)");
		textbuffer = (uchar *) malloc(ElfTextSize);
		if (!textbuffer)
		    ElfError("error: cannot allocate text segment buffer");
		ELF_LONG(tmp, buffer + 4);	/* p_offset - file offset to segment */
		ELF_SEEK(tmp);
		ELF_GETBLOCK(textbuffer, ElfTextSize);
	    } else {
		textbuffer = NULL;
	    }
	} else {		/* Data segment (.data, .bss) */
	    ElfDataAddress = ElfSectionAddress;
	    ElfDataSize = ElfSectionSize;
	    if (ElfDataSize) {
		if (ElfSectionSize > DEFAULT_DATA_SZ_MAX)
		    ElfWarning("warning: data segment larger than default (0x4000)");
		databuffer = (uchar *) malloc(ElfDataSize);
		if (!databuffer)
		    ElfError("error: cannot allocate data segment buffer");
		ELF_LONG(tmp, buffer + 4);	/* p_offset - file offset to segment */
		ELF_SEEK(tmp);
		ELF_GETBLOCK(databuffer, ElfDataSize);
	    } else {
		databuffer = NULL;
	    }
	}
    }
}

void OutErrorFatal(void)
{
    ElfError("error writing file");
}

void ElfErrorFatal(void)
{
    ElfError("fatal error");
}

void ElfError(char *msg)
{
    printf("\n%s\n", msg);
    exit(2);
}

void ElfWarning(char *msg)
{
    printf("\n%s\n", msg);
}

/*eof*/
