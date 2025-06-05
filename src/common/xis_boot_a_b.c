/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_boot_a_b.c
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
* 1.00  sd   05/09/24 First release
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xis_config.h"
#include "xis_plat.h"
#include "xis_boot_a_b.h"
#include "xis_fwu_mdata.h"
#include "xis_fwu.h"
#include "xis_crc32.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XIs_PrintGuid(efi_guid_t guid);
/************************** Variable Definitions *****************************/
struct fwu_mdata Mdata;
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function check for bank A/B selection and update
 * multiboot value
 *
 * @return	XST_SUCCESS
 *
 ******************************************************************************/
u32 XIs_BootABImageBank(void)
{
	u32 Status = XST_FAILURE;
	u32 rollback_count;

	XIs_InitBootCnt();

	Status = XIs_ReadMetaData(&Mdata);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read meta data failed"
			" Launch recovery tool\r\n");
		goto RCRY;
	}

	XIs_ReadBootCnt();
	Status = XIs_ReadRollBackCnt(&rollback_count, Mdata.active_index);
	if (Status != XST_SUCCESS) {
		XIs_Printf(XIS_DEBUG_GENERAL, "Read Rollback counter fail"
			" Launch recovery tool\r\n");
		goto RCRY;
	}
	XIs_Printf(XIS_DEBUG_GENERAL, "Rollback counter: %x\n", rollback_count);
	XIs_PrintMdata();
	(void)XIs_PrintVerStrn(Mdata.active_index);

	if(XIs_ReadBankState(&Mdata, Mdata.active_index)
						 == XIS_FWU_ACCEPTED_BANK_STATE){
		XIs_Printf(XIS_DEBUG_GENERAL, "Reset the Boot counter as bank"
					" %d is in accepted state\r\n", Mdata.active_index);
		XIs_ResetBootCnt();
		XIs_UpdateBootPart(Mdata.active_index);
		goto END;
	}else{
		XIs_Printf(XIS_DEBUG_GENERAL, "Check for trial state"
					" and validate boot counter\r\n");
		if(XIs_ValidateBootCnt() != XIS_TRUE){
			XIs_Printf(XIS_DEBUG_GENERAL, "Boot counter limit exceeded,"
					" Launch recovery tool\r\n");
			goto RCRY;
		}
		if(XIs_IsTrialState(&Mdata) == XIS_TRUE){
			XIs_Printf(XIS_DEBUG_GENERAL, "Boot is in trial state\r\n");
			XIs_Printf(XIS_DEBUG_GENERAL, "Check if bank %d is in accepted"
						" state\r\n", Mdata.previous_active_index);
			if(XIs_ReadBankState(&Mdata, Mdata.previous_active_index)
										== XIS_FWU_ACCEPTED_BANK_STATE){
				XIs_UpdateBootPart(Mdata.active_index);
				goto END;
			}else{
				XIs_Printf(XIS_DEBUG_GENERAL, "Launch recovery tool,"
							" as the bank %d is not in accepted"
							" state\r\n", Mdata.previous_active_index);
				goto RCRY;
			}
		}else{
			XIs_Printf(XIS_DEBUG_GENERAL, "Boot is not in trial state\r\n");
			if(XIs_ReadBankState(&Mdata, Mdata.previous_active_index)
										== XIS_FWU_ACCEPTED_BANK_STATE){
				XIs_Printf(XIS_DEBUG_GENERAL, "Reset the Boot counter as bank"
							" is not in trial state\r\n");
				XIs_ResetBootCnt();
				XIs_UpdateBootPart(Mdata.previous_active_index);
				goto END;
			}else{
				XIs_Printf(XIS_DEBUG_GENERAL, "Launch recovery tool,"
							"as the bank %d is not in accepted"
							" state\r\n", Mdata.previous_active_index);
				goto RCRY;
			}
		}
	}

RCRY:
	XIs_JumpToImgRcry();

