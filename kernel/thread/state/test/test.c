/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdio.h>
#include <stdlib.h>
#include <state.h>
#include <vmblock.h>
#include <idtype.h>
#include <asm_offsets.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

static H2K_thread_context me;
static H2K_vmblock_t vmblock;

int main() {

	H2K_id_t id;
	u32_t *p;
	int i, j;
	u32_t offset;
	u64_t val, v;
	u8_t *q;

	vmblock.contexts = &me;
	vmblock.max_cpus = 1;

	me.vmblock = &vmblock;
	me.id.vmidx = 3;

	
	id.vmidx = 2;  // != 3
	id.cpuidx = 0;

	if (H2K_thread_state(id, 0, &me) != -1ULL) {
		FAIL("Didn't catch bad vmidx");
	}
	id.vmidx = 3;

	id.cpuidx = 1;
	if (H2K_thread_state(id, 0, &me) != -1ULL) {
		FAIL("Didn't catch bad cpuidx");
	}
	id.cpuidx = 0;

	if (H2K_thread_state(id, CONTEXT_SIZE, &me) != -1ULL) {
		FAIL("Didn't catch bad offset");
	}

	srand(3);
	for (p = (u32_t *)&me; p < ((u32_t *)&me + CONTEXT_SIZE / 4); p++) {
		*p = rand();
	}
	me.vmblock = &vmblock;
	me.id.vmidx = 3;

	for (i = 0; i < 10; i++) {
		offset = rand() % (CONTEXT_SIZE - 8);
		q = (u8_t *)&val;
		for (j = 0; j < 8; j++) {
			*q = *((u8_t *)&me + offset + j);
			q++;
		}

		if (v = H2K_thread_state(id, offset, &me) != val) {
			printf("Read 0x%016llu, expected 0x%016llu at offset %d\n", v, val, offset);
			FAIL("Bad value from context");
		}
	}

	puts("TEST PASSED\n");
	return 0;
}

