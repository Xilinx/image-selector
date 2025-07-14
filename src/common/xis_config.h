/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_config.h
*
* This is the header file which contains all configuration data.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana  06/24/20 First release
* 2.00  td   06/10/21 Added check to ensure board param option is only enabled
*                     when UPDATE_A_B option is disabled and vice-versa
* 3.00  sd   06/04/24 Moved to common directory
*
* </pre>
*
******************************************************************************/

#ifndef XIS_CONFIG_H
#define XIS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xis_debug.h"
#include "xis_error.h"
#include "xis_plat.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_XLNX_LENGTH              0x4U
#define XIS_SIZE_1KB                 0x400U
#define XIS_SIZE_32KB                0x8000U

/*
 * Below mentioned macros support two different mechanisms
 * We will support any one mechanism at a time
 *
 * XIS_UPDATE_A_B_MECHANISM
 * It will read the Meta data from QSPI/OSPI and based on it
 * will update the multiboot value
 *
 * XIS_GET_BOARD_PARAMS
 * It will read the board name from EEPROM and based on board name
 * it will update the multiboot value
 *
 */

#ifdef XIS_VERSAL_PLAT
#define XIS_UPDATE_A_B_MECHANISM
#define XIS_FWU_UPDATE
//#define XIS_QSPI_FLSH
#define XIS_OSPI_FLSH
#else
#define XIS_UPDATE_A_B_MECHANISM
//#define XIS_GET_BOARD_PARAMS
//#define XIS_FWU_UPDATE
#define XIS_QSPI_FLSH
//#define XIS_UART_ENABLE
#endif

#ifdef XIS_FWU_UPDATE
#define XIS_MDATA_OFFSET             0x1500000U
#define XIS_MDATA_BKP_OFFSET         0x1520000U
#define XIS_BANK_0_OFFSET            0x1580000U
#ifdef XIS_512G_FLASH
#define XIS_BANK_1_OFFSET            0x25C0000U
#elif XIS_1G_FLASH
#define XIS_BANK_1_OFFSET            0x47C0000U
#else
#define XIS_BANK_1_OFFSET            0x87C0000U
#endif
#define XIS_RECOVERY_OFFSET          0xE0000U
#else
#define XIS_MDATA_OFFSET             0x100000U
#define XIS_MDATA_BKP_OFFSET         0x120000U
#define XIS_BANK_0_OFFSET            0x200000U
#define XIS_BANK_1_OFFSET            0xF80000U
#define XIS_RECOVERY_OFFSET          0x1E00000U
#endif

#ifdef XPAR_XIICPS_NUM_INSTANCES
#ifndef XIS_UPDATE_A_B_MECHANISM
#define XIS_GET_BOARD_PARAMS
#endif
#define XIS_MUX_ADDR                 0x74U
#define XIS_I2C_MUX_INDEX            0x1U
#define XIS_EEPROM_ADDRESS           0x54U
#endif

#ifdef __cplusplus
}
#endif

#endif
