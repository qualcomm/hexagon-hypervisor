/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_SYMTAB_H
#define H2_SYMTAB_H

/**
 * Initialize the symbol table by parsing the ELF file
 * This should be called once at system initialization
 *
 * @param elf_path Path to the ELF file to parse (can be NULL to use default)
 * @return 0 on success, -1 on failure
 */
int h2_symtab_init(const char *elf_path);

/**
 * Lookup a symbol name for a given address
 *
 * @param addr The address to lookup
 * @param offset Pointer to store the offset from the symbol base (can be NULL)
 * @return Symbol name if found, NULL otherwise
 */
const char* h2_symtab_lookup(unsigned int addr, unsigned int *offset);

/**
 * Free resources used by the symbol table
 */
void h2_symtab_cleanup(void);

/**
 * Get the number of symbols loaded
 *
 * @return Number of symbols in the table
 */
int h2_symtab_count(void);

#endif /* H2_SYMTAB_H */
