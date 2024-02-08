/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <globals.h>
#include <hvx.h>

H2K_kg_t H2K_kg;

void FAIL(const char *str)
{
    puts("FAIL");
    puts(str);
    exit(1);
}

//TODO:REDO for HLX

int main()
{
    __asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
    H2K_kg.info_boot_flags.boot_have_hvx = 1;
    
    H2K_kg.hvx_state = H2K_HVX_STATE_OFF;
    H2K_hvx_poweron(); // attempt hvx power on
    if (H2K_kg.hvx_state != H2K_HVX_STATE_ON) FAIL("hvx_poweron");
    H2K_hvx_poweroff(); // re-attempt hvx power on to check no breakage
    
    H2K_kg.hvx_state = H2K_HVX_STATE_ON;
    H2K_hvx_poweroff(); // attempt hvx power off
    if (H2K_kg.hvx_state != H2K_HVX_STATE_OFF) FAIL("hvx_poweroff");    
    H2K_hvx_poweroff(); // re-attempt hvx power off to check no breakage
    
    puts("TEST PASSED");
    return 0;
}
