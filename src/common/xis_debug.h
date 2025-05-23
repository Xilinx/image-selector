/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_debug.h
*
* This file contains the debug verbose information for ImgSel print functionality
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana  07/02/20 First release
* 2.00  sd   05/17/24 Removed redundant macro
* 2.01  sd   06/04/24 Moved to common directory
*
* </pre>
*
******************************************************************************/

#ifndef XIS_DEBUG_H
#define XIS_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_PRINT_VAL			(1U)
#define XIS_DEBUG_VAL			(1U)
#define XIS_DEBUG_DETAILED_VAL	(0U)

/**
 * ImgSel Debug options
 */

#if XIS_PRINT_VAL
#define XIS_PRINT
#endif

#if XIS_DEBUG_VAL
#define XIS_DEBUG
#endif

#if XIS_DEBUG_DETAILED_VAL
#define XIS_DEBUG_DETAILED
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * Debug levels for ImgSel
 */
#define XIS_DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define XIS_DEBUG_GENERAL	      (0x00000002U)    /* general debug  messages */
#define XIS_DEBUG_INFO			  (0x00000004U)    /* More debug information */
#define XIS_DEBUG_DETAILED_INFO	      (0x00000008U)    /* More debug information */

#if defined (XIS_DEBUG_DETAILED)
#define XImgSelDbgCurrentTypes ((XIS_DEBUG_DETAILED_INFO) | (XIS_DEBUG_INFO) | \
         (XIS_DEBUG_GENERAL) | (XIS_DEBUG_PRINT_ALWAYS))
#elif defined (XIS_DEBUG)
#define XImgSelDbgCurrentTypes ((XIS_DEBUG_GENERAL) | (XIS_DEBUG_PRINT_ALWAYS))
#elif defined (XIS_PRINT)
#define XImgSelDbgCurrentTypes (XIS_DEBUG_PRINT_ALWAYS)
#else
#define XImgSelDbgCurrentTypes (0U)
#endif
#define XIs_Printf(DebugType,...) \
		if(((DebugType) & XImgSelDbgCurrentTypes)!=XST_SUCCESS) {xil_printf (__VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif /* XIS_DEBUG_H */
