/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_update_a_b.h
*
* This is the file which contains code for the ImgSel update A/B mechanism.
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
* 
*
* </pre>
*
******************************************************************************/
#ifndef XIS_UPDATE_A_B_H
#define XIS_UPDATE_A_B_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xis_config.h"

#ifdef XIS_UPDATE_A_B_MECHANISM

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XIS_PERS_REGISTER_BASE_ADDRESS				(0x100000U)
#define XIS_PERS_REGISTER_BACKUP_ADDRESS			(0x120000U)
#define XIS_SIZE_256B								(256U)
#define XIS_IMAGE_A									(0U)
#define XIS_IMAGE_B									(1U)
#define XIS_IDENTIFICATION_STRING					(0x4D554241)
#define XIS_IDENTIFICATION_STRING_OFFSET			(0x0U)
#define XIS_LENGTH_OF_REGISTERS						(0x4U)
#define XIS_LENGTH_OFFSET							(0x8U)
#define XIS_CHECKSUM_OFFSET							(0xCU)
#define XIS_LAST_BOOTED_IMAGE 						(0x10U)
#define	XIS_REQUESTED_BOOT_IMAGE					(0x11U)
#define XIS_IMAGE_B_BOOTABLE 						(0x12U)
#define XIS_IMAGE_A_BOOTABLE 						(0x13U)
#define XIS_IMAGE_A_OFFSET 							(0x14U)
#define XIS_IMAGE_B_OFFSET 							(0x18U)
#define XIS_RECOVERY_IMAGE_OFFSET 					(0x1CU)
#define XIS_XLNX_LENGTH								(0x4U)
#define XIS_XLNX_OFFSET								(0x24U)
#define XIS_RECOVERY_ADDRESS						(0x01E00000U)
#define XIS_SIZE_32KB								(32768U)

/************************** Function Prototypes ******************************/

int XIs_IsImageExist(u32 PartitionAddr);
int XIs_UpdateABMultiBootValue(void);

#endif

#ifdef __cplusplus
}
#endif

#endif