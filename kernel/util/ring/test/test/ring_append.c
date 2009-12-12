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
	H2K_ring_append(&tmp,getnode('a'));
	checkring(tmp,"a");
}

static void test1()
{
	H2K_ringnode_t *tmp;
	tmp = makering("b");
	H2K_ring_append(&tmp,getnode('a'));
	checkring(tmp,"ba");
}

static void test2()
{
	H2K_ringnode_t *tmp;
	tmp = makering("bc");
	H2K_ring_append(&tmp,getnode('a'));
	checkring(tmp,"bca");
}

void test_ring_append()
{
	test0();
	test1();
	test2();
}

