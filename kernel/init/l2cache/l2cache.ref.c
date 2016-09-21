/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <fatal.h>
#include <hwconfig.h>
#include <globals.h>
#include <hw.h>
#include <h2_common_kerror.h>

typedef struct {
	u32_t archv;
	u8_t **ua;
} arch_t;

// L2 tag sizes
enum {
	s0 = 0, s64 = 1, s128 = 2, s256 = 3, s512 = 4, s1024 = 5, reserved = 7
};

// array size -> tag size

///// v5

static u8_t l2_v5_0[] =
	{
		[0x0] = s0,
		[0x1] = s128,
		[0x2] = s128,
		[0x3] = reserved,
		[0x4] = s128,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v5_1[] =
	{
		[0x0] = s0,
		[0x1] = s128,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = s256,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

///// v55

static u8_t l2_v55_0[] =
	{
		[0x0] = s0,
		[0x1] = s128,
		[0x2] = s128,
		[0x3] = reserved,
		[0x4] = reserved,  // was s256
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v55_1[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s256,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v55_2[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s0,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v55_3[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s128,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v55_f[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = s256,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

///// v56

static u8_t l2_v56_0[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = s256,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_1[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s256,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_3[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s512,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

#define l2_v56_5 (l2_v56_3)

static u8_t l2_v56_6[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s0,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_7[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_8[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s0,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_9[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_a[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s0,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_b[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s256,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_c[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = s256,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v56_e[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = s512,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

#define l2_v56_f (l2_v56_e)

///// v60

static u8_t l2_v60_0[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s0,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

#define l2_v60_1 (l2_v60_0)
#define l2_v60_2 (l2_v60_0)
#define l2_v60_4 (l2_v60_0)
#define l2_v60_5 (l2_v60_0)

static u8_t l2_v60_f[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s256,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v61_3[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s1024,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_0[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_1[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s1024,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_2[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = s512,
		[0x7] = reserved,
		[0x8] = s512,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_3[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s1024,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_4[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = s512,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_6[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_7[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = s1024,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v62_8[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s1024,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_0[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s1024,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_1[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s512,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_2[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = s512,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_6[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_7[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v65_8[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s1024,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_0[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s1024,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_1[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = s512,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_2[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = s1024,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_3[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = s256,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_5[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = s512,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = reserved,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

static u8_t l2_v66_6[] =
	{
		[0x0] = s0,
		[0x1] = reserved,
		[0x2] = reserved,
		[0x3] = reserved,
		[0x4] = reserved,
		[0x5] = reserved,
		[0x6] = reserved,
		[0x7] = reserved,
		[0x8] = reserved,
		[0x9] = reserved,
		[0xa] = s512,
		[0xb] = reserved,
		[0xc] = reserved,
		[0xd] = reserved,
		[0xe] = reserved,
		[0xf] = reserved
	};

// uarch -> (array-size -> tag-size map)

static u8_t *uarches_v5[] =
	{
		[0x0] = l2_v5_0,
		[0x1] = l2_v5_1,
		[0x2] = NULL,
		[0x3] = NULL,
		[0x4] = NULL,
		[0x5] = NULL,
		[0x6] = NULL,
		[0x7] = NULL,
		[0x8] = NULL,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = NULL
	};

static u8_t *uarches_v55[] =
	{
		[0x0] = l2_v55_0,
		[0x1] = l2_v55_1,
		[0x2] = l2_v55_2,
		[0x3] = l2_v55_3,
		[0x4] = NULL,
		[0x5] = NULL,
		[0x6] = NULL,
		[0x7] = NULL,
		[0x8] = NULL,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = l2_v55_f
	};

static u8_t *uarches_v56[] =
	{
		[0x0] = l2_v56_0,
		[0x1] = l2_v56_1,
		[0x2] = NULL,
		[0x3] = l2_v56_3,
		[0x4] = NULL,
		[0x5] = l2_v56_5,
		[0x6] = l2_v56_6,
		[0x7] = l2_v56_7,
		[0x8] = l2_v56_8,
		[0x9] = l2_v56_9,
		[0xa] = l2_v56_a,
		[0xb] = l2_v56_b,
		[0xc] = l2_v56_c,
		[0xd] = NULL,
		[0xe] = l2_v56_e,
		[0xf] = l2_v56_f
	};

static u8_t *uarches_v60[] =
	{
		[0x0] = l2_v60_0,
		[0x1] = l2_v60_1,
		[0x2] = l2_v60_2,
		[0x3] = NULL,
		[0x4] = l2_v60_4,
		[0x5] = l2_v60_5,
		[0x6] = NULL,
		[0x7] = NULL,
		[0x8] = NULL,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = l2_v60_f
	};

static u8_t *uarches_v61[] =
	{
		[0x0] = NULL,
		[0x1] = NULL,
		[0x2] = NULL,
		[0x3] = l2_v61_3,
		[0x4] = NULL,
		[0x5] = NULL,
		[0x6] = NULL,
		[0x7] = NULL,
		[0x8] = NULL,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = NULL,
	};

static u8_t *uarches_v62[] =
	{
		[0x0] = l2_v62_0,
		[0x1] = l2_v62_1,
		[0x2] = l2_v62_2,
		[0x3] = l2_v62_3,
		[0x4] = l2_v62_4,
		[0x5] = NULL,
		[0x6] = l2_v62_6,
		[0x7] = l2_v62_7,
		[0x8] = l2_v62_8,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = NULL,
	};

static u8_t *uarches_v65[] =
	{
		[0x0] = l2_v65_0,
		[0x1] = l2_v65_1,
		[0x2] = l2_v65_2,
		[0x3] = NULL,
		[0x4] = NULL,
		[0x5] = NULL,
		[0x6] = l2_v65_6,
		[0x7] = l2_v65_7,
		[0x8] = l2_v65_8,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = NULL,
	};

static u8_t *uarches_v66[] =
	{
		[0x0] = l2_v66_0,
		[0x1] = l2_v66_1,
		[0x2] = l2_v66_2,
		[0x3] = l2_v66_3,
		[0x4] = NULL,
		[0x5] = l2_v66_5,
		[0x6] = l2_v66_6,
		[0x7] = NULL,
		[0x8] = NULL,
		[0x9] = NULL,
		[0xa] = NULL,
		[0xb] = NULL,
		[0xc] = NULL,
		[0xd] = NULL,
		[0xe] = NULL,
		[0xf] = NULL,
	};

// small core. FIXME
#define uarches_v1 uarches_v65

static arch_t arches[] =
	{
		{0x01, uarches_v1},  // small core. FIXME
		{0x05, uarches_v5},
		{0x55, uarches_v55},
		{0x56, uarches_v56},
		{0x60, uarches_v60},
		{0x61, uarches_v61},
		{0x62, uarches_v62},
		{0x65, uarches_v65},
		{0x66, uarches_v66},
		{0x0, NULL}
	};

u32_t H2K_l2cache_init() {

	u32_t arch = H2K_gp->arch;
	u32_t uarch = H2K_gp->uarch;
	u32_t i = 0;
	u8_t *ptr;
	u32_t tag_size;

	// watermark lo = 24; watermark l2 = 24
	if (H2K_trap_hwconfig_setl2reg
			(0, NULL, L2REGS_QOS_SCOREBOARD_WATERMARK,
			 H2K_trap_hwconfig_getl2reg(0, NULL, L2REGS_QOS_SCOREBOARD_WATERMARK, 0, NULL)
			 | (24 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_BITS)
			 | (24 << L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_BITS),
			 NULL) == -1) {  // error
		return 0;
	}
	// qos mode tl = 1
	if (H2K_trap_hwconfig_setl2reg
			(0, NULL, L2REGS_QOS_MODE,
			 H2K_trap_hwconfig_getl2reg(0, NULL, L2REGS_QOS_MODE, 0, NULL)
			 | (1 << L2REGS_QOS_MODE_TL_BIT),
			 NULL) == -1) {  // error
		return 0;
	}

	while (arches[i].archv != arch) {
		if (0 == arches[i].archv) {  // whoa
			H2K_gp->kernel_error = KERROR_L2CACHE_INIT_ARCH;
			return 0;
		}
		i++;
	}
	ptr = (arches[i].ua)[uarch];

	if (NULL == ptr) {  // no table
		H2K_gp->kernel_error = KERROR_L2CACHE_INIT_UARCH;
		return 0;
	}

	tag_size = ptr[H2K_gp->l2arr];
	if (reserved == tag_size) {
		H2K_gp->kernel_error = KERROR_L2CACHE_INIT_SIZE;
		return 0;
	}

	if (H2K_gp->l2arr > CORE_REV_L2_CHUNK_SWITCH) {
		H2K_gp->l2size = (CORE_REV_L2_CHUNK_SWITCH * L2_CHUNK)
			+ ((H2K_gp->l2arr - CORE_REV_L2_CHUNK_SWITCH) * L2_BIG_CHUNK);
	} else {
		H2K_gp->l2size = H2K_gp->l2arr * L2_CHUNK;
	}

	if (H2K_trap_hwconfig_l2cache(0, NULL, tag_size, 1, NULL) == -1) {  // error
		H2K_gp->kernel_error = KERROR_L2CACHE_INIT_CONFIG;
		return 0;
	}
	H2K_gp->l2tags = tag_size;
	return tag_size;
}
