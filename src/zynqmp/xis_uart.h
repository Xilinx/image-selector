/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xis_uart.h
 *
 * This is the main file which will contain the UART initialization
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 *
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XIS_UART_H
#define XIS_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xis_config.h"

#if defined(XIS_UART_ENABLE) && defined(STDOUT_BASEADDRESS)

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XIs_UartConfiguration(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
