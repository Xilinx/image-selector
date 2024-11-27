/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xis_ospi.h
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sd  10/18/24 First release
*
*</pre>
*
******************************************************************************/
#ifndef XOSPIPSV_FLASH_CONFIG_H
#define XOSPIPSV_FLASH_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "xis_config.h"

#ifdef XIS_UPDATE_A_B_MECHANISM
#ifdef XIS_OSPI_FLSH

#ifndef SDT
#include "xparameters.h"	/* SDK generated parameters */
#endif
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xil_printf.h"
#include "xil_cache.h"



/************************** Constant Definitions *****************************/

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */

#define WRITE_STATUS_CMD	0x01
#define WRITE_DISABLE_CMD	0x04
#define WRITE_ENABLE_CMD	0x06
#define BULK_ERASE_CMD		0xC7
#define DIE_ERASE_CMD		0xC4
#define READ_ID			0x9F
#define READ_CONFIG_CMD		0x35
#define WRITE_CONFIG_CMD	0x01
#define READ_STATUS_CMD		0x05
#define READ_FLAG_STATUS_CMD	0x70
#define READ_CMD_4B			0x13
#define WRITE_CMD_4B		0x12
#define SEC_ERASE_CMD_4B	0xDC
#define SEC_ERASE_CMD_MX	0x21
#define READ_CMD_OPI_MX		0xEE
#define READ_CMD_OCTAL_IO_4B	0xCC
#define READ_CMD_OCTAL_DDR	0x9D
#define WRITE_CMD_OCTAL_4B	0x84
#define ENTER_4B_ADDR_MODE	0xB7
#define EXIT_4B_ADDR_MODE	0xE9
#define WRITE_CONFIG_REG	0x81
#define READ_CONFIG_REG		0x85
#define WRITE_CONFIG2_REG_MX	0x72
#define READ_CONFIG2_REG_MX		0x71
#define WRITE_CONFIG_REG_SPN	0x71
#define READ_CONFIG_REG_SPN	0x65
#define STATUS_REG_1_ADDR_SPN	0x800000
#define CONFIG_REG_4_ADDR_SPN	0x800005
#define CONFIG_REG_5_ADDR_SPN	0x800006
#define DDR_READ_CMD_4B_SPN	0xEE

/*
 * Sixteen MB
 */
#define SIXTEENMB 0x1000000

#define FLASH_PAGE_SIZE_256		256
#define FLASH_SECTOR_SIZE_4KB		0x1000
#define FLASH_SECTOR_SIZE_256KB 	0x40000
#define FLASH_SECTOR_SIZE_64KB		0x10000
#define FLASH_SECTOR_SIZE_128KB		0x20000
#define FLASH_DEVICE_SIZE_256M		0x2000000
#define FLASH_DEVICE_SIZE_512M		0x4000000
#define FLASH_DEVICE_SIZE_1G		0x8000000
#define FLASH_DEVICE_SIZE_2G		0x10000000

#define	MICRON_OCTAL_ID_BYTE0		0x2c
#define GIGADEVICE_OCTAL_ID_BYTE0	0xc8
#define ISSI_OCTAL_ID_BYTE0		0x9d
#define MACRONIX_OCTAL_ID_BYTE0		0xc2
#define SPANSION_OCTAL_ID_BYTE0		0x34

#define MICRON_BP_BITS_MASK		0x7C
#define CONFIG_REG2_VOLATILE_ADDR_MX 	0x00000300

/**************************** Type Definitions *******************************/

typedef struct {
	u32 jedec_id;	/* JEDEC ID */
	u32 SectSize;		/* Individual sector size or
						 * combined sector size in case of parallel config*/
	u32 NumSect;		/* Total no. of sectors in one/two flash devices */
	u32 PageSize;		/* Individual page size or
				 * combined page size in case of parallel config*/
	u32 NumPage;		/* Total no. of pages in one/two flash devices */
	u32 FlashDeviceSize;	/* This is the size of one flash device
				 * NOT the combination of both devices, if present
				 */
	u32 SectMask;		/* Mask to get sector start address */
	u8 NumDie;		/* No. of die forming a single flash */
	u32 ReadCmd;		/* Read command used to read data from flash */
	u32 WriteCmd;	/* Write command used to write data to flash */
	u32 EraseCmd;	/* Erase Command */
	u8 StatusCmd;	/* Status Command */
	u8 DummyCycles;	/* Number of Dummy cycles for Read operation */
} OspiFlashInfo;

int XIs_OspiInit(void);
int XIs_OspiWrite(u32 Address, u8 *WriteBfrPtr, u32 ByteCount);
int XIs_OspiRead(u32 SrcAddress, u8* DestAddress, u32 Length);
int XIs_FlashExit4BAddMod(void);

#endif /* end of XIS_UPDATE_A_B_MECHANISM */
#endif

#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_FLASH_CONFIG_H */
