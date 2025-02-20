/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xis_ospi.c
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sd  10/18/24 First release
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xis_ospi.h"

#ifdef XIS_UPDATE_A_B_MECHANISM
#ifdef XIS_OSPI_FLSH

#include "xplmi.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"

/************************** Constant Definitions *****************************/

#ifndef SDT
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID
#endif

#define PAGE_COUNT		32
#define MAX_PAGE_SIZE 1024

/**************************** Type Definitions *******************************/

OspiFlashInfo Ospi_Flash_Config_Table[] = {
	/* Micron */
	{
		0x2c5b1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	{
		0x2c5b1b, FLASH_SECTOR_SIZE_128KB, 0x400, FLASH_PAGE_SIZE_256, 0x80000,
		FLASH_DEVICE_SIZE_1G, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	{
		0x2c5b1c, FLASH_SECTOR_SIZE_128KB, 0x800, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* GIGADEVICE */
	{
		0xc86819, FLASH_SECTOR_SIZE_64KB, 0x200, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFF0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* ISSI */
	{
		0x9d5b19, FLASH_SECTOR_SIZE_128KB, 0x100, FLASH_PAGE_SIZE_256, 0x20000,
		FLASH_DEVICE_SIZE_256M, 0xFFFE0000, 1,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/*is25lx512m*/
	{
		0x9d5a1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFE0000, 2,
		READ_CMD_OCTAL_IO_4B, (WRITE_CMD_OCTAL_4B << 8) | WRITE_CMD_4B,
		(DIE_ERASE_CMD << 16) | (BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B,
		READ_FLAG_STATUS_CMD, 16
	},
	/* MACRONIX */
	{
		0xc2813a, FLASH_SECTOR_SIZE_4KB, 0x4000, FLASH_PAGE_SIZE_256, 0x40000,
		FLASH_DEVICE_SIZE_512M, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/*mx66um2g45g*/
	{
		0xc2803c, FLASH_SECTOR_SIZE_4KB, 0x10000, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/*mx66uw2g345gxrix0*/
	{
		0xc2943c, FLASH_SECTOR_SIZE_4KB, 0x10000, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFFF000, 1,
		(READ_CMD_OPI_MX << 8) | READ_CMD_4B, WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_MX, READ_STATUS_CMD, 0
	},
	/* Infineon */
	/*s28hs02gt*/
	{
		0x345b1c, FLASH_SECTOR_SIZE_256KB, 0x400, FLASH_PAGE_SIZE_256, 0x100000,
		FLASH_DEVICE_SIZE_2G, 0xFFFC0000, 2,
		(DDR_READ_CMD_4B_SPN  << 8) | READ_CMD_4B, WRITE_CMD_4B,
		(BULK_ERASE_CMD << 8) | SEC_ERASE_CMD_4B, READ_STATUS_CMD, 0
	},
};

/***************** Macros (Inline Functions) Definitions *********************/

static INLINE u32 CalculateFCTIndex(u32 ReadId, u32 *FCTIndex)
{
	u32 Index;

	for (Index = 0; Index < sizeof(Ospi_Flash_Config_Table) / sizeof(Ospi_Flash_Config_Table[0]);
	     Index++) {
		if (ReadId == Ospi_Flash_Config_Table[Index].jedec_id) {
			*FCTIndex = Index;
			return XST_SUCCESS;
		}
	}

	return XST_FAILURE;
}

/************************** Function Prototypes ******************************/

#ifndef SDT
static int XIs_OspiPsvConfig(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId);
#else
static int XIs_OspiPsvConfig(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress);
#endif
static int XIs_FlashReadID(XOspiPsv *OspiPsvPtr);
static int XIs_FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
static int XIs_FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		 u8 *WriteBfrPtr);
static int XIs_FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		     u8 *WriteBfrPtr);
static int XIs_FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
	      u8 *WriteBfrPtr, u8 *ReadBfrPtr);
static u32 XIs_GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address);
static int XIs_BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
static int XIs_DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
static int XIs_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable);
static int XIs_FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode);
static int XIs_SpansionSetEccMode(XOspiPsv *OspiPsvPtr);
static int XIs_MxConfigDummy(XOspiPsv *OspiPsvPtr);
static int XIs_OspiFlashErase(u32 Address, u32 ByteCount);

/************************** Variable Definitions *****************************/
u8 TxBfrPtr;
u8 ReadBfrPtr[8]__attribute__ ((aligned(4)));
u32 FlashMake;
u32 FCTIndex;	/* Flash configuration table index */
u32 PageSize;
u32 FlashDeviceSize;
static XOspiPsv OspiPsvInstance;
static XOspiPsv_Msg FlashMsg;
u8 CmdBfr[8];
u8 FSRFlag;


/*****************************************************************************/
/**
*
* Main function to call the OSPIPSV Flash Polled example.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int XIs_OspiInit(void)
{
	int Status;

	/*
	 * Run the OspiPsv Polled example.
	 */
#ifndef SDT
	Status = XIs_OspiPsvConfig(&OspiPsvInstance, OSPIPSV_DEVICE_ID);
#else
	Status = XIs_OspiPsvConfig(&OspiPsvInstance, XPAR_XOSPIPSV_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Ospi Init failed\r\n");
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XOspiPsv
* device driver using OSPI flash devices greater than or equal to 128Mb.
* This function reads data in either DMA or DAC mode.
*
* @param	OspiPsvInstancePtr is a pointer to the OSPIPSV driver component to use.
* @param        OspiPsvDeviceId is the Device ID of the OSpi Device and is the
*               XPAR_<OSPI_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
#ifndef SDT
static int XIs_OspiPsvConfig(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId)
#else
static int XIs_OspiPsvConfig(XOspiPsv *OspiPsvInstancePtr, UINTPTR BaseAddress)
#endif
{
	int Status = XST_FAILURE;
	u8 UniqueValue;
	XOspiPsv_Config *OspiPsvConfig;

#ifdef VERSAL_AIEPG2
	Status = XPm_PmcRequestDevice(PM_DEV_OSPI);
#else
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_OSPI,
		((u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE), XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the OSPIPSV driver so that it's ready to use
	 */
#ifndef SDT
	OspiPsvConfig = XOspiPsv_LookupConfig(OspiPsvDeviceId);
#else
	OspiPsvConfig = XOspiPsv_LookupConfig(BaseAddress);
#endif
	if (NULL == OspiPsvConfig) {
		return XST_FAILURE;
	}

	Status = XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiPsvConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal) && !defined (VERSAL_NET)
	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
#else
	Status = XOspiPsv_DeviceResetViaOspi(OspiPsvInstancePtr, XOSPIPSV_HWPIN_RESET);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);
	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_12);

	Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = XIs_FlashReadID(OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Configure Flash ECC mode */
	if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Status = XIs_SpansionSetEccMode(OspiPsvInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Configure Flash Dummy cycles in flash device */
	if(FlashMake == MACRONIX_OCTAL_ID_BYTE0){
		Status = XIs_MxConfigDummy(OspiPsvInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/* Set Flash device and Controller modes */
	Status = XIs_FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = XIs_FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (OspiPsvInstancePtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		Ospi_Flash_Config_Table[FCTIndex].NumPage *= 2;
		Ospi_Flash_Config_Table[FCTIndex].NumSect *= 2;

		/* Reset the controller mode to NON-PHY */
		Status = XOspiPsv_SetSdrDdrMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Set Flash device and Controller modes */
		Status = XIs_FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			Status = XIs_FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	PageSize = Ospi_Flash_Config_Table[FCTIndex].PageSize;
	FlashDeviceSize = Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize;

	return Status;
}

int XIs_OspiWrite(u32 Address, u8 *WriteBfrPtr, u32 ByteCount)
{
	int Status = XST_FAILURE;
	int Page = 0;

	if(ByteCount%PageSize != 0) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Data write is not multiples of page size\n\r");
		return XST_FAILURE;
	}

	Status = XIs_OspiFlashErase(Address, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (XOspiPsv_GetOptions(&OspiPsvInstance) == XOSPIPSV_DAC_EN_OPTION) {
		Status = XIs_FlashLinearWrite(&OspiPsvInstance, Address,
					  (PageSize * ByteCount), WriteBfrPtr);
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "Flash linear write failed\n\r");
			return XST_FAILURE;
		}
	} else {
		for (Page = 0; Page < ByteCount/PageSize; Page++) {
			Status = XIs_FlashIoWrite(&OspiPsvInstance,
					      ((Page * PageSize) + Address),
					      PageSize, WriteBfrPtr);
			if (Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Flash IO write failed\n\r");
				return XST_FAILURE;
			}
		}
	}

	return Status;
}

int XIs_OspiRead(u32 SrcAddress, u8* DestAddress, u32 Length)
{
	int Status = XST_FAILURE;
	Status = XIs_FlashRead(&OspiPsvInstance, SrcAddress, Length,
			   CmdBfr, DestAddress);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Flash read failed\n\r");
		return XST_FAILURE;
	}

	return Status;
}

static int XIs_OspiFlashErase(u32 Address, u32 ByteCount)
{
	int Status = XST_FAILURE;
	Status = XIs_FlashErase(&OspiPsvInstance, Address, ByteCount, CmdBfr);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Flash erase failed\n\r");
		return XST_FAILURE;
	}

	return Status;
}

int XIs_FlashExit4BAddMod(void)
{
	int Status = XST_FAILURE;
	if (FlashDeviceSize > SIXTEENMB) {
		Status = XIs_FlashEnterExit4BAddMode(&OspiPsvInstance, 0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to determine the number of lines used
* for command, address and data
*
* @param	OspiPsvInstancePtr is a pointer to the OSPIPSV driver component to use.
* @param	Read is to tell whether a read or write
*
* @return	returns value to program the lines for command, address and data.
*
* @note		None.
*
*****************************************************************************/
u32 XOspiPsv_Get_Proto(XOspiPsv *OspiPsvInstancePtr, int Read)
{
	u32 Val;

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Val = XOSPIPSV_READ_1_1_1;
		return Val;
	}

	if (Read) {
		Val = XOSPIPSV_READ_1_8_8;
	}
	else {
		if (OspiPsvInstancePtr->OpMode == XOSPIPSV_IDAC_MODE) {
			Val = XOSPIPSV_WRITE_1_1_1;
		}
		else {
			Val = XOSPIPSV_WRITE_1_1_8;
		}
	}
	return Val;
}

/*****************************************************************************/
/**
*
* Reads the flash ID and identifies the flash in FCT table.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int XIs_FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status;
	int ReadIdBytes = 8;
	u32 ReadId = 0;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ReadIdBytes;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Dummy += 8;
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Addrsize = 4U;
		FlashMsg.Addrvalid = 1U;
		FlashMsg.Dummy = 4;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIs_Printf(XIS_DEBUG_INFO, "FlashID = ");
	while (ReadIdBytes >= 0 ) {
		XIs_Printf(XIS_DEBUG_INFO, "0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		ReadIdBytes--;
	}
	XIs_Printf(XIS_DEBUG_INFO, "\n\r");

	OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[3] << 24) | (ReadBfrPtr[2] << 16) |
				    (ReadBfrPtr[1] << 8) | ReadBfrPtr[0]);
	ReadId = ((ReadBfrPtr[0] << 16) | (ReadBfrPtr[1] << 8) | ReadBfrPtr[2]);

	FlashMake = ReadBfrPtr[0];

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		FSRFlag = 0;
	}
	else {
		FSRFlag = 1;
	}

	Status = CalculateFCTIndex(ReadId, &FCTIndex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used when controller
* is in Linear mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		     u8 *WriteBfrPtr)
{
	u8 Status;

	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = (u8)(Ospi_Flash_Config_Table[FCTIndex].WriteCmd >> 8);
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = WriteBfrPtr;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
	}
	FlashMsg.IsDDROpCode = 0;
	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		 u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 Bytestowrite;
	u32 RealAddr;

	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XIs_GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (ByteCount <= 8) {
			Bytestowrite = ByteCount;
			ByteCount = 0;
		}
		else {
			Bytestowrite = 8;
			ByteCount -= 8;
		}

		FlashMsg.Opcode = (u8)Ospi_Flash_Config_Table[FCTIndex].WriteCmd;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = WriteBfrPtr;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = Bytestowrite;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
		FlashMsg.Dummy = 0;
		FlashMsg.Addrsize = 4;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Addr = RealAddr;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		WriteBfrPtr += 8;
		Address += 8;

		while (1) {
			FlashMsg.Opcode = Ospi_Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			FlashMsg.Addr = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
				if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
					FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
					FlashMsg.Opcode = READ_CONFIG_REG_SPN;
					FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
					FlashMsg.Addrsize = 4;
					FlashMsg.Addrvalid = 1;
				}
			}

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial Flash connected to the
* OSPIPSV interface.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
	       u8 *WriteBfrPtr)
{
	int Sector;
	u32 NumSect;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 RealAddr;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Ospi_Flash_Config_Table[FCTIndex]).NumSect *
			  (Ospi_Flash_Config_Table[FCTIndex]).SectSize) ) {

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS0);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		if (Ospi_Flash_Config_Table[FCTIndex].NumDie == 1) {
			XIs_Printf(XIS_DEBUG_INFO, "EraseCmd 0x%x\n\r", (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 8));
			/*
			 * Call Bulk erase
			 */
			XIs_BulkErase(OspiPsvPtr, WriteBfrPtr);
		}

		if (Ospi_Flash_Config_Table[FCTIndex].NumDie > 1) {
			XIs_Printf(XIS_DEBUG_INFO, "EraseCmd 0x%x\n\r", (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 16));
			/*
			 * Call Die erase
			 */
			XIs_DieErase(OspiPsvPtr, WriteBfrPtr);
		}

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS1);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (Ospi_Flash_Config_Table[FCTIndex].NumDie == 1) {
				XIs_Printf(XIS_DEBUG_INFO, "EraseCmd 0x%x\n\r", (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 8));
				/*
				 * Call Bulk erase
				 */
				XIs_BulkErase(OspiPsvPtr, WriteBfrPtr);
			}

			if (Ospi_Flash_Config_Table[FCTIndex].NumDie > 1) {
				XIs_Printf(XIS_DEBUG_INFO, "EraseCmd 0x%x\n\r", (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 16));
				/*
				 * Call Die erase
				 */
				XIs_DieErase(OspiPsvPtr, WriteBfrPtr);
			}
		}
		return 0;
	}

	XIs_Printf(XIS_DEBUG_INFO, "EraseCmd 0x%x\n\r", (u8)Ospi_Flash_Config_Table[FCTIndex].EraseCmd);
	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount / (Ospi_Flash_Config_Table[FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if ( ((Address + ByteCount) & Ospi_Flash_Config_Table[FCTIndex].SectMask) ==
	     ((Address + (NumSect * Ospi_Flash_Config_Table[FCTIndex].SectSize)) &
	      Ospi_Flash_Config_Table[FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XIs_GetRealAddr(OspiPsvPtr, Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */

		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		FlashMsg.Opcode = (u8)Ospi_Flash_Config_Table[FCTIndex].EraseCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (1) {
			FlashMsg.Opcode = Ospi_Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			FlashMsg.Addr = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
				if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
					FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
					FlashMsg.Opcode = READ_CONFIG_REG_SPN;
					FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
					FlashMsg.Addrsize = 4;
					FlashMsg.Addrvalid = 1;
				}
			}

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
		Address += Ospi_Flash_Config_Table[FCTIndex].SectSize;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function performs read. DMA is the default setting.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	WriteBfrPtr is Pointer to the write buffer which contains data to be transmitted
* @param	ReadBfrPtr is Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
	      u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u8 Status;
	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;

	if ((Address < Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
	    ((Address + ByteCount) >= Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
	    (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED)) {
		BytesToRead = (Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	}
	else {
		BytesToRead = ByteCount;
	}
	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XIs_GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = (u8)Ospi_Flash_Config_Table[FCTIndex].ReadCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBfrPtr;
		FlashMsg.ByteCount = BytesToRead;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
		FlashMsg.Dummy = Ospi_Flash_Config_Table[FCTIndex].DummyCycles +
				 OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
			if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
				FlashMsg.Dummy = 20 + OspiPsvPtr->Extra_DummyCycle;
				FlashMsg.Opcode = (u8)(Ospi_Flash_Config_Table[FCTIndex].ReadCmd >> 8);
			}
			if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
				FlashMsg.Opcode = (u8)(Ospi_Flash_Config_Table[FCTIndex].ReadCmd >> 8);
				FlashMsg.Dummy = 21 + OspiPsvPtr->Extra_DummyCycle;
			}
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WriteBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg.Opcode = (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 8);
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (1) {
		FlashMsg.Opcode = Ospi_Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
			if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
				FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
				FlashMsg.Opcode = READ_CONFIG_REG_SPN;
				FlashMsg.Addr = STATUS_REG_1_ADDR_SPN;
				FlashMsg.Addrsize = 4;
				FlashMsg.Addrvalid = 1;
			}
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (FSRFlag) {
			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		else {
			if ((FlashStatus[0] & 0x01) == 0) {
				break;
			}
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WriteBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int XIs_DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	u8 DieCnt;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;

	for (DieCnt = 0; DieCnt < Ospi_Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		} else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		FlashMsg.Opcode = (u8)(Ospi_Flash_Config_Table[FCTIndex].EraseCmd >> 16);
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (1) {
			FlashMsg.Opcode = Ospi_Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
				FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (FSRFlag) {
				if ((FlashStatus[0] & 0x80) != 0) {
					break;
				}
			}
			else {
				if ((FlashStatus[0] & 0x01) == 0) {
					break;
				}
			}
		}
	}
	return 0;
}

/*****************************************************************************/
/**
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
static int XIs_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable)
{
	int Status;
	u8 Command;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0 || FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		return XST_SUCCESS;
	}

	if (Enable) {
		Command = ENTER_4B_ADDR_MODE;
	}
	else {
		Command = EXIT_4B_ADDR_MODE;
	}

	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = 0;
	FlashMsg.Addrsize = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 3;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (1) {
		FlashMsg.Opcode = Ospi_Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
		}

		if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
			FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
		}
		else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
			FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (FSRFlag) {
			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		else {
			if ((FlashStatus[0] & 0x01) == 0) {
				break;
			}
		}
	}

	switch (FlashMake) {
		case MICRON_OCTAL_ID_BYTE0:
			FlashMsg.Opcode = WRITE_DISABLE_CMD;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = NULL;
			FlashMsg.ByteCount = 0;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
			}

			if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			break;

		default:
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
* This API enters the flash device into Octal DDR mode or exit from octal DDR
* mode (switches to Extended SPI mode).
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Mode is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
static int XIs_FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode)
{
	int Status;
	u8 Write_Reg_Opcode = WRITE_CONFIG_REG;
	u8 Read_Reg_Opcode = READ_CONFIG_REG;
	u8 AddrSize = 3;
	u8 Dummy = 8;
	u32 Address = 0;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
#endif
	if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
		Write_Reg_Opcode = WRITE_CONFIG_REG_SPN;
		Read_Reg_Opcode = READ_CONFIG_REG_SPN;
		AddrSize = 4;
		Address = CONFIG_REG_5_ADDR_SPN;
		Dummy = 0;
	}

	if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		Write_Reg_Opcode = WRITE_CONFIG2_REG_MX;
		Read_Reg_Opcode = READ_CONFIG2_REG_MX;
		AddrSize = 4;
		if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			Dummy = 4;
			OspiPsvPtr->DeviceIdData = 0;
			OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[0] << 8) | ReadBfrPtr[0]);
			OspiPsvPtr->DeviceIdData |= ((ReadBfrPtr[1] << 24) | (ReadBfrPtr[1] << 16));
		}
		else {
			Dummy = 0;
			OspiPsvPtr->DeviceIdData = 0;
			OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[3] << 24) | (ReadBfrPtr[2] << 16) |
						    (ReadBfrPtr[1] << 8) | ReadBfrPtr[0]);
		}
	}

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Data[0] = 0x2;
			Data[1] = 0x2;
		}
		else if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
			Data[0] = 0x43;
			Data[1] = 0x43;
		}
		else {
			Data[0] = 0xE7;
			Data[1] = 0xE7;
		}
	}
	else {
		if (FlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Data[0] = 0x0;
			Data[1] = 0x0;
		}
		else if (FlashMake == SPANSION_OCTAL_ID_BYTE0) {
			Data[0] = 0x40;
			Data[1] = 0x40;
		}
		else {
			Data[0] = 0xFF;
			Data[1] = 0xFF;
		}
	}


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION) {
		XOspiPsv_ConfigureAutoPolling(OspiPsvPtr, Mode);
	}

	FlashMsg.Opcode = Write_Reg_Opcode;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		FlashMsg.Addrsize = 4;
		FlashMsg.ByteCount = 2;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Dummy += 8;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		if ((FlashMake == MACRONIX_OCTAL_ID_BYTE0) && (OspiPsvPtr->DualByteOpcodeEn == 0U)) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_ENABLE);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
		else if ((FlashMake == SPANSION_OCTAL_ID_BYTE0) && (OspiPsvPtr->DualByteOpcodeEn == 0U)) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_SAME);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}
	else {
		if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
			Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr, XOSPIPSV_DUAL_BYTE_OP_DISABLE);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read Configuration register */
	FlashMsg.Opcode = Read_Reg_Opcode;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		/* Read Configuration register */
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	} else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
		FlashMsg.Dummy += 8;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0]) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *			for stacked, the lower flash size is subtracted;
 *
 * @note	In addition to get the actual address to work on flash this
 *			function also selects the CS based on the configuration detected.
 *
 ******************************************************************************/
static u32 XIs_GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address)
{
	u32 RealAddr = Address;
	u8 Chip_Sel = XOSPIPSV_SELECT_FLASH_CS0;

	if ((OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) &&
	    (Address & Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize)) {
		Chip_Sel = XOSPIPSV_SELECT_FLASH_CS1;
		RealAddr = Address & (~Ospi_Flash_Config_Table[FCTIndex].FlashDeviceSize);
	}

	(void)XOspiPsv_SelectFlash(OspiPsvPtr, Chip_Sel);

	return RealAddr;
}

/*****************************************************************************/
/**
 *
 * This function configures the ECC mode in Flash device.
 * In some flashes by default 2-bit ECC is enabled, which prevents byte-programming( <16-Bytes).
 * This function enables byte-programming by setting the ECC configuration to 1-bit error
 * detection/correction.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 *
 * @return	XST_SUCCESS or XST_FAILURE.
 *
 * @note	This function is specific to some spansion flashes(S28HS02GT).
 *
 ******************************************************************************/
static int XIs_SpansionSetEccMode(XOspiPsv *OspiPsvPtr)
{
	int Status;
	u8 Write_Reg_Opcode = WRITE_CONFIG_REG_SPN;
	u8 Read_Reg_Opcode = READ_CONFIG_REG_SPN;
	u8 AddrSize = 4;
	u8 Dummy = 8;
	u32 Address = CONFIG_REG_4_ADDR_SPN;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
#endif

	Data[0] = 0xA0;
	Data[1] = 0xA0;

	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
	}

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = Write_Reg_Opcode;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		FlashMsg.Addrsize = 4;
		FlashMsg.ByteCount = 2;
	}

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read Configuration register */
	FlashMsg.Opcode = Read_Reg_Opcode;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = Address;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		/* Read Configuration register */
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (ConfigReg[0] != Data[0]) {
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function configures the Dummy cycles in Flash device.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 *
 * @return	XST_SUCCESS or XST_FAILURE.
 *
 ******************************************************************************/
static int XIs_MxConfigDummy(XOspiPsv *OspiPsvPtr){

	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
	u32 RegAddr=CONFIG_REG2_VOLATILE_ADDR_MX;
	u32 Status;
	Data[0]=0x00;
	Data[1]=0x00;

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = WRITE_CONFIG2_REG_MX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Addr = RegAddr;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.ByteCount = 2;
	}
	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Read back the configuration register 2 */
	FlashMsg.Opcode = READ_CONFIG2_REG_MX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = 4;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Addr = RegAddr;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.ByteCount = 2;
	}

	if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	else if (OspiPsvPtr->DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_SAME) {
		FlashMsg.ExtendedOpcode = (u8)(FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIs_Printf(XIS_DEBUG_INFO, "ConfigReg:0x%x \n",ConfigReg[0]);
	if (ConfigReg[0] != Data[0]) {
			return XST_FAILURE;
	}
	return Status;
}

#endif
#endif