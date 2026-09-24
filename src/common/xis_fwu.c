/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_fwu.c
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
* 1.00  sd   26/04/24 First release
* 2.00  sd   10/07/26 Fix infinite loop in XIs_SearchOptionalData on zero length
* 3.00  aa   09/23/26 Parse and display BIF Optional Data fields (platform,
*                     version, release and manifest) from the boot image
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xis_config.h"
#include "xis_plat.h"
#include "xis_fwu.h"
#include "xis_crc32.h"
#ifdef XIS_VERSAL_PLAT
#include "xilpdi.h"
#endif
#ifdef XIS_QSPI_FLSH
#include "xis_qspi.h"
#elif defined XIS_OSPI_FLSH
#include "xis_ospi.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
u8 ReadBuff[XIS_SIZE_1KB] __attribute__ ((aligned(32U)));
u32 MaxBootCnt;

#ifdef XIS_VERSAL_PLAT
/* BIF Optional Data fields to parse and display from the boot image */
typedef struct {
	u32 DataId;
	const char *Label;
} XIs_OptDataInfo;

static const XIs_OptDataInfo XIs_BootImgInfo[] = {
	{ XIS_OPT_DATA_ID_1, "Boot Firmware         " },
	{ XIS_OPT_DATA_ID_2, "Image Selector Version" },
	{ XIS_OPT_DATA_ID_3, "Release Version       " },
	{ XIS_OPT_DATA_ID_4, "Build info            " },
};
#endif
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function initilizes boot counter value and increments
 * for soft reset
 *
 * @return	none
 *
 ******************************************************************************/

void XIs_InitBootCnt(void)
{
	if(XIs_In32(XIS_RESET_REASON_REG) == XIS_PS_POR_B_RESET) {
		XIs_ResetBootCnt();
	} else {
		XIs_IncBootCnt();
	}
}

/*****************************************************************************/
/**
 * This function reads boot counter value form the register
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_ReadBootCnt(void)
{
	u32 BootCnt;

	BootCnt = (XIs_In32(XIS_PMC_GLOB_PERS_REG) >> XIS_BOOT_CNT_BYT) & 0xFF;
	if(BootCnt >= MaxBootCnt) {
		BootCnt = XIs_SetBootCnt(XIS_DEFAULT_VAL);
	}
	XIs_Printf(XIS_DEBUG_GENERAL, "Boot Count: %x\r\n", BootCnt);
	XIs_Printf(XIS_DEBUG_GENERAL, "MaxBootCnt: %x\r\n", MaxBootCnt);
}

/*****************************************************************************/
/**
 * This function check if boot counter exceeds maximum allowed
 * counter value
 *
 * @return	true or false
 *
 ******************************************************************************/
u32 XIs_ValidateBootCnt(void)
{
	u32 BootCnt;
	BootCnt = (XIs_In32(XIS_PMC_GLOB_PERS_REG) >> XIS_BOOT_CNT_BYT) & 0xFF;

	if(BootCnt <= MaxBootCnt) {
		return XIS_TRUE;
	} else {
		return XIS_FALSE;
	}
}

/*****************************************************************************/
/**
 * This function resets boot counter value
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_ResetBootCnt(void)
{
	u32 PerRegVal;

	XIs_Out32(XIS_PMC_GLOB_PERS_REG, XIS_PMC_REG_MAGIC_NUM);

	PerRegVal = XIs_In32(XIS_PMC_GLOB_PERS_REG);
	PerRegVal = (PerRegVal & ~(0xFF << XIS_BOOT_CNT_BYT));

	XIs_Out32(XIS_PMC_GLOB_PERS_REG, PerRegVal);
}

/*****************************************************************************/
/**
 * This function increments boot counter value
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_IncBootCnt(void)
{
	u32 PerRegVal;
	u32 BootCnt;

	BootCnt = (XIs_In32(XIS_PMC_GLOB_PERS_REG) >> XIS_BOOT_CNT_BYT) & 0xFF;
	BootCnt++;

	PerRegVal = XIs_In32(XIS_PMC_GLOB_PERS_REG);
	PerRegVal = ((PerRegVal & ~(0xFF << XIS_BOOT_CNT_BYT)) | (BootCnt << XIS_BOOT_CNT_BYT));

	XIs_Out32(XIS_PMC_GLOB_PERS_REG, PerRegVal);
}

/*****************************************************************************/
/**
 * This function sets boot counter value
 *
 * @param	set_val, value to be set
 *
 * @return	boot counter value
 *
 ******************************************************************************/
