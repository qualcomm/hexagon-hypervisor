/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "h2_symtab.h"
#include "h2_elf.h"

typedef struct {
	unsigned int addr;
	const char *name;
} symbol_entry_t;

static symbol_entry_t *symbol_table = NULL;
static int symbol_count = 0;
static char *string_table = NULL;

/* Comparison function for qsort */
static int compare_symbols(const void *a, const void *b)
{
	const symbol_entry_t *sym_a = (const symbol_entry_t *)a;
	const symbol_entry_t *sym_b = (const symbol_entry_t *)b;

	if (sym_a->addr < sym_b->addr) return -1;
	if (sym_a->addr > sym_b->addr) return 1;
	return 0;
}

/* Binary search for symbol lookup */
static int find_symbol_index(unsigned int addr)
{
	int left = 0;
	int right = symbol_count - 1;
	int best_match = -1;

	while (left <= right) {
		int mid = left + (right - left) / 2;

		if (symbol_table[mid].addr == addr) {
			return mid;
		} else if (symbol_table[mid].addr < addr) {
			best_match = mid;  /* This symbol is before our address */
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	/* Return the closest symbol that's <= addr */
	return best_match;
}

int h2_symtab_init(const char *elf_path)
{
	int fdesc;
	Elf32_Ehdr ehdr;
	Elf32_Shdr symhdr, strhdr;
	Elf32_Sym *symtab = NULL;
	int i, j;
	int func_count = 0;

	printf("h2_symtab: Initializing symbol table...\n");

	/* Use default path if none provided */
	if (elf_path == NULL) {
		printf("h2_symtab: elf_path == NULL");
		return -1;
	} else {
		printf("h2_symtab: Using ELF path: %s\n", elf_path);
	}

	/* Open ELF file */
	fdesc = open(elf_path, O_RDONLY);
	if (fdesc < 0) {
		printf("h2_symtab: ERROR - Failed to open ELF file: %s\n", elf_path);
		return -1;
	}
	printf("h2_symtab: Successfully opened ELF file (fd=%d)\n", fdesc);

	/* Read ELF header */
	if (elf_get_ehdr(fdesc, &ehdr) != 0) {
		printf("h2_symtab: ERROR - Failed to read ELF header\n");
		close(fdesc);
		return -1;
	}
	printf("h2_symtab: ELF header read successfully (sections=%d)\n", ehdr.e_shnum);

	/* Find symbol table and string table sections */
	int found_symtab = 0, found_strtab = 0;
	for (i = 0; i < ehdr.e_shnum; i++) {
		Elf32_Shdr shdr;

		if (elf_get_shdr(fdesc, i, &shdr, &ehdr) != 0) {
			continue;
		}

		if (shdr.sh_type == SHT_SYMTAB) {
			symhdr = shdr;
			found_symtab = 1;
		} else if (shdr.sh_type == SHT_STRTAB && i != ehdr.e_shstrndx) {
			strhdr = shdr;
			found_strtab = 1;
		}

		if (found_symtab && found_strtab) {
			break;
		}
	}

	if (!found_symtab || !found_strtab) {
		printf("h2_symtab: ERROR - Symbol table or string table not found (symtab=%d, strtab=%d)\n",
		       found_symtab, found_strtab);
		close(fdesc);
		return -1;
	}
	printf("h2_symtab: Found symbol table (offset=0x%x, size=0x%x) and string table (offset=0x%x, size=0x%x)\n",
	       symhdr.sh_offset, symhdr.sh_size, strhdr.sh_offset, strhdr.sh_size);

	/* Allocate symbol table */
	symtab = malloc(symhdr.sh_size);
	if (symtab == NULL) {
		printf("h2_symtab: Failed to allocate memory for symbol table\n");
		close(fdesc);
		return -1;
	}

	/* Read symbol table */
	if (lseek(fdesc, symhdr.sh_offset, SEEK_SET) == -1 ||
	    read(fdesc, symtab, symhdr.sh_size) != symhdr.sh_size) {
		printf("h2_symtab: Failed to read symbol table\n");
		free(symtab);
		close(fdesc);
		return -1;
	}

	/* Allocate string table */
	string_table = malloc(strhdr.sh_size);
	if (string_table == NULL) {
		printf("h2_symtab: Failed to allocate memory for string table\n");
		free(symtab);
		close(fdesc);
		return -1;
	}

	/* Read string table */
	if (lseek(fdesc, strhdr.sh_offset, SEEK_SET) == -1 ||
	    read(fdesc, string_table, strhdr.sh_size) != strhdr.sh_size) {
		printf("h2_symtab: Failed to read string table\n");
		free(string_table);
		free(symtab);
		close(fdesc);
		return -1;
	}

	/* Count function symbols */
	int num_symbols = symhdr.sh_size / sizeof(Elf32_Sym);
	printf("h2_symtab: Total symbols in table: %d\n", num_symbols);

	for (i = 0; i < num_symbols; i++) {
		unsigned char st_type = ELF32_ST_TYPE(symtab[i].st_info);
		if (st_type == STT_FUNC && symtab[i].st_value != 0) {
			func_count++;
		}
	}
	printf("h2_symtab: Found %d function symbols\n", func_count);

	/* Allocate symbol table */
	symbol_table = malloc(func_count * sizeof(symbol_entry_t));
	if (symbol_table == NULL) {
		printf("h2_symtab: Failed to allocate symbol table\n");
		free(string_table);
		free(symtab);
		close(fdesc);
		return -1;
	}

	/* Populate symbol table with function symbols */
	j = 0;
	for (i = 0; i < num_symbols; i++) {
		unsigned char st_type = ELF32_ST_TYPE(symtab[i].st_info);
		if (st_type == STT_FUNC && symtab[i].st_value != 0) {
			symbol_table[j].addr = symtab[i].st_value;
			symbol_table[j].name = string_table + symtab[i].st_name;
			j++;
		}
	}

	symbol_count = func_count;

	/* Sort symbol table by address for binary search */
	qsort(symbol_table, symbol_count, sizeof(symbol_entry_t), compare_symbols);

	/* Cleanup */
	free(symtab);
	close(fdesc);

	printf("h2_symtab: Successfully loaded %d function symbols\n", symbol_count);

	return 0;
}

const char* h2_symtab_lookup(unsigned int addr, unsigned int *offset)
{
	static int first_lookup = 1;

	if (symbol_table == NULL || symbol_count == 0) {
		if (first_lookup) {
			printf("h2_symtab_lookup: WARNING - Symbol table not initialized (table=%p, count=%d)\n",
			       symbol_table, symbol_count);
			first_lookup = 0;
		}
		return NULL;
	}

	if (first_lookup) {
		first_lookup = 0;
	}

	int idx = find_symbol_index(addr);
	if (idx < 0) {
		return NULL;
	}

	/* Calculate offset from symbol base */
	if (offset != NULL) {
		*offset = addr - symbol_table[idx].addr;
	}

	return symbol_table[idx].name;
}

void h2_symtab_cleanup(void)
{
	if (symbol_table != NULL) {
		free(symbol_table);
		symbol_table = NULL;
	}

	if (string_table != NULL) {
		free(string_table);
		string_table = NULL;
	}

	symbol_count = 0;
}

int h2_symtab_count(void)
{
	return symbol_count;
}
