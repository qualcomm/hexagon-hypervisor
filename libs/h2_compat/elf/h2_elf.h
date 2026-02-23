/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * elf.h 
 * 
 * Type in all the damn stuff from the stupid ELF spec 
 * because the elf.h files on real machines are GPL or Solaris or whatever
 * 
 * $Id$
 * 
 */

#ifndef _ELF_H_ 
#define _ELF_H_ 1

#ifdef MULTICORE
#define BOOTER_PRINTF(...) if (!((silent >> core_id) & 0x1)) {printf("CORE %d:\t", core_id); printf(__VA_ARGS__);}
#else
#define BOOTER_PRINTF(...) if (!silent) printf(__VA_ARGS__)
#endif

typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;

typedef int Elf32_Sword; /* Alex, I'll take "swords" for 1000 */
typedef unsigned int Elf32_Word;

/* e_type values */
#define ET_NONE 0 /* no file type */
#define ET_REL  1 /* relocatable file */
#define ET_EXEC 2 /* executable file */
#define ET_DYN  3 /* Shared Object file */
#define ET_CORE 4 /* Core dump ... Ha ha! */
#define ET_LOPROC 0xff00 /* Processor-specific */
#define ET_HIPROC 0xffff /* Processor-specific */

/* e_machine values */
#define EM_NONE		0 	/* no machine */
#define EM_M32		1 	/* AT&T WE 32100 */
#define EM_SPARC	2	/* SPARC ... can't you tell from macro? */
#define EM_386		3	/* IA-32 */
#define EM_68k		4	/* Motorola 68k */
#define EM_88k		5	/* Motorola failed 88k architecture */
/* 6 is reserved */
#define EM_860		7	/* Intel failed 80860 */
#define EM_MIPS		8	/* MIPS */
#define EM_S370		9	/* IBM System/370 */
#define EM_MIPS_2	10	/* MIPS Rs? Big? Little? Endian */
/* 11-14 reserved */
#define EM_PARISC	15	/* HP PA-RISC */
/* 16 reserved */
#define EM_VPP500	17	/* Fujitsu VPP500 */
#define EM_SPARC32PLUS	18	/* SPARC with more instructions */
#define EM_960		19	/* Intel 80960 */
#define EM_PPC		20	/* Motorola/IBM/Apple/Etc PowerPC */
#define EM_PPC64	21	/* Motorola/IBM/Apple/Etc PowerPC 64-bit */
/* 22-35 reserved */
#define EM_V800		36	/* NEC V800 */
#define EM_FR20		37	/* Fujitsu FR20. Why am I putting these in? */
#define EM_RH32		38	/* TRW RH-32 */
#define EM_RCE		39	/* Moto RCE... not to be confused with RCF */
#define EM_ARM		40	/* Advanced RISC Machine? Yeah, right */
#define EM_ALPHA	41	/* DEC Alpha, Rest In Peace */
#define EM_SH		42	/* Hitachi SH, found in Dreamcast I think */
#define EM_SPARCV9	43	/* Sparc Version 9 */
#define EM_TRICORE	44	/* Siemens "Tricore" */
#define EM_ARC		45	/* Argonaut RISC */
#define EM_H8_300	46	/* Hitachi H8/300 */
#define EM_H8_300H	47	/* Hitachi H8/300H.. too many H's */
#define EM_H8S		48	/* Hitachi H8S */
#define EM_H8_500	49	/* Hitachi H8/500 */
#define EM_IA_64	50	/* Bwuahaha... Itanic Architecture */
#define EM_MIPS_X	51	/* from Stanford */
#define EM_COLDFIRE	52	/* Moto Coldfire, in my Palm Pilot */
#define EM_68HC12	53	/* Moto 68HC12 microcontroller, dodge neon */
#define EM_MMA		54	/* Fujitsu MMA Multimedia Accellerator */
#define EM_PCP		55	/* Siemens PCP */
#define EM_NCPU		56	/* Sony nCPU embedded RISC */
#define EM_NDR1		57	/* Denso NDR1 Microprocessor */
#define EM_STARCORE	58	/* Moto/Agere/Infineon/SCLLC Star*Core DSP */
#define EM_ME16		59	/* Toyota ME16 processor */
#define EM_ST100	60	/* STMicro ST100 */
#define EM_TINYJ	61	/* Advanced Logic TinyJ processor */
/* 62-65 reserved */
#define EM_FX66		66	/* reserved for future use */
#define EM_ST9PLUS	67	/* STMicro ST9+ microcontroller */
#define EM_ST7		68	/* STMicro ST7 microcontroller */
#define EM_68HC16	69	/* Moto microcontroller */
#define EM_68HC11	70	/* Moto microcontroller */
#define EM_68HC08	71	/* Moto microcontroller */
#define EM_68HC05	72	/* Moto microcontroller */
#define EM_SVX		73	/* SGI SVx */
#define EM_ST19		74	/* STMicro ST19 microcontroller */
#define EM_VAX		75	/* DEC VAX */
#define EM_CRIS		76	/* Axis Comm. CRIS... gcc for dunces */
#define EM_JAVELIN	77	/* Infineon embedded proc */
#define EM_FIREPATH	78	/* element 14 64-bit DSP */
#define EM_ZSP		79	/* LSI Logic 16-bit DSP */
#define EM_MMIX		80	/* Donald Knuth's 64-bit processor */
#define EM_HUANY	81	/* Harvard machine-indep object files */
#define EM_PRISM	82	/* SiTera Prism */
#define EM_AVR		83	/* Amtel AVR 8-bit microcontroller */
#define EM_FR30		84	/* Fujitsu FR30 */
#define EM_D10V		85	/* Mitsubishi D10V */
#define EM_D30V		86	/* Mitsubishi D30V */
#define EM_V850		87	/* NEC v850 */
#define EM_M32R		88	/* Mitsubishi M32R */
#define EM_MN10300	89	/* Matsushita MN10300 */
#define EM_MN10200	90	/* Matsushita MN10200 */
#define EM_PJ		91	/* Pico Java... Ben & Jerrys Java Chip */
#define EM_QDSP6	128	/* The greatest architecture... EVER */
#define EM_POLARIS	0xdead	/* It was true... */

