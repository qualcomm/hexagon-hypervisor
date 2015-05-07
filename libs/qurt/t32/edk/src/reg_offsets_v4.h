/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef REG_OFFSETS_V4_H
#define REG_OFFSETS_V4_H

// Undefine all registers, so we don't accidentally pick up 
// a definition we don't want

#undef REG_EVB
#undef REG_IAHL
#undef REG_CFGBASE
#undef REG_DIAG
#undef REG_REV
#undef REG_PCYCLELO
#undef REG_PCYCLEHI
#undef REG_ISDBST
#undef REG_ISDBCFG0
#undef REG_ISDBCFG1
#undef REG_MODECTL
#undef REG_BRKPTPC0
#undef REG_BRKPTCFG0
#undef REG_BRKPTPC1
#undef REG_BRKPTCFG1
#undef REG_ISDBMBXIN
#undef REG_ISDBMBXOUT
#undef REG_ISDBEN
#undef REG_ISDBGPR
#undef REG_SYSCFG
#undef REG_PMUCNT0
#undef REG_PMUCNT1
#undef REG_PMUCNT2
#undef REG_PMUCNT3
#undef REG_PMUEVTCFG
#undef REG_PMUCFG
#undef REG_BRKPTINFO
#undef REG_STFINST
#undef REG_ISDBCMD
#undef REG_ISDBVER
#undef REG_AVS
#undef REG_RGDR
#undef REG_ACC0
#undef REG_ACC1
#undef REG_CHICKEN
#undef REG_IPEND
#undef REG_VID
#undef REG_IAD
#undef REG_IEL

#undef REG_R00
#undef REG_R01
#undef REG_R02
#undef REG_R03
#undef REG_R04
#undef REG_R05
#undef REG_R06
#undef REG_R07
#undef REG_R08
#undef REG_R09
#undef REG_R10
#undef REG_R11
#undef REG_R12
#undef REG_R13
#undef REG_R14
#undef REG_R15
#undef REG_R16
#undef REG_R17
#undef REG_R18
#undef REG_R19
#undef REG_R20
#undef REG_R21
#undef REG_R22
#undef REG_R23
#undef REG_R24
#undef REG_R25
#undef REG_R26
#undef REG_R27
#undef REG_R28
#undef REG_R29
#undef REG_R30
#undef REG_R31
#undef REG_SP
#undef REG_FP
#undef REG_LR
#undef REG_SA0
#undef REG_LC0
#undef REG_SA1
#undef REG_LC1
#undef REG_P3_0
#undef REG_M0
#undef REG_M1
#undef REG_USR
#undef REG_PC
#undef REG_UGP
#undef REG_GP
#undef REG_CS0
#undef REG_CS1
#undef REG_SGP
#undef REG_SGP0
#undef REG_SGP1
#undef REG_STID
#undef REG_ELR
#undef REG_BADVA0
#undef REG_BADVA1
#undef REG_SSR
#undef REG_CCR
#undef REG_HTID
#undef REG_BADVA
#undef REG_IMASK 
#undef REG_G0
#undef REG_G1
#undef REG_G2
#undef REG_G3

#define REG_EVB 0
#define REG_IAHL 10 
#define REG_CFGBASE 11
#define REG_DIAG 12
#define REG_REV 13
#define REG_PCYCLELO 14 
#define REG_PCYCLEHI 15
#define REG_ISDBST 16
#define REG_ISDBCFG0 17
#define REG_ISDBCFG1 18
#define REG_MODECTL 1
#define REG_BRKPTPC0 20
#define REG_BRKPTCFG0 21
#define REG_BRKPTPC1 22
#define REG_BRKPTCFG1 23
#define REG_ISDBMBXIN 24
#define REG_ISDBMBXOUT 25
#define REG_ISDBEN 26
#define REG_ISDBGPR 27
#define REG_SYSCFG 2
#define REG_PMUCNT0 32
#define REG_PMUCNT1 33
#define REG_PMUCNT2 34
#define REG_PMUCNT3 35
#define REG_PMUEVTCFG 36
#define REG_PMUCFG 37
#define REG_BRKPTINFO 38
#define REG_STFINST 39
#define REG_ISDBCMD 40
#define REG_ISDBVER 41
#define REG_AVS 43
#define REG_RGDR 44
#define REG_ACC0 45
#define REG_ACC1 46
#define REG_CHICKEN 47
#define REG_IPEND 4
#define REG_VID 5
#define REG_IAD 6
#define REG_IEL 8

#define REG_R00 0
#define REG_R01 1
#define REG_R02 2
#define REG_R03 3
#define REG_R04 4
#define REG_R05 5
#define REG_R06 6
#define REG_R07 7
#define REG_R08 8
#define REG_R09 9
#define REG_R10 10
#define REG_R11 11
#define REG_R12 12
#define REG_R13 13
#define REG_R14 14
#define REG_R15 15
#define REG_R16 16
#define REG_R17 17
#define REG_R18 18
#define REG_R19 19
#define REG_R20 20
#define REG_R21 21
#define REG_R22 22
#define REG_R23 23
#define REG_R24 24
#define REG_R25 25
#define REG_R26 26
#define REG_R27 27
#define REG_R28 28
#define REG_R29 29
#define REG_R30 30
#define REG_R31 31
#define REG_SP 29
#define REG_FP 30
#define REG_LR 31
#define REG_SA0 32
#define REG_LC0 33
#define REG_SA1 34
#define REG_LC1 35
#define REG_P3_0 36
#define REG_M0 38
#define REG_M1 39
#define REG_USR 40
#define REG_PC 41
#define REG_UGP 42
#define REG_GP 43
#define REG_CS0 44
#define REG_CS1 45
#define REG_SGP 64
#define REG_SGP0 64
#define REG_SGP1 65
#define REG_STID 66
#define REG_ELR 67
#define REG_BADVA0 68
#define REG_BADVA1 69
#define REG_SSR 70
#define REG_CCR 71
#define REG_HTID 72
#define REG_BADVA 73
#define REG_IMASK 74 
#define REG_G0 80
#define REG_G1 81
#define REG_G2 82
#define REG_G3 83

#endif /* REG_OFFSETS_V4_H */
