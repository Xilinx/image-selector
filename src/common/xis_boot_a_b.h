/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_boot_a_b.h
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  sd   09/05/24 First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_BOOT_A_B_H
#define XIS_BOOT_A_B_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XIs_BootABImageBank(void);
void XIs_PrintMdata(void);
void XIs_UpdateBootPart(u32 bank_id, u8 trial);

#ifdef __cplusplus
}
#endif

#endif /* XIS_BOOT_A_B_H */