/* ELF Identification */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7
#define EI_NIDENT	16

#define ELFMAG0		0x7f	/* e_ident[EI_MAG0] */
#define ELFMAG1		'E'	/* e_ident[EI_MAG1] */
#define ELFMAG2		'L'	/* e_ident[EI_MAG2] */
#define ELFMAG3		'F'	/* e_ident[EI_MAG3] */

#define ELFCLASSNONE	0	/* Invalid class */
#define ELFCLASS32	1	/* 32-bit ELF */
#define ELFCLASS64	2	/* 64-bit ELF */

#define ELFDATANONE	0	/* Invalid encoding */
#define ELFDATA2LSB	1	/* Little Endian */
#define ELFDATA2MSB	2	/* Big Endian */

/* ELF Version */
#define EV_NONE		0	/* Invalid Version */
#define EV_CURRENT	1	/* Current Version */

#define SHN_UNDEF	0	/* undefined section header num */
#define SHN_LORESERVE	0xff00	/* Lower bound of reserved indexes */
#define SHN_LOPROC	0xff00	/* Processor-specific low bound */
#define SHN_HIPROC	0xff1f	/* processor-specific high bound */
#define SHN_ABS		0xfff1	/* Absolute Values for reference */
#define SHN_COMMON	0xfff2	/* Symbols are common */
#define SHN_HIRESERVE	0xffff	/* High bound of reserved indexes */

