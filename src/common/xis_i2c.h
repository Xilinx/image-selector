/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_i2c.h
*
* This is the main header file which contains definitions for the i2c.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana  07/02/20 First release
* 2.00  sd   06/04/24 Moved to common directory
*
* </pre>
*
******************************************************************************/

#ifndef XIS_I2C_H
#define XIS_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xis_config.h"

#ifdef XIS_GET_BOARD_PARAMS
#include "xiicps.h"

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/*
 * The AddressType should be u8 as the address pointer in the on-board
 * EEPROM is 1 byte.
 */
typedef u16 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_EEPROM_OFFSET_1_WRITE_BYTES   		(0x01U)
#define XIS_EEPROM_OFFSET_2_WRITE_BYTES  		(0x02U)
#define XIS_IIC_SCLK_RATE						(100000U)
#define XIS_MAX_SIZE							(0x100)

/************************** Function Prototypes ******************************/
/*
 * Functions defined in xis_i2c.c
 */
int XIs_IicPsMuxInit(void);
int XIs_EepromReadData(u8 *BufferPtr, u16 ReadAddress,
						u16 ByteCount, u32 WrBfrOffset);

#endif /* end of XIS_GET_BOARD_PARAMS */

#ifdef __cplusplus
}
#endif

#endif
