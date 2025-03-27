/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* 
 * elf.c
 * 
 * Fun functions for dealing with ELF files 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <angel.h>

char errstr[ERRSTR_LEN];

extern unsigned int silent, core_id;

void error(char *str1, char *str2) {

	int err = sys_errno();
	strncat(errstr, ": ", ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str1, ERRSTR_LEN - strlen(errstr) - 1);
	strncat(errstr, str2, ERRSTR_LEN - strlen(errstr) - 1);
	errno = err;
	perror(errstr);

	exit(1);
}

int elf_get_ehdr(int fdesc, Elf32_Ehdr *ehdr)
{
	if (lseek(fdesc,0,SEEK_SET) == -1) return -1;
	if (read(fdesc,ehdr,sizeof(*ehdr)) != sizeof(*ehdr)) return -1;
	if ((ehdr->e_ident[EI_MAG0] != ELFMAG0) ||
		(ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
		(ehdr->e_ident[EI_MAG2] != ELFMAG2) ||
		(ehdr->e_ident[EI_MAG3] != ELFMAG3) ||
		(ehdr->e_ident[EI_DATA] != ELFDATA2LSB)) {
		/* Not a valid ELF File */
		return -1;
	}
	return 0;
}

int elf_get_phdr(int fdesc, int i, Elf32_Phdr *phdr, const Elf32_Ehdr *ehdr)
{
	if (lseek(fdesc,ehdr->e_phoff+i*ehdr->e_phentsize,SEEK_SET) == -1) return -1;
	if (read(fdesc,phdr,sizeof(*phdr)) != sizeof(*phdr)) return -1;
	return 0;
}

int elf_get_shdr(int fdesc, int i, Elf32_Shdr *shdr, const Elf32_Ehdr *ehdr)
{
	if (lseek(fdesc,ehdr->e_shoff+i*ehdr->e_shentsize,SEEK_SET) == -1) return -1;
	if (read(fdesc,shdr,sizeof(*shdr)) != sizeof(*shdr)) return -1;
	return 0;
}

int elf_get_specials(int fdesc, special_symbols specials[], int nsyms, const Elf32_Ehdr *ehdr) {
	int i, bytes, pos;

	int ntomatch = nsyms;
	Elf32_Shdr hdr[2],	// used for strhdr & symhdr
		shstr;  // section-header string table section header :()

	if (SHN_UNDEF == ehdr->e_shstrndx) error("1", NULL);
	if (-1 == elf_get_shdr(fdesc, ehdr->e_shstrndx, &shstr, ehdr)) error("2", NULL);

	//	BOOTER_PRINTF("%s: reading symhdr & strhdr\n", __func__);
	/* find symhdr & strhdr */
	Elf32_Shdr *pSymhdr, *pStrhdr;
	int hdrNum = 0;
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (i == ehdr->e_shstrndx) continue;
		if (elf_get_shdr(fdesc,i,&hdr[hdrNum],ehdr) == -1) error("3", NULL);
		
		if (hdr[hdrNum].sh_type == SHT_SYMTAB) {
			pSymhdr = &hdr[hdrNum];
			hdrNum++;
		} else if (hdr[hdrNum].sh_type == SHT_STRTAB) {
			pStrhdr = &hdr[hdrNum];
			hdrNum++;
		}
		if (hdrNum == 2) break;	// got both headers
	}
	if (pSymhdr->sh_type != SHT_SYMTAB) error("4", NULL);
	if (pStrhdr->sh_type != SHT_STRTAB) error("5", NULL);
	
	/* read symtab & strtab */
	//	BOOTER_PRINTF("%s: reading symtab\n", __func__);
	Elf32_Sym *symtab = malloc(pSymhdr->sh_size);
	if (NULL == symtab) error("Can't malloc symtab", NULL);
	//	BOOTER_PRINTF("%s: symtab=0x%p 0x%xB\n", __func__, symtab, pSymhdr->sh_size);
	if (symtab == NULL) error("6", NULL);
	if ((pos = lseek(fdesc, pSymhdr->sh_offset, SEEK_SET)) == -1) error("7", NULL);
	if ( (bytes = read(fdesc, symtab, pSymhdr->sh_size)) != pSymhdr->sh_size) error("8", NULL);
	
	//	BOOTER_PRINTF("%s: reading strtab\n", __func__);
	char *strtab = malloc(pStrhdr->sh_size);
	if (NULL == strtab) error("Can't malloc strtab", NULL);
	//	BOOTER_PRINTF("%s: strtab=0x%p 0x%xB\n", __func__, strtab, pStrhdr->sh_size);
	if (strtab == NULL) error("9", NULL);
	if ((pos = lseek(fdesc, pStrhdr->sh_offset, SEEK_SET)) == -1) error("10", NULL);
	if ( (bytes = read(fdesc, strtab, pStrhdr->sh_size)) != pStrhdr->sh_size) error("11", NULL);
	
	//	BOOTER_PRINTF("%s: looking for special symbol values\n", __func__);
	/* Loop through the symbol table */
	Elf32_Sym *pSym = symtab;
	/* BOOTER_PRINTF("%s: symtab=0x%p pSymhdr->sh_size=0x%x (symtab + (pSymhdr->sh_size)/sizeof(Elf32_Sym)))=0x%p\n",  */
	/* 	__func__, symtab, pSymhdr->sh_size, (symtab + (pSymhdr->sh_size)/sizeof(Elf32_Sym))); */
	while ( ntomatch && (pSym < (symtab + (pSymhdr->sh_size)/sizeof(Elf32_Sym))) ) {
		const char *st_name = strtab+(pSym->st_name);
		/* BOOTER_PRINTF("%s: pSym=0x%p st_name=0x%x st_value=0x%x st_size=0x%x st_info=0x%02x st_other=0x%02x st_shndx=0x%04x strtab+(pSym->st_name)=0x%p\n",  */
		/* 	__func__,  */
		/* 	pSym, pSym->st_name, pSym->st_value, pSym->st_size, pSym->st_info, pSym->st_other, pSym->st_shndx,  */
		/* 	strtab+(pSym->st_name)); */
		/* BOOTER_PRINTF("%s: symbol=%s\n", __func__, st_name); */
		/* Loop through each of the specials we're looking for */
		for ( i = 0; i < nsyms; i++) {
			const char *looking_for = specials[i].name;
			if (0 == strcmp(looking_for, st_name)) {
				specials[i].addr = pSym->st_value;
				ntomatch--;
				//				BOOTER_PRINTF("%s: found %s\n", __func__, looking_for);
				break; // presumably we're looking for unique symbols
			}
		}
		pSym++;
	}
	
//	BOOTER_PRINTF("%s: got the symbol values\n", __func__);
	
	free(strtab);
	free(symtab);
	
//	BOOTER_PRINTF("%s: free'd memory\n", __func__);

	return 0;
}