u32 XIs_SetBootCnt(u32 set_val)
{

	u32 PerRegVal;
	u32 BootCnt;

	PerRegVal = XIs_In32(XIS_PMC_GLOB_PERS_REG);
	PerRegVal = ((PerRegVal & ~(0xFF << XIS_BOOT_CNT_BYT)) | (set_val << XIS_BOOT_CNT_BYT));

	BootCnt = (XIs_In32(XIS_PMC_GLOB_PERS_REG) >> XIS_BOOT_CNT_BYT) & 0xFF;

	return BootCnt;
}

/*****************************************************************************/
/**
 * This function reads meta data stored in the qspi flash and
 * validates its crc32 checksum
 *
 * @param	mdata
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_ReadMetaData(struct fwu_mdata *mdata)
{
	u32 Status = XST_FAILURE;

#ifdef XIS_QSPI_FLSH
	Status = XIs_QspiRead(XIS_MDATA_OFFSET, (u8 *)ReadBuff, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
	Status = XIs_OspiRead(XIS_MDATA_OFFSET, (u8 *)ReadBuff, XIS_SIZE_1KB);
#endif
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed for Meta data\r\n");
		return Status;
	}

	Status = XIs_ValidateMetaData(mdata, ReadBuff);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "CRC32 Checksum failed for Meta data\r\n");
		XIs_Printf(XIS_DEBUG_GENERAL, "Reading Backup Meta data\r\n");

#ifdef XIS_QSPI_FLSH
		Status = XIs_QspiRead(XIS_MDATA_BKP_OFFSET, (u8 *)ReadBuff, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
		Status = XIs_OspiRead(XIS_MDATA_BKP_OFFSET, (u8 *)ReadBuff, XIS_SIZE_1KB);
#endif
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed for Backup Meta data\r\n");
			return Status;
		}

		Status = XIs_ValidateMetaData(mdata, ReadBuff);
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "CRC32 Checksum failed for Backup Meta data\r\n");
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function calculates and validates against actual crc32 checksum
 *
 * @param	mdata
 * @param	data_buff
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_ValidateMetaData(struct fwu_mdata *mdata, u8 *data_buff)
{
	u32 Status = XST_FAILURE;
	u32 MdataSize;
	u32 MdataVer;
	u32 *AddrPtr;
	u32 CalCrc32;
	u32 ActCrc32;

	AddrPtr = (u32*)&data_buff[XIS_MDATA_VER_OFFSET];
	MdataVer = *AddrPtr;

	if(MdataVer != XIS_FWU_MDATA_VER){
		XIs_Printf(XIS_DEBUG_GENERAL, "Mdata version check failed\r\n");
		return Status;
	}

	AddrPtr = (u32*)&data_buff[XIS_MDATA_SIZE_OFFSET];
	MdataSize = *AddrPtr;

	if(MdataSize > XIS_SIZE_1KB){
		XIs_Printf(XIS_DEBUG_GENERAL, "Invalid Mdata size\r\n");
		return Status;
	}

	AddrPtr = (u32*)&data_buff[XIS_CRC32_OFFSET];
	ActCrc32 = *AddrPtr;

	CalCrc32 = XIs_CalculateCrc32(&data_buff[XIS_MDATA_START_OFFSET],
						MdataSize-XIS_CRC32_SIZE);

	if(CalCrc32 == ActCrc32) {
		/* load meta data structure */
		memcpy(mdata, data_buff, MdataSize);
		AddrPtr = (u32*)&data_buff[MdataSize-XIS_BOOTCNT_SIZE];
		MaxBootCnt = *AddrPtr;
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function reads the bank status
 *
 * @param	mdata
 *
 * @param	bank_id
 *
 * @return	bank status
 *
 ******************************************************************************/

