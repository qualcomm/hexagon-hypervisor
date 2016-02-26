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

#define SPECIALS_BUFSIZE 256

int elf_get_specials(int fdesc, special_symbols specials[], int nsyms, const Elf32_Ehdr *ehdr) {

	int i, j, n_el, bytes, pos;
	int ntomatch = nsyms;
	Elf32_Shdr strhdr, symhdr, 
		shstr;  // section-header string table section header :()
	Elf32_Sym sym;
	int shstrtab_offset;
	char *shstrtab;

	char buf[SPECIALS_BUFSIZE];

	if (SHN_UNDEF == ehdr->e_shstrndx) goto error;
	if (-1 == elf_get_shdr(fdesc, ehdr->e_shstrndx, &shstr, ehdr)) goto error;
	shstrtab_offset = shstr.sh_offset;
	shstrtab = (char *)ehdr + shstrtab_offset;

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (i == ehdr->e_shstrndx) continue;
		if (elf_get_shdr(fdesc,i,&strhdr,ehdr) == -1) goto error;
		if (strhdr.sh_type != SHT_STRTAB) continue;
		if (0 == strcmp(shstrtab + strhdr.sh_name, ".strtab")) break;
	}
	if (strhdr.sh_type != SHT_STRTAB) goto error;
	if ((pos = lseek(fdesc, strhdr.sh_offset, SEEK_SET)) == -1) goto error;

	/* Find the symbols in the string table first */
	while (ntomatch && (bytes = read(fdesc, buf, SPECIALS_BUFSIZE)) && bytes != -1) {
		for ( i = 0; i < nsyms; i++) {
			if (0 == strcmp(specials[i].name, buf)) {
				/* got a match; temporarily store the offset in the array */
				specials[i].addr = pos - strhdr.sh_offset;
				ntomatch--;
				break; // presumably we're looking for unique symbols
			}
		}
		if ((pos = lseek(fdesc, pos + strlen(buf) + 1, SEEK_SET)) == -1) goto error;
	}

	/* Get the addresses from the symbol table */
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (elf_get_shdr(fdesc,i,&symhdr,ehdr) == -1) goto error;
		if (symhdr.sh_type == SHT_SYMTAB) break;
	}
	if (symhdr.sh_type != SHT_SYMTAB) goto error;

	n_el = symhdr.sh_size / symhdr.sh_entsize;
	if (lseek(fdesc, symhdr.sh_offset,SEEK_SET) == -1) goto error;

	ntomatch = nsyms;
	for (i = 1; i < n_el; i++) {
		if (read(fdesc, &sym, sizeof(sym)) != sizeof(sym)) goto error;
		for (j = 0; j < nsyms; j++) {
			if (specials[j].addr == sym.st_name) { // match
				specials[j].addr = sym.st_value; // replace with value
				if (--ntomatch == 0) {
					break;
				}
			}
		}
	}
	return 0;

 error:
	return -1;
}
