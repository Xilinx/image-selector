/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_update_a_b.c
*
* This is the file which contains code for the ImgSel update A/B mechanism.
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
* 2.00  SD   13/03/24 Added function to check if boot image
*                     exist in partitions A and B
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_update_a_b.h"
#ifdef XIS_QSPI_FLSH
#include "xis_qspi.h"
#elif defined XIS_OSPI_FLSH
#include "xis_ospi.h"
#endif
#ifdef XIS_VERSAL_PLAT
#include "xplmi.h"
#endif
#include "xis_plat.h"

#ifdef XIS_UPDATE_A_B_MECHANISM
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_LENGTH	(0x8U)
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * This function is used to calculate the checksum of QSPI registers data
 * @param	PersData QSPI registers data.
 *
 * @return	returns checksum of QSPI registers data
 *
 ******************************************************************************/
static u32 XIs_CheckSumCalculation(u32 *PersData, u8 Update)
{
	u32 Index = 0U;
	u32 ChkSum = 0U;

	for(Index = 0U; Index < XIS_LENGTH; Index++) {
		if( Index != (XIS_CHECKSUM_OFFSET / 4U)) {
			ChkSum += PersData[Index];
		}
	}
	ChkSum  = 0xFFFFFFFFU - ChkSum;

	if(Update == TRUE) {
		PersData[XIS_CHECKSUM_OFFSET / 4U] = ChkSum;
	}

	return ChkSum;
}

/*****************************************************************************/
/**
 * This function is used to validate the QSPI registers data
 * @param	ReadDataBuffer QSPI registers data.
 *
 * @return	returns XST_FAILURE on failure
 *			returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
static int XIs_DataValidations(u8 *ReadDataBuffer)
{
	int Status = XST_FAILURE;
	u32 ChkSum = 0U;
	u32 *PersRegData = (u32 *)ReadDataBuffer;

	if(PersRegData[XIS_IDENTIFICATION_STRING_OFFSET] !=
								XIS_IDENTIFICATION_STRING) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Identification String:%x\r\n",
						PersRegData[XIS_IDENTIFICATION_STRING_OFFSET]);
		Status = XIS_IDEN_STRING_MISMATCH_ERROR;
		goto END;
	}
	if(PersRegData[XIS_LENGTH_OFFSET / 4U] != XIS_LENGTH_OF_REGISTERS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Length:%x\r\n",
									PersRegData[XIS_LENGTH_OFFSET / 4U]);
		Status = XIS_REGISTERS_LENGTH_MISMATCH_ERROR;
		goto END;
	}
	ChkSum = XIs_CheckSumCalculation(PersRegData, (u8)FALSE);
	if(ChkSum != PersRegData[XIS_CHECKSUM_OFFSET / 4U]) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Chksum:%08x", ChkSum);
		Status = XIS_CHECKSUM_MISMATCH_ERROR;
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function checks if valid image exists in partition address
 * @param	PartitionAddr of QSPI flash
 *
 * @return	returns XST_FAILURE if image doesn't exist else
 *			XST_SUCCESS
 *
 *
 ******************************************************************************/
