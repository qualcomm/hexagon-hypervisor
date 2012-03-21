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