END:
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function update bank partition to boot bank register and
 * updates multiboot for bank partition
 *
 * @param	bank_id
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_UpdateBootPart(u32 bank_id)
{
	XIs_Printf(XIS_DEBUG_GENERAL, "Updated bank %d to boot part"
					" register\r\n", bank_id);
	XIs_UpdateBootPartReg(bank_id);

	XIs_Printf(XIS_DEBUG_GENERAL, "Updated multiboot value to bank"
				" %d\r\n", bank_id);
	XIs_JumpToBootBank(bank_id);
}

/*****************************************************************************/
/**
 * This function prints metadata structure information
 *
 * @return	none
 *
 ******************************************************************************/
void XIs_PrintMdata(void)
{
	u32 i, j;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_info;

	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.crc32: %x\r\n", Mdata.crc32);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.version: %x\r\n", Mdata.version);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.active_index: %x\r\n",
								Mdata.active_index);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.previous_active_index: %x\r\n",
								Mdata.previous_active_index);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.metadata_size: %x\r\n",
								Mdata.metadata_size);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.desc_offset: %x\r\n",
								Mdata.desc_offset);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.bank_state[0]: %x\r\n",
								Mdata.bank_state[0]);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.bank_state[1]: %x\r\n",
								Mdata.bank_state[1]);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.bank_state[2]: %x\r\n",
								Mdata.bank_state[2]);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.bank_state[3]: %x\r\n",
								Mdata.bank_state[3]);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.fw_desc.num_banks: %x\r\n",
								Mdata.fw_desc.num_banks);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.fw_desc.num_images: %x\r\n",
								Mdata.fw_desc.num_images);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.fw_desc.img_entry_size: %x\r\n",
								Mdata.fw_desc.img_entry_size);
	XIs_Printf(XIS_DEBUG_GENERAL, "Mdata.fw_desc.bank_info_entry_size: %x\r\n",
								Mdata.fw_desc.bank_info_entry_size);

	for (i = 0; i < Mdata.fw_desc.num_images; i++) {
		img_entry = &Mdata.fw_desc.img_entry[i];
		XIs_Printf(XIS_DEBUG_GENERAL, "Image Type Guid: ");
		XIs_PrintGuid(img_entry->image_type_guid);
		XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
		XIs_Printf(XIS_DEBUG_GENERAL, "Location Guid: ");
		XIs_PrintGuid(img_entry->location_guid);
		XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
		for (j = 0; j < Mdata.fw_desc.num_banks; j++) {
			img_info = &img_entry->img_bank_info[j];
			XIs_Printf(XIS_DEBUG_GENERAL, "Image Guid: ");
			XIs_PrintGuid(img_info->image_guid);
			XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
			XIs_Printf(XIS_DEBUG_GENERAL, "Image Acceptance: %s",
					img_info->accepted == 0x1 ? "yes" : "no");
			XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
		}
		XIs_Printf(XIS_DEBUG_GENERAL, "\r\n");
	}
}

/*****************************************************************************/
/**
 * This function prints guid formats
 *
 * @param	efi_guid_t guid data type
 *
 * @return	None
 *
 ******************************************************************************/
static void XIs_PrintGuid(efi_guid_t guid)
{
	u32 *guid_a;
	u16 *guid_b, *guid_c, k;
	u8 guid_d[8];

	guid_a = (u32*)&guid.b[0];
	guid_b = (u16*)&guid.b[4];
	guid_c = (u16*)&guid.b[6];
	memcpy(guid_d, &guid.b[8], 8);
	XIs_Printf(XIS_DEBUG_GENERAL, "%08x-%04x-%04x-", *guid_a, *guid_b, *guid_c);
	for(k=0;k<8;k++){
		XIs_Printf(XIS_DEBUG_GENERAL, "%02x", guid_d[k]);
		if(k == 1) XIs_Printf(XIS_DEBUG_GENERAL, "-");
	}
}
