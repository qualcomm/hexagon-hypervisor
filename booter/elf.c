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

int elf_get_ehdr(FILE *f, Elf32_Ehdr *ehdr)
{
	fseek(f,0,SEEK_SET);
	if (fread(ehdr,1,sizeof(*ehdr),f) != sizeof(*ehdr)) return -1;
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

int elf_get_phdr(FILE *f, int i, Elf32_Phdr *phdr, const Elf32_Ehdr *ehdr)
{
	fseek(f,ehdr->e_phoff+i*ehdr->e_phentsize,SEEK_SET);
	if (fread(phdr,1,sizeof(*phdr),f) != sizeof(*phdr)) return -1;
	return 0;
}

int elf_get_shdr(FILE *f, int i, Elf32_Shdr *shdr, const Elf32_Ehdr *ehdr)
{
	fseek(f,ehdr->e_shoff+i*ehdr->e_shentsize,SEEK_SET);
	if (fread(shdr,1,sizeof(*shdr),f) != sizeof(*shdr)) return -1;
	return 0;
}

int elf_get_symbol(FILE *f, const char *name, const Elf32_Ehdr *ehdr)
{
	int i,n_el, ret = -1;
	Elf32_Shdr strhdr,symhdr;
	Elf32_Sym sym;
	char *strings = NULL;
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (i == ehdr->e_shstrndx) continue;
		if (elf_get_shdr(f,i,&strhdr,ehdr) == -1) goto done;
		if (strhdr.sh_type == SHT_STRTAB) break;
	}
	if (strhdr.sh_type != SHT_STRTAB) return -1;
	if ((strings = malloc(strhdr.sh_size)) == NULL) goto done;
	fseek(f,strhdr.sh_offset,SEEK_SET);
	if (fread(strings,1,strhdr.sh_size,f) != strhdr.sh_size) goto done;

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (elf_get_shdr(f,i,&symhdr,ehdr) == -1) goto done;
		if (symhdr.sh_type == SHT_SYMTAB) break;
	}
	if (symhdr.sh_type != SHT_SYMTAB) goto done;

	n_el = symhdr.sh_size / symhdr.sh_entsize;
	fseek(f,symhdr.sh_offset,SEEK_SET);
	for (i = 1; i < n_el; i++) {
		if (fread(&sym,1,sizeof(sym),f) != sizeof(sym)) goto done;
		if (0==strcmp(name,strings+sym.st_name)) {
			ret = sym.st_value;
			break;
		} 
	}
done:
	if (strings) free(strings);
	return ret;
}