u32 XIs_ReadBankState(struct fwu_mdata *mdata, u32 bank_id)
{
	return mdata->bank_state[bank_id];
}

/*****************************************************************************/
/**
 * This function reads rollback counter value for each bank
 *
 * @param	rollback_cnt
 *
 * @param	bank_id
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_ReadRollBackCnt(u32 *rollback_cnt, u32 bank_id)
{
	u32 Status = XST_FAILURE;
#ifdef XIS_VERSAL_PLAT
	u32 opt_data_off;
	u32 opt_data_id_off;
#endif

	Status = XIs_ReadActiveBankData(bank_id, XIS_ZERO_OFFSET);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank data failed\r\n");
		return Status;
	}

#ifdef XIS_VERSAL_PLAT
	opt_data_off = *(u32 *)&ReadBuff[XIS_IHT_OFFSET] + XIH_IHT_LEN;
	Status = XIs_ReadActiveBankData(bank_id, opt_data_off);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank optional data failed\r\n");
		return Status;
	}
	Status = XIs_SearchOptionalData(XIS_OPT_DATA_ID_2, &opt_data_id_off);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Optional Data Id not found\r\n");
		return Status;
	}

	*rollback_cnt = *(u32 *)&ReadBuff[opt_data_id_off+XIS_VER_STRN_START_OFFSET];

#else
	*rollback_cnt = *(u32 *)&ReadBuff[XIS_ROLLBACK_CNT_OFFSET];
#endif

	return Status;
}

/*****************************************************************************/
/**
 * This function reads and prints the boot firmware information stored in the
 * BIF Optional Data section of the active bank boot image. It parses the
 * platform description (0x21), version (0x22), release (0x23) and build info
 * (0x24) fields and displays each one.
 *
 * @param	bank_id
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_PrintVerStrn(u32 bank_id)
{
#ifdef XIS_VERSAL_PLAT
	u32 Status = XST_FAILURE;
	u32 opt_data_off;
	u32 opt_data_id_off;
	u32 Index;
	u32 Data;
	u32 Len;
	u8 InfoStrn[XIS_IMG_REVISON_SIZE];

	Status = XIs_ReadActiveBankData(bank_id, XIS_ZERO_OFFSET);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank data failed\r\n");
		return Status;
	}

	opt_data_off = *(u32 *)&ReadBuff[XIS_IHT_OFFSET] + XIH_IHT_LEN;
	Status = XIs_ReadActiveBankData(bank_id, opt_data_off);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank optional data failed\r\n");
		return Status;
	}

	for (Index = 0U;
		Index < (sizeof(XIs_BootImgInfo) / sizeof(XIs_BootImgInfo[0]));
		Index++) {
		Status = XIs_SearchOptionalData(XIs_BootImgInfo[Index].DataId,
						&opt_data_id_off);
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "Optional Data Id 0x%x not found\r\n",
					XIs_BootImgInfo[Index].DataId);
			continue;
		}

		/* Derive the field length (in bytes) from the optional data header */
		Data = *(u32 *)&ReadBuff[opt_data_id_off];
		Len = ((Data & XIH_OPT_DATA_HDR_LEN_MASK) >>
			XIH_OPT_DATA_LEN_SHIFT) << XILPDI_WORD_LEN_SHIFT;
		if (Len > XIS_VER_STRN_START_OFFSET) {
			Len -= XIS_VER_STRN_START_OFFSET;
		} else {
			Len = 0U;
		}
		if (Len >= XIS_IMG_REVISON_SIZE) {
			Len = XIS_IMG_REVISON_SIZE - 1U;
		}

		memcpy(InfoStrn,
			&ReadBuff[opt_data_id_off + XIS_VER_STRN_START_OFFSET], Len);
		InfoStrn[Len] = '\0';
		XIs_Printf(XIS_DEBUG_PRINT_ALWAYS, "%s : %s\r\n",
			XIs_BootImgInfo[Index].Label, InfoStrn);
	}

	return XST_SUCCESS;
