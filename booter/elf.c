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

int elf_get_ehdr(int fdesc, Elf32_Ehdr *ehdr)
{
	lseek(fdesc,0,SEEK_SET);
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
	lseek(fdesc,ehdr->e_phoff+i*ehdr->e_phentsize,SEEK_SET);
	if (read(fdesc,phdr,sizeof(*phdr)) != sizeof(*phdr)) return -1;
	return 0;
}

int elf_get_shdr(int fdesc, int i, Elf32_Shdr *shdr, const Elf32_Ehdr *ehdr)
{
	lseek(fdesc,ehdr->e_shoff+i*ehdr->e_shentsize,SEEK_SET);
	if (read(fdesc,shdr,sizeof(*shdr)) != sizeof(*shdr)) return -1;
	return 0;
}

int elf_get_symbol(int fdesc, const char *name, const Elf32_Ehdr *ehdr)
{
	int i,n_el, ret = -1;
	Elf32_Shdr strhdr,symhdr;
	Elf32_Sym sym;
	char *strings = NULL;
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (i == ehdr->e_shstrndx) continue;
		if (elf_get_shdr(fdesc,i,&strhdr,ehdr) == -1) goto done;
		if (strhdr.sh_type == SHT_STRTAB) break;
	}
	if (strhdr.sh_type != SHT_STRTAB) return -1;
	if ((strings = malloc(strhdr.sh_size)) == NULL) goto done;
	lseek(fdesc,strhdr.sh_offset,SEEK_SET);
	if (read(fdesc,strings,strhdr.sh_size) != strhdr.sh_size) goto done;

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (elf_get_shdr(fdesc,i,&symhdr,ehdr) == -1) goto done;
		if (symhdr.sh_type == SHT_SYMTAB) break;
	}
	if (symhdr.sh_type != SHT_SYMTAB) goto done;

	n_el = symhdr.sh_size / symhdr.sh_entsize;
	lseek(fdesc,symhdr.sh_offset,SEEK_SET);
	for (i = 1; i < n_el; i++) {
		if (read(fdesc,&sym,sizeof(sym)) != sizeof(sym)) goto done;
		if (0==strcmp(name,strings+sym.st_name)) {
			ret = sym.st_value;
			break;
		}
	}
done:
	if (strings) free(strings);
	return ret;
}