int XIs_IsImageExist(u32 PartitionAddr)
{
	int Status = XST_FAILURE;
	u8 DataBuff[XIS_SIZE_1KB] __attribute__ ((aligned(32U)));

#ifdef XIS_QSPI_FLSH
	Status = XIs_QspiRead(PartitionAddr, (u8 *)DataBuff, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
	Status = XIs_OspiRead(PartitionAddr, (u8 *)DataBuff, XIS_SIZE_1KB);
#endif
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed\r\n");
		goto END;
	}

	/*Check if string XNLX exists in boot header*/
	if(strncmp((char *)&DataBuff[XIS_XLNX_OFFSET], "XNLX", \
		XIS_XLNX_LENGTH) == 0){
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

END:
	return Status;

}

/*****************************************************************************/
/**
 * This function is used to update the multiboot value
 * @param	None.
 *
 * @return	returns error value defined in xis_error.h file on failure
 *			returns XST_SUCCESS on success
 *
 *
 ******************************************************************************/
int XIs_UpdateABMultiBootValue(void)
{
	int Status = XST_FAILURE;
	u32 *PerstRegPtr;
	u32 Offset;
	u8 CurrentImage;
	u8 ReadDataBuffer[XIS_SIZE_1KB] __attribute__ ((aligned(32U)));

#ifdef XIS_QSPI_FLSH
	Status = XIs_QspiRead(XIS_MDATA_OFFSET,
					(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
	Status = XIs_OspiRead(XIS_MDATA_OFFSET,
					(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#endif
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed\r\n");
		goto END;
	}
	Status = XIs_DataValidations(ReadDataBuffer);
	if(Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Data Validations Failed\r\n");
		goto END;
	}

	/**
	 * If Image A and Image B are both are marked as bootable,
	 * Requested image will be loaded
	 * If Image A or Image B any one of the image is marked as bootable,
	 * Requested image is not bootable bootable image is loaded
	 * If Last Image booted is non-bootable, bootable image will be loaded
	 * If None of the Image is marked as bootable, recovery image is loaded
	 */
	if((ReadDataBuffer[XIS_IMAGE_A_BOOTABLE] == TRUE) ||
			(ReadDataBuffer[XIS_IMAGE_B_BOOTABLE] == TRUE)) {
		if((ReadDataBuffer[XIS_LAST_BOOTED_IMAGE] == XIS_IMAGE_A) &&
					(ReadDataBuffer[XIS_IMAGE_A_BOOTABLE] == FALSE)) {
			CurrentImage = XIS_IMAGE_B;
			PerstRegPtr = (u32 *)&ReadDataBuffer[XIS_IMAGE_B_OFFSET];
			Status = XIs_IsImageExist(*PerstRegPtr);
			if(Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Image does not exist in Partition B, Launching Image recovery\r\n");
				goto RCRY;
			}
		}
		else if((ReadDataBuffer[XIS_LAST_BOOTED_IMAGE]
					== XIS_IMAGE_B) &&
					(ReadDataBuffer[XIS_IMAGE_B_BOOTABLE] == FALSE)){
			CurrentImage = XIS_IMAGE_A;
			PerstRegPtr = (u32 *)&ReadDataBuffer[XIS_IMAGE_A_OFFSET];
			Status = XIs_IsImageExist(*PerstRegPtr);
			if(Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Image does not exist in Partition A, Launching Image recovery\r\n");
				goto RCRY;
			}
		}
		else {
			if(ReadDataBuffer[XIS_REQUESTED_BOOT_IMAGE]
					== XIS_IMAGE_A) {
				CurrentImage = XIS_IMAGE_A;
				PerstRegPtr = (u32 *)&ReadDataBuffer[XIS_IMAGE_A_OFFSET];
				Status = XIs_IsImageExist(*PerstRegPtr);
				if(Status != XST_SUCCESS) {
					XIs_Printf(XIS_DEBUG_GENERAL, "Image does not exist in Partition A, Launching Image recovery\r\n");
					goto RCRY;
				}
			}
			else {
				CurrentImage = XIS_IMAGE_B;
				PerstRegPtr = (u32 *)&ReadDataBuffer[XIS_IMAGE_B_OFFSET];
				Status = XIs_IsImageExist(*PerstRegPtr);
				if(Status != XST_SUCCESS) {
					XIs_Printf(XIS_DEBUG_GENERAL, "Image does not exist in Partition B, Launching Image recovery\r\n");
					goto RCRY;
				}
			}
		}
	}
	else {
		goto RCRY;
	}

	Offset = (u32)(*PerstRegPtr / XIS_SIZE_32KB);
	XIs_UpdateMultiBootValue(Offset);

	if(ReadDataBuffer[XIS_LAST_BOOTED_IMAGE] != CurrentImage) {
		ReadDataBuffer[XIS_LAST_BOOTED_IMAGE] = CurrentImage;
		(void)XIs_CheckSumCalculation((u32*)ReadDataBuffer, (u8)TRUE);
#ifdef XIS_QSPI_FLSH
		Status = XIs_QspiWrite(XIS_MDATA_OFFSET,
						(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
		Status = XIs_OspiWrite(XIS_MDATA_OFFSET,
						(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#endif
		if(Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Last image booted"
                                  " Write failed\r\n");
			goto END;
		}

#ifdef XIS_QSPI_FLSH
		Status = XIs_QspiWrite(XIS_MDATA_BKP_OFFSET,
						(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
		Status = XIs_OspiWrite(XIS_MDATA_BKP_OFFSET,
						(u8 *)ReadDataBuffer, XIS_SIZE_1KB);
#endif
		if(Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Last image booted"
                                  " Backup Write failed\r\n");
			goto END;
		}
	}

END:
	return Status;

RCRY:
	PerstRegPtr = (u32 *)&ReadDataBuffer[XIS_RECOVERY_IMAGE_OFFSET];
	Offset = (u32)(*PerstRegPtr / XIS_SIZE_32KB);
	XIs_UpdateMultiBootValue(Offset);
	return XST_SUCCESS;
}
#endif