#endif
}

/*****************************************************************************/
/**
 * This function reads active bank data
 *
 * @param	bank_id
 *
 * @param	sub_offset
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_ReadActiveBankData(u32 bank_id, u32 sub_offset)
{
	u32 Status = XST_FAILURE;

	if(bank_id == XIS_ACTIVE_INDEX){
#ifdef XIS_QSPI_FLSH
		Status = XIs_QspiRead(XIS_BANK_0_OFFSET + sub_offset,
						(u8 *)ReadBuff, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
		Status = XIs_OspiRead(XIS_BANK_0_OFFSET + sub_offset,
						(u8 *)ReadBuff, XIS_SIZE_1KB);
#endif
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed\r\n");
			return Status;
		}
	}else if(bank_id == XIS_PREV_ACTIVE_INDEX){
#ifdef XIS_QSPI_FLSH
		Status = XIs_QspiRead(XIS_BANK_1_OFFSET + sub_offset,
						(u8 *)ReadBuff, XIS_SIZE_1KB);
#elif defined XIS_OSPI_FLSH
		Status = XIs_OspiRead(XIS_BANK_1_OFFSET + sub_offset,
						(u8 *)ReadBuff, XIS_SIZE_1KB);
#endif
		if (Status != XST_SUCCESS) {
			XIs_Printf(XIS_DEBUG_GENERAL, "QSPI Read failed\r\n");
			return Status;
		}
	} else {
		XIs_Printf(XIS_DEBUG_GENERAL, "Not a valid bank\r\n");
		Status = XST_FAILURE;
	}

	if(sub_offset == XIS_ZERO_OFFSET){
		/*Check if string XNLX exists in boot header*/
		if(strncmp((char *)&ReadBuff[XIS_XLNX_OFFSET], "XNLX", \
			XIS_XLNX_LENGTH) == 0){
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	}

	return Status;
}

#ifdef XIS_VERSAL_PLAT
/*****************************************************************************/
/**
 * This function search for optional data using Data Id
 *
 * @param	DataId
 *
 * @return	Data
 *
 ******************************************************************************/
