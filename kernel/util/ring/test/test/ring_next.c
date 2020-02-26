/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <ringtests.h>
#include <ring.h>
#include <stdio.h>

static void test0()
{
	H2K_ringnode_t *tmp, *ret;
	tmp = makering("a");
	ret = H2K_ring_next(tmp,getnode('a'));
	checkring(ret,"");
}

static void test1()
{
	H2K_ringnode_t *tmp, *ret;
	tmp = makering("b");
	ret = H2K_ring_next(tmp,getnode('a'));
	checkring(ret,"a");
}

static void test2()
{
	H2K_ringnode_t *tmp, *ret;
	tmp = makering("b");
	ret = H2K_ring_next(getnode('a'),tmp);
	checkring(ret,"b");
}

void test_ring_next()
{
	test0();
	test1();
	test2();
}

