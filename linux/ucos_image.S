/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

.global ucos_image_start
.p2align 2
ucos_image_start:
.incbin "ucos.bin"
.global ucos_image_end
ucos_image_end:
