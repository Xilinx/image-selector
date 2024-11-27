/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_fwu_mdata.h
*
* #TODO
*
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
#ifndef XIS_FWU_MDATA_H
#define XIS_FWU_MDATA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XIS_FWU_MDATA_VER                       0x2
#define XIS_FWU_MAX_NUM_BANKS                   0x4
#define XIS_FWU_NUM_BANKS                       0x2
#define XIS_FWU_NUM_IMAGES_PER_BANK             0x1
#define XIS_FWU_INVALID_BANK_STATE              0xFF
#define XIS_FWU_VALID_BANK_STATE                0xFE
#define XIS_FWU_ACCEPTED_BANK_STATE             0xFC

/**
 * The EFI spec defines the EFI_GUID as
 * "128-bit buffer containing a unique identifier value. Unless otherwise specified,
 * aligned on a 64-bit boundary".
 * Page 163 of the UEFI specification v2.10 and
 * EDK2 reference implementation both define EFI_GUID as
 * struct { u32 a; u16; b; u16 c; u8 d[8]; }; which is 4-byte
 * aligned.
 */
typedef struct {
    u8 b[16];
} efi_guid_t __attribute__((aligned(4)));

/**
 * struct fwu_image_bank_info - firmware image information
 * @image_uuid: Guid value of the image in this bank
 * @accepted: Acceptance status of the image
 * @reserved: Reserved
 *
 * The structure contains image specific fields which are
 * used to identify the image and to specify the image's
 * acceptance status
 */
struct fwu_image_bank_info {
    efi_guid_t image_guid;
    uint32_t accepted;
    uint32_t reserved;
};

/**
 * struct fwu_image_entry - information for a particular type of image
 * @image_type_uuid: Guid value for identifying the image type
 * @location_uuid: Guid of the storage volume where the image is located
 * @img_bank_info: Array containing properties of images
 *
 * This structure contains information on various types of updatable
 * firmware images. Each image type then contains an array of image
 * information per bank.
 */
struct fwu_image_entry {
    efi_guid_t image_type_guid;
    efi_guid_t location_guid;
    struct fwu_image_bank_info img_bank_info[XIS_FWU_NUM_IMAGES_PER_BANK];
};

/**
 * struct fwu_fw_desc_store - FWU updatable image information
 * @num_banks: Number of firmware banks
 * @num_images: Number of images per bank
 * @img_entry_size: The size of the img_entry array
 * @bank_info_entry_size: The size of the img_bank_info array
 * @img_entry: Array of image entries each giving information on a image
 *
 * This image descriptor structure contains information on the number of
 * updatable banks and images per bank. It also gives the total sizes of
 * the fwu_image_entry and fwu_image_bank_info arrays.
 */
struct fwu_fw_store_desc {
    uint8_t num_banks;
    uint8_t reserved;
    uint16_t num_images;
    uint16_t img_entry_size;
    uint16_t bank_info_entry_size;
    struct fwu_image_entry img_entry[XIS_FWU_NUM_BANKS];
};

/**
 * struct fwu_mdata - FWU metadata structure for multi-bank updates
 * @crc32: crc32 value for the FWU metadata
 * @version: FWU metadata version
 * @active_index: Index of the bank currently used for booting images
 * @previous_active_inde: Index of the bank used before the current bank
 *                        being used for booting
 * @img_entry: Array of information on various firmware images that can
 *             be updated
 *
 * This structure is used to store all the needed information for performing
 * multi bank updates on the platform. This contains info on the bank being
 * used to boot along with the information needed for identification of
 * individual images
 */
struct fwu_mdata {
    uint32_t crc32;
    uint32_t version;
    uint32_t active_index;
    uint32_t previous_active_index;
    uint32_t metadata_size;
    uint16_t desc_offset;
    uint16_t reserved1;
    uint8_t bank_state[XIS_FWU_MAX_NUM_BANKS];
    uint32_t reserved2;
    struct fwu_fw_store_desc fw_desc;
};

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* XIS_FWU_MDATA_H */
