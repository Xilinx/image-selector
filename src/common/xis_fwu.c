/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
	if(BootCnt > MaxBootCnt) {
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
 * @param	mdata
 *
 * @param	bank_id
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 ******************************************************************************/
u32 XIs_ReadRollBackCnt(struct fwu_mdata *mdata, u32 *rollback_cnt, u32 bank_id)
{
	u32 Status = XST_FAILURE;
#ifdef XIS_VERSAL_PLAT
	u32 OptOff;
#endif

	Status = XIs_ReadActiveBankData(bank_id, XIS_ZERO_OFFSET);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank data failed\r\n");
		return Status;
	}

#ifdef XIS_VERSAL_PLAT
	OptOff = *(u32 *)&ReadBuff[XIS_IHT_OFFSET] + XIH_IHT_LEN;
	Status = XIs_ReadActiveBankData(bank_id, OptOff);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Active bank optional data failed\r\n");
		return Status;
	}
	Status = XIs_SearchOptionalData(XIS_OPT_DATA_ID, rollback_cnt);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Optional Data Id not found\r\n");
		return Status;
	}
#else
	*rollback_cnt = *(u32 *)&ReadBuff[XIS_ROLLBACK_CNT_OFFSET];
#endif

	return Status;
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
u32 XIs_SearchOptionalData(u32 DataId, u32 *rollback_cnt)
{
	u32 Offset = 0;
	u32 Data;

	while(Offset < XIS_SIZE_1KB){
		Data = *(u32 *)&ReadBuff[Offset];
		if ((Data & XIH_OPT_DATA_HDR_ID_MASK) !=
				DataId) {
			Offset += ((Data & XIH_OPT_DATA_HDR_LEN_MASK) >>
				XIH_OPT_DATA_LEN_SHIFT) << XILPDI_WORD_LEN_SHIFT;
		} else {
			*rollback_cnt = *(u32 *)&ReadBuff[Offset+4];
			return XST_SUCCESS;
			break;
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
			Status = XIs_ReadRollBackCnt(mdata, &rollback_old, mdata->previous_active_index);
			if (Status != XST_SUCCESS) {
				XIs_Printf(XIS_DEBUG_GENERAL, "Rollback counter old read failed\r\n");
				return Status;
			}
			XIs_ReadRollBackCnt(mdata, &rollback_new, mdata->active_index);
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
			XIs_Printf(XIS_DEBUG_GENERAL, "Boot Partition is equal to active bank"
					" image, hence trial state failed\r\n");
			return XIS_FALSE;
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
}
