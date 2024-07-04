/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_plat.c
*
* This is the file which contains error update functionality,
* multiboot value update functionality and soft reset functionality
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  		Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana   	18/06/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_plat.h"

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
Boards_List Board[XIS_MAX_BOARDS] = {
	{"ZCU102" , 1020U},
	{"ZCU104" , 1040U},
	{"ZCU106" , 1060U},
	{"ZCU111" , 1110U},
	{"ZCU208" , 2080U},
	{"ZCU216" , 2160U}
};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This functions updates the multiboot value into CSU_MULTIBOOT register.
 *
 * @param	Multiboot offset Value.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_UpdateMultiBootValue(u32 Offset)
{
	XIs_Out32(XIS_CSU_MULTI_BOOT, Offset);
}

/*****************************************************************************/
/**
 * This function will do soft reset.
 *
 * @param	None.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_Softreset(void)
{
	XIs_Out32(XIS_CRL_APB_RESET_CTRL, XIS_CSU_APB_RESET_VAL);
}

/*****************************************************************************/
/**
 * This function updates the error value into error status register.
 *
 * @param	Error Value.
 *
 * @return	None.
 *
 ******************************************************************************/
void XIs_UpdateError(int Error)
{
	XIs_Out32(XIS_ERROR_STATUS_REGISTER_OFFSET, ((u32)Error << 16U));
}

/*****************************************************************************/
/**
 * This function returns board name from supported board list.
 *
 * @param	Index
 *
 * @return	Board name
 *
 ******************************************************************************/
char * XIs_GetBoardList(u8 Index)
{
	return Board[Index].Name;
}

/*****************************************************************************/
/**
 * This function returns board offset value from supported board list.
 *
 * @param	Index
 *
 * @return	Board offset value
 *
 ******************************************************************************/
u32 XIs_GetBoardOffset(u8 Index)
{
	return Board[Index].Offset;
}