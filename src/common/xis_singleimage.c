/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_singleimage.c
*
* This is the file which contains code for the ImgSel single image.
*
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
* 1.00  Ana  24/06/20 First release
* 2.00  sd   06/04/24 Moved to common directory
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_singleimage.h"
#include "xis_i2c.h"
#include "xis_plat.h"

#ifdef XIS_GET_BOARD_PARAMS
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


static u8 ReadBuffer[XIS_MAX_SIZE]; /* Read buffer for reading a page. */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function is used to Get the board specific information from IIC EEPROM
 * @param	ReadAddress  Board specific parameters address location
 * 			Offset		 Multiboot offset value
 *			WrBfrOffset  No.Of IIC Address bytes
 *
 * @return	returns the error codes described in xpfsbl_error.h on any error
 *				returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XIs_GetBoardName(u16 ReadAddress, u16 *Offset,u32 WrBfrOffset)
{
	int Status = XST_FAILURE;
	u32 BoardIndex;

	/*
	 * Read from the EEPROM.
	 */
	Status = XIs_EepromReadData(ReadBuffer, ReadAddress,
					XIS_PAGE_SIZE, WrBfrOffset);
	if (Status != XST_SUCCESS) {
		Status = XIS_EEPROM_READ_ERROR;
		goto END;
	}

	for(BoardIndex = 0U; BoardIndex < XIS_MAX_BOARDS; BoardIndex++) {
		if(strncmp((char *)(ReadBuffer + XIS_BOARDNAME_OFFSET), (char *)XIs_GetBoardList(BoardIndex),
				XIS_BOARDNAME_SIZE) == 0U) {
			*Offset = XIs_GetBoardOffset(BoardIndex);
			goto END;
		}
	}
	Status = XIS_BOARD_NAME_NOTFOUND_ERROR;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to update the multiboot value
 * @param	None.
 *
 * @return	returns XIS_BOARD_NAME_NOTFOUND on failure
 *				returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XIs_ImageSelBoardParam(void)
{
	int Status = XST_FAILURE;
	u16 BoardOffset;

	Status = XIs_IicPsMuxInit();
	if (Status != XST_SUCCESS) {
		Status = XIS_MUX_INIT_ERROR;
		goto END;
	}

#ifdef VERSAL
	sleep(1);
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "***********Versal Image Selector***********\n\r");
#endif

	Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET_1,
					&BoardOffset, XIS_EEPROM_OFFSET_1_WRITE_BYTES);
	if(Status != XST_SUCCESS) {
		Status = XIs_GetBoardName(XIS_EEPROM_BOARD_ADDR_OFFSET_2,
					&BoardOffset, XIS_EEPROM_OFFSET_2_WRITE_BYTES);
		if(Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board Not Found\r\n");
			goto END;
		}
	}

	ReadBuffer[XIS_BOARDNAME_SIZE + 1U] = '\0';
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board name: %s\n\r", &ReadBuffer[XIS_BOARDNAME_OFFSET]);

	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Board offset: %x\r\n", BoardOffset);


#ifdef VERSAL
	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "*******************************************\n\r");
	sleep(1);
#endif

	XIs_UpdateMultiBootValue(BoardOffset);

END:
	return Status;
}
#endif
