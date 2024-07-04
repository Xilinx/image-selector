/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_singleimage.h
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
* 1.00  sd   06/04/24 First release
*
* </pre>
*
******************************************************************************/

#ifndef XIS_SINGLE_IMAGE_H
#define XIS_SINGLE_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xis_config.h"

#ifdef XIS_GET_BOARD_PARAMS

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XIs_ImageSelBoardParam(void);

#endif

#ifdef __cplusplus
}
#endif

#endif