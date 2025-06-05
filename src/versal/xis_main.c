/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_main.c
*
* This is the main file which contains code for Versal Image Selector.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_config.h"
#include "xis_i2c.h"
#include "xis_singleimage.h"
#include "xis_pm.h"
#include "xis_proc.h"
#include "xis_loader.h"
#include "xis_module.h"
#include "xplmi.h"
#include "xis_plat.h"

#ifdef XIS_UPDATE_A_B_MECHANISM
	#ifdef XIS_FWU_UPDATE
		#include "xis_boot_a_b.h"
	#else
		#include "xis_update_a_b.h"
	#endif
	#ifdef XIS_QSPI_FLSH
		#include "xis_qspi.h"
	#elif defined XIS_OSPI_FLSH
		#include "xis_ospi.h"
	#endif
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_IMAGESEL_MAJOR_VER 0x2
#define XIS_IMAGESEL_MINOR_VER 0x0
/************************** Function Prototypes ******************************/
static int XPlm_Init(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	Status = XPlm_Init();
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	Status = XPlm_ModuleInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_ProcessPmcCdo();
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	Status = XPlm_LoadBootPdi();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "ImageSelector Version: %x.%x\r\n"
					,XIS_IMAGESEL_MAJOR_VER, XIS_IMAGESEL_MINOR_VER );

#if defined (XIS_GET_BOARD_PARAMS)
	Status = XIs_ImageSelBoardParam();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#elif defined(XIS_UPDATE_A_B_MECHANISM)
#ifdef XIS_QSPI_FLSH
	Status = XIs_QspiInit();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Init failed\r\n");
		goto END;
	}
#elif defined XIS_OSPI_FLSH
	Status = XIs_OspiInit();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "OSPI Init failed\r\n");
		goto END;
	}
#endif
#ifdef XIS_FWU_UPDATE
	Status = XIs_BootABImageBank();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "FWU A/B Bank selection"
					" failed\r\n");
		goto END;
	}
#else
	Status = XIs_UpdateABMultiBootValue();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "A/B Image Multiboot"
							" value update failed\r\n");
		goto END;
	}
#endif
#endif

	XIs_Softreset();

	while(1U) {
		;
	}

END:
	/* Control should never reach here */
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes DMA, Run Time Config area, the processor
 * 		and task list structures.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_Init(void)
{
	int Status = XST_FAILURE;

	/**
	 * Reset the wakeup signal set by ROM
	 * Otherwise MB will always wakeup, irrespective of the sleep state
	 */
	XPlmi_PpuWakeUpDis();

	XPlmi_InitDebugLogBuffer();

	Status = XPlm_InitProc();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initializes the DMA pointers */
	Status = XPlmi_DmaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_RunTimeConfigInit();

END:
	return Status;
}
