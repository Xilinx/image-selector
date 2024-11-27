/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_crc32.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  sd   27/11/23 First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_CRC32_H
#define XIS_CRC32_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xil_types.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XIs_CalculateCrc32(u8 *data, u32 len);

#ifdef __cplusplus
}
#endif

#endif /* XIS_CRC32_H */
