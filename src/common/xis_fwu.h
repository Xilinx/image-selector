/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_fwu.h
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

#ifndef XIS_FWU_H
#define XIS_FWU_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xis_fwu_mdata.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XIS_PS_POR_B_RESET          0x1
#define XIS_BOOTCNT_SIZE            0x4

/* Magic values*/
#define XIS_BOOTCNT_MAGIC           0xB002C000
#define XIS_BOOTPART_MAGIC          0xB0020FF0
#define XIS_ROLLBACK_MAGIC          0x1D000000
#define XIS_WDT_MAGIC               0xFEE1DEAD

#define XIS_ACTIVE_INDEX            0x0
#define XIS_PREV_ACTIVE_INDEX       0x1

#define XIS_ROLLBACK_CNT_OFFSET     0x94
#define XIS_ROLLBACK_CNT_SIZE       0x4
#define XIS_MAX_ROLLBACK_CNT        0x1000000

#define XIS_TRUE                    0xF0
#define XIS_FALSE                   0xF1

#define XIS_MDATA_SIZE_OFFSET       0x10
#define XIS_MDATA_VER_OFFSET        0x4
#define XIS_MDATA_START_OFFSET      0x4
#define XIS_CRC32_SIZE              0x4
#define XIS_CRC32_OFFSET            0x0
#define XIS_DEFAULT_VAL             0x1
#define XIS_IHT_OFFSET              0xC4
#define XIS_OPT_DATA_ID             0x22
#define XIS_ZERO_OFFSET             0x00


/************************** Function Prototypes ******************************/

void XIs_InitBootCnt(void);
void XIs_ImageSelect(void);
void XIs_InitBootCnt(void);
void XIs_ResetBootCnt(void);
void XIs_IncBootCnt(void);
u32 XIs_SetBootCnt(u32 set_val);
void XIs_ReadBootCnt(void);
u32 XIs_ValidateBootCnt(void);
u32 XIs_ReadMetaData(struct fwu_mdata *mdata);
u32 XIs_ValidateMetaData(struct fwu_mdata *mdata, u8 *data_buff);
u32 XIs_ReadBankState(struct fwu_mdata *mdata, u32 bank_id);
u32 XIs_ReadRollBackCnt(struct fwu_mdata *mdata, u32 *rollback_cnt, u32 bank_id);
void XIs_UpdateBootPartReg(u32 bank_id);
u32 XIs_UpdateRollbackReg(u32 counter_val);
u32 XIs_IsTrialState(struct fwu_mdata *mdata);
void XIs_JumpToBootBank(u32 bank_id);
void XIs_JumpToImgRcry(void);
u32 XIs_ReadActiveBankData(u32 bank_id, u32 sub_offset);
u32 XIs_SearchOptionalData(u32 DataId, u32 *rollback_cnt);

#ifdef __cplusplus
}
#endif

#endif /* XIS_FWU_H */
