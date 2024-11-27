/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_plat.h
*
* This file contains Common API's and Macros used across the
* the application
*
*
******************************************************************************/

#ifndef XIS_COMMON_H
#define XIS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_util.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

typedef struct board
{
    char *Name; /* Name of the board */
    int Offset; /* Multiboot Offset value */
} Boards_List;

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_VERSAL_PLAT
#define XIS_CSU_MULTI_BOOT							(0xFFCA0010U)
#define XIS_CRL_APB_RESET_CTRL 						(0xFF5E0218U)
#define XIS_CSU_APB_RESET_VAL  						(0x10U)
#define XIS_ERROR_STATUS_REGISTER_OFFSET			(0xFFD80060U)

#define XIS_BOOTCNT_PERS_REG		(0xF1110054U)
#define XIS_BOOTPART_PERS_REG		(0xF111005CU)
#define XIS_ROLLBACK_PERS_REG		(0xF1110060U)
//#define XIS_WDT_PERS_REG			(0xF111005CU)
#define XIS_RESET_REASON_REG		(0xF1260220U)

#define XIs_In32(Addr)								Xil_In32(Addr)
#define XIs_Out32(Addr, Data)						Xil_Out32(Addr, Data)

#define XIS_MAX_BOARDS 				(5U)
#define XIS_BOARDNAME_SIZE 			(6U)
#define XIS_PAGE_SIZE				(0x20U)
#define XIS_BOARDNAME_OFFSET		(0x16U)
#define XIS_XLNX_OFFSET				(0x14U)

/* Board Parameters will present at 0x0 Address location
 * for all versal boards
 */
#define XIS_EEPROM_BOARD_ADDR_OFFSET_1			(0x00U)
#define XIS_EEPROM_BOARD_ADDR_OFFSET_2			(0x00U)

/************************** Function Prototypes ******************************/

void XIs_UpdateMultiBootValue(u32 Offset);
void XIs_Softreset(void);
char * XIs_GetBoardList(u8 index);
u32 XIs_GetBoardOffset(u8 index);

#ifdef __cplusplus
}
#endif

#endif
