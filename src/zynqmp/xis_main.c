/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_main.c
*
* This is the main file which contains code for the ImgSel.
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
* 1.00  Ana  07/02/20 First release
* 1.01  Ana  18/06/20 Added QSPI support
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_config.h"
#include "psu_init.h"
#include "xis_plat.h"
#include "xis_uart.h"


#if defined(XIS_GET_BOARD_PARAMS)
#include "xis_singleimage.h"
#endif

#if defined(XIS_UPDATE_A_B_MECHANISM)
#include "xis_qspi.h"
#ifdef XIS_FWU_UPDATE
#include "xis_boot_a_b.h"
#else
#include "xis_update_a_b.h"
#endif
#if defined(XPAR_XGPIOPS_NUM_INSTANCES)
#include "xis_gpio.h"
#endif
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This is the ImgSel main function and is implemented to support
* A/B update mechanism and get board params mechanism
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;
	u32 MultiBootVal;
#if defined(XIS_UPDATE_A_B_MECHANISM) && defined(XPAR_XGPIOPS_NUM_INSTANCES)
	u32 GpioStatus;
	u32 OffsetVal;
#endif

	Status = Psu_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#if defined(XIS_UART_ENABLE) && defined(STDOUT_BASEADDRESS)
	Status = XIs_UartConfiguration();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "Image Selector boot Started\r\n");

#if defined(XIS_GET_BOARD_PARAMS)
	Status = XIs_ImageSelBoardParam();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Single Image Multiboot"
							"value update failed\r\n");
		goto END;
	}
#elif defined(XIS_UPDATE_A_B_MECHANISM)
	Status = XIs_QspiInit();
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Init failed\r\n");
		goto END;
	}
#if defined(XPAR_XGPIOPS_NUM_INSTANCES)
	Status = GpioInit();
	if(Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Gpio Init Failed\r\n");
	}
	GpioStatus = GetGpioStatus();
	if(GpioStatus == 0U) {
		OffsetVal = (u32)(XIS_RECOVERY_OFFSET / XIS_SIZE_32KB);
		XIs_UpdateMultiBootValue(OffsetVal);
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
#else
	MultiBootVal = XIs_In32(XIS_CSU_MULTI_BOOT);
	(void)XIs_UpdateMultiBootValue(MultiBootVal + 1U);
#endif

END:
	if (Status != XST_SUCCESS) {
		(void)XIs_UpdateError(Status);
		MultiBootVal = XIs_In32(XIS_CSU_MULTI_BOOT);
		(void)XIs_UpdateMultiBootValue(MultiBootVal + 1U);
	}

	dsb();

	isb();

	XIs_Softreset();

	while(1U) {
		;
	}

	return Status;
}
