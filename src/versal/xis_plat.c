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
#include "xplmi_err_common.h"

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
Boards_List Board[XIS_MAX_BOARDS] = {
	{"VPK120" , 1200},
	{"VMK180" , 1800},
	{"VCK190" , 1900},
	{"VPK180" , 1810},
	{"VEK280" , 2800}
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
	XPlmi_UtilRMW(PMC_GLOBAL_PMC_MULTI_BOOT, 0xFFFFFFFU, Offset);
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
	XPlmi_SoftResetHandler();
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