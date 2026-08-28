/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_module.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
* 2.00  sd   03/05/24 Remove argumet in XPlmi_EmInit function
* 3.00  aa   12/02/25 Update Xplmi_init function
* 4.00  sd   02/10/26 Enable AES/SHA init function for versal device
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_module.h"
#include "xplmi_config.h"
#include "xpm_api.h"
#include "xplmi_err_common.h"
#include "xplmi.h"
#include "xis_loader.h"
#include "xis_pm.h"
#include "xpm_mem.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	Arg is not used
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_ModuleInit(void)
{
	int Status = XST_FAILURE;

	Status = XPlmi_Init(XPm_IsMemAddressValid);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XPlm_PmInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_EmInit(XPm_SystemShutdown);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XLoader_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_AesShaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}
