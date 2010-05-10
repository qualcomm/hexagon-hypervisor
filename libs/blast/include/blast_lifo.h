/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLAST LIFO interface
 * 
 * We provide lock free LastInFirstOut algorithm, which can be used in a
 * variaty of situation for allocation/free fixed size buffer
 *
 * This implementation will touch the first word of your FREED buffer. Even
 * though it doesn't matter how you use it when it is allocated, you might want
 * to be a bit careful not to put your MAGIC number as the first field.
 * Because it will not hold the magic value for "freed"
 *
 */

#ifndef BLAST_LIFO_H
#define BLAST_LIFO_H 1

/**
 * BLAST LIFO interface
 * 
 * We provide lock free LastInFirstOut algorithm, which can be used in a
 * variaty of situation for allocation/free fixed size buffer
 *
 * This implementation will touch the first word of your FREED buffer. Even
 * though it doesn't matter how you use it when it is allocated, you might want
 * to be a bit careful not to put your MAGIC number as the first field.
 * Because it will not hold the magic value for "freed"
 * 
 * blast_lifo_pop
 *
 * @param  freelist  pointer to the head of your list 
 * @return the top object from the list 
 */
void * blast_lifo_pop(void *freelist);

/**
 * Push an element into the LIFO
 *
 * @param  freelist  pointer to the head of your list 
 * @param  buf  pointer to your buffer to push into the list 
 * @return void 
 */
void blast_lifo_push(void *freelist, void *buf);

#endif

