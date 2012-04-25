/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"

typedef unsigned long long int u64_t;

#define SIZE_4K 0
#define SIZE_16K 1
#define SIZE_64K 2
#define SIZE_256K 3
#define SIZE_1M 4
#define SIZE_4M 5
#define SIZE_16M 6

#define L1WB_L2UC 0
#define L1WT_L2UC 1
#define L1WB_L2UC_S 2
#define L1WT_L2UC_S 3
#define UC 4
#define L1WT_L2C 5
#define UC_S 6
#define L1WB_L2C 7
#define L1WB_L2CWB_AUX 0xa

#define MAIN 0
#define AUX 1

#define U 1
#define R 2
#define W 4
#define X 8

#define RW (R|W)
#define RX (R|X)
#define WX (W|X)
#define RWX (R|W|X)
#define UR (U|R)
#define UW (U|W)
#define UX (U|X)
#define URW (U|R|W)
#define URX (U|R|X)
#define UWX (U|W|X)
#define URWX (U|R|W|X)

#define NONE 0

#define MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) \
        (((u64_t)((VPN) | ((PGSIZE) << 20)) << 32) | \
                (unsigned int)((((u64_t)(PPN))) | (((u64_t)(CFIELD)) << 24) | ((u64_t)(PERM) << 28))) ,

u64_t map[] = {
MEMORY_MAP_THREAD(      0,      0,      0x00000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x00000)
MEMORY_MAP_THREAD(      0,      0,      0x01000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x01000)
MEMORY_MAP_THREAD(      0,      0,      0x02000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x02000)
MEMORY_MAP_THREAD(      0,      0,      0x03000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x03000)
MEMORY_MAP_THREAD(      0,      0,      0x04000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x04000)
MEMORY_MAP_THREAD(      0,      0,      0x05000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x05000)
MEMORY_MAP_THREAD(      0,      0,      0x06000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x06000)
MEMORY_MAP_THREAD(      0,      0,      0x07000,        URWX,           L1WB_L2C,       SIZE_16M,       MAIN,   0x07000)
        0
};

#define MAX_SIZE (1024*1024)
#define NUM_TOTAL_THREADS 32
unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));
void spawn_vm(void *pc)
{
	unsigned int size;
	void *vmb;
	size = h2_config_vmblock_size(NUM_TOTAL_THREADS,1);
	if (size > MAX_SIZE) {
		printf("Size too small.");
		exit(1);
	}
	vmb = h2_config_vmblock_init(storage,SET_STORAGE,0,0);
	vmb = h2_config_vmblock_init(vmb,SET_PMAP_TYPE,0,0);
	h2_config_vmblock_init(vmb,SET_CPUS_INTS,NUM_TOTAL_THREADS,1);
	h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	h2_vmboot(pc,(void *)0x07fffff0,0,0,vmb);
	printf("vm booted\n");
}

int main(int argc, char **argv)
{
	int ret,i;
	FILE *f;
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	h2_init(0);
	if (argc < 2) {
		printf("%s <file>",argv[0]);
		return 1;
	}
	h2_vmtrap_newmap(map,H2K_ASID_TRANS_TYPE_LINEAR,H2K_ASID_TLB_INVALIDATE_FALSE);
	f = fopen(argv[1],"rb");
	if ((ret = elf_get_ehdr(f,&ehdr)) < 0) {
		printf("Invalid ELF file\n");
		return 1;
	}
	for (i = 0; i < ehdr.e_phnum; i++) {
		if (elf_get_phdr(f,i,&phdr,&ehdr) < 0) continue;
		if (phdr.p_memsz == 0) continue;
		if (phdr.p_type != PT_LOAD) continue;
		fseek(f,phdr.p_offset,SEEK_SET);
		if (phdr.p_filesz < phdr.p_memsz) phdr.p_filesz = phdr.p_memsz;
		fread((char *)phdr.p_paddr,1,phdr.p_filesz,f);
		memset((char *)phdr.p_paddr+phdr.p_filesz,0,phdr.p_memsz-phdr.p_filesz);
	}
	spawn_vm((void *)ehdr.e_entry);
	h2_thread_stop();
}

