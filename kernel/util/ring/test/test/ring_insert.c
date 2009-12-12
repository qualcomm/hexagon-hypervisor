/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <ringtests.h>
#include <ring.h>
#include <stdio.h>

static void test0()
{
	H2K_ringnode_t *tmp;
	tmp = makering("");
	H2K_ring_insert(&tmp,getnode('a'));
	checkring(tmp,"a");
}

static void test1()
{
	H2K_ringnode_t *tmp;
	tmp = makering("b");
	H2K_ring_insert(&tmp,getnode('a'));
	checkring(tmp,"ab");
}

static void test2()
{
	H2K_ringnode_t *tmp;
	tmp = makering("bc");
	H2K_ring_insert(&tmp,getnode('a'));
	checkring(tmp,"abc");
}

void test_ring_insert()
{
	test0();
	test1();
	test2();
}