#define SHT_NULL	0	/* section header w/ no assoc section */
#define SHT_PROGBITS	1	/* Section holds info for program */
#define SHT_SYMTAB	2	/* Symbol table */
#define SHT_STRTAB	3	/* String Table */
#define SHT_RELA	4	/* Relocation entries with addends */
#define SHT_HASH	5	/* Symbol hash table */
#define SHT_DYNAMIC	6	/* Info for dynamic linking */
#define SHT_NOTE	7	/* Some kind of information about the file */
#define SHT_NOBITS	8	/* Like SHT_PROGBITS, but no space in file */
#define SHT_REL		9	/* relocation entries w/o addends */
#define SHT_SHLIB	10	/* reserved */
#define SHT_DYNSYM	11	/* Symbol Table */
#define SHT_LOPROC	0x70000000	/* Processor Specific semantics */
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000	/* Application programs reserved */
#define SHT_HIUSER	0xffffffff

#define SHF_WRITE	0x1	/* writable during process execution */
#define SHF_ALLOC	0x2	/* occupies memory during execution */
#define SHF_EXECINSTR	0x4	/* Contains instructions */
#define SHF_MASKPROC	0xf0000000	/* Processor-specific */

#define STN_UNDEF	0	/* Symbol Table Number */

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4) + ((t)&0xf))

/* Symbol Binding */
#define STB_LOCAL	0	/* symbols invis outside file */
#define STB_GLOBAL	1	/* symbols vis to all files */
#define STB_WEAK	2	/* like global, but lower prec */
#define STB_LOPROC	13
#define STB_HIPROC	15

/* Symbol Types */
#define STT_NOTYPE	0	/* not specified */
#define STT_OBJECT	1	/* data */
#define STT_FUNC	2	/* Function or executable code */
#define STT_SECTION	3	/* section, used for relocation */
#define STT_FILE	4	/* file symbol */
#define STT_LOPROC	13	/* Processor specific */
#define STT_HIPROC	15

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

/* Program Segment Types */
#define PT_NULL		0	/* unused */
#define PT_LOAD		1	/* loadable segment */
#define PT_DYNAMIC	2	/* dynamic linking information */
#define PT_INTERP	3	/* location and size of path of interpreter */
#define PT_NOTE		4	/* Location and size of aux info */
#define PT_SHLIB	5	/* Reserved */
#define PT_PHDR		6	/* location, size of Program header Table */
#define PT_LOPROC	0x70000000	/* Processor-specific */
#define PT_HIPROC	0x7fffffff

#define PF_X		1	/* executable */
#define PF_W		2	/* writable */
#define PF_R		4	/* Readable */
#define PF_MASKPROC	0xf0000000	/* Processor-specific mask */

typedef struct elf_header_struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;  // index of 'section-header string table' section header
} Elf32_Ehdr;

typedef struct elf_section_struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_line;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct elf_symtab_struct {
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
} Elf32_Sym;

typedef struct elf_relocation_struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;

typedef struct elf_relocationa_struct {
	Elf32_Addr r_offset;	/* location to apply relocation */
	Elf32_Word r_info;	/* symbol table index, type of relocation */
	Elf32_Sword r_addend;	/* constant addend to compute value */
} Elf32_Rela;

typedef struct elf_program_struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

typedef struct special_symbols {
	const char *name;
	int addr;
} special_symbols;

int elf_get_ehdr(int fdesc, Elf32_Ehdr *ehdr);
int elf_get_phdr(int fdesc, int i, Elf32_Phdr *phdr, const Elf32_Ehdr *ehdr);
int elf_get_shdr(int fdesc, int i, Elf32_Shdr *shdr, const Elf32_Ehdr *ehdr);
//int elf_get_symbol(int fdesc, const char *sym, const Elf32_Ehdr *ehdr);
int elf_get_specials(int fdesc, special_symbols specials[], int nsyms, const Elf32_Ehdr *ehdr);
void elf_error(char *str1, char *str2);

#define ERRSTR_LEN 1024
extern char elf_errstr[ERRSTR_LEN];

#endif /* _ELF_H_ */
