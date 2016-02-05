/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <translate.h>
#include <linear.h>
#include <offset.h>
#include <pagewalk.h>

/* 
 * EJP: consider: having H2K_xxxx_transate also take vmblock and lookup
 * info->vmid to vmblock in parallel with info->transtype to func ptr.
 * 
 * Should H2K_translate be static inline?  
 * Let's start out this way for testing.  
 * I'm concerned about having H2K_translate_funcs static inline.
 * 
 * Is indirect branch slow enough that we should check types 
 * individually instead of funcptr?
 */

typedef H2K_translation_t (*translation_funcptr)(H2K_translation_t in, H2K_asid_entry_t info);

static const translation_funcptr H2K_translate_funcs[H2K_ASID_TRANS_TYPE_XXX_LAST] = {
	[H2K_ASID_TRANS_TYPE_LINEAR] = H2K_linear_translate,
        [H2K_ASID_TRANS_TYPE_TABLE] = H2K_pagewalk_translate,
        [H2K_ASID_TRANS_TYPE_OFFSET] = H2K_offset_translate,
};

H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	return H2K_translate_funcs[info.fields.type](in,info);
}