u32 XIs_SearchOptionalData(u32 DataId, u32 *data_off)
{
	u32 Offset = 0;
	u32 Data;
	u32 Len;

	while(Offset < XIS_SIZE_1KB){
		Data = *(u32 *)&ReadBuff[Offset];
		if ((Data & XIH_OPT_DATA_HDR_ID_MASK) !=
				DataId) {
			Len = ((Data & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT) << XILPDI_WORD_LEN_SHIFT;
			if (Len == 0U) {
				break;
			}
			Offset += Len;
		} else {
				*data_off = Offset;
				return XST_SUCCESS;
		}
	}

	return XST_FAILURE;
}
#endif

/*****************************************************************************/
/**
 * This function update current bank to boot bank  persistent register
 *
 *
 * @param	bank_id
 *
 * @return	none
 *
 ******************************************************************************/

void XIs_UpdateBootPartReg(u32 bank_id)
{
	u32 PerRegVal;

	PerRegVal = XIs_In32(XIS_PMC_GLOB_PERS_REG);
	PerRegVal = ((PerRegVal & ~(0xFF << XIS_BOOT_PART_BYT)) | (bank_id << XIS_BOOT_PART_BYT));

	XIs_Out32(XIS_PMC_GLOB_PERS_REG, PerRegVal);
}

/*****************************************************************************/
/**
 * This function update rollback  persistent register
 *
 *
 * @param	counter_val
 *
 * @return	none
 *
 ******************************************************************************/

u32 XIs_UpdateRollbackReg(u32 counter_val)
{

	u32 PerRegVal;
	PerRegVal = XIs_In32(XIS_PMC_GLOB_PERS_REG);
	PerRegVal = ((PerRegVal & ~(0xFF << XIS_ROLLBACK_CNT_BYT)) | (counter_val << XIS_ROLLBACK_CNT_BYT));

	if(counter_val < XIS_MAX_ROLLBACK_CNT){
		XIs_Out32(XIS_PMC_GLOB_PERS_REG, PerRegVal);
		return XST_SUCCESS;
	} else {
		XIs_Printf(XIS_DEBUG_GENERAL, "Rollback counter limit reached\r\n");
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
 * This function check if current boot is in trial run state
 *
 * @param	mdata
 *
 * @param	bank_id
 *
 * @return	none
 *
 ******************************************************************************/
 
u32 XIs_IsTrialState(struct fwu_mdata *mdata)
{
	u32 Status = XST_FAILURE;
	u32 rollback_old, rollback_new;

	if((XIs_In32(XIS_PMC_GLOB_PERS_REG)>>XIS_MAGIC_VAL_BYT) == (XIS_PMC_REG_MAGIC_NUM>>XIS_MAGIC_VAL_BYT)) {
		if(((XIs_In32(XIS_PMC_GLOB_PERS_REG) >> XIS_BOOT_PART_BYT) & 0xFF) != mdata->active_index) {
			Status = XIs_ReadRollBackCnt(&rollback_old, mdata->previous_active_index);
			if (Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Rollback counter old read failed\r\n");
				return Status;
			}
			Status = XIs_ReadRollBackCnt(&rollback_new, mdata->active_index);
			if (Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Rollback counter new read failed\r\n");
				return Status;
			}
			if(rollback_new > rollback_old) {
				Status = XIs_UpdateRollbackReg(rollback_new);
				if(Status != XST_SUCCESS){
					return XIS_FALSE;
				}else{
					return XIS_TRUE;
				}
			} else {
				XIs_Printf(XIS_DEBUG_GENERAL, "Device already has the latest firmware"
				" image, hence trial state failed\r\n");
				return XIS_FALSE;
			}
		} else {
			/* Boot is already in trial states, hence return true */
			return XIS_TRUE;
		}
	} else {
		XIs_Printf(XIS_DEBUG_GENERAL, "Magic number doesn't exist,"
					" hence trial state failed\r\n");
		return XIS_FALSE;
	}
}

/*****************************************************************************/
/**
 * This function updates multiboot offset of the bank 
 *
 * @param	bank_id
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_JumpToBootBank(u32 bank_id)
{
	u32 boot_offset;

	if(bank_id == XIS_ACTIVE_INDEX){
		boot_offset = (XIS_BANK_0_OFFSET/ XIS_SIZE_32KB);
		XIs_Printf(XIS_DEBUG_GENERAL, "Multiboot boot_offset: 0x%x\r\n",boot_offset);
		XIs_UpdateMultiBootValue(boot_offset);
	} else {
		boot_offset = (XIS_BANK_1_OFFSET/ XIS_SIZE_32KB);
		XIs_Printf(XIS_DEBUG_GENERAL, "Multiboot boot_offset: 0x%x\r\n",boot_offset);
		XIs_UpdateMultiBootValue(boot_offset);
	}
	XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
}

/*****************************************************************************/
/**
 * This function updates multiboot offset of recovery image.
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_JumpToImgRcry(void)
{
	u32 boot_offset;

	XIs_ResetBootCnt();
	boot_offset = (XIS_RECOVERY_OFFSET/ XIS_SIZE_32KB);
	XIs_Printf(XIS_DEBUG_GENERAL, "Multiboot boot_offset: 0x%x\r\n",boot_offset);
	XIs_UpdateMultiBootValue(boot_offset);
	XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
}
