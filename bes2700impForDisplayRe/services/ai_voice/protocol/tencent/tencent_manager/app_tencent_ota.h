#if 0
#ifndef __APP_TENCENT_OTA_H__
#define __APP_TENCENT_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OTA_SW_VERSION		2
#define OTA_HW_VERSION		1

#define FLASH_SECTOR_SIZE_IN_BYTES          4096
#define OTA_DATA_BUFFER_SIZE_FOR_BURNING    FLASH_SECTOR_SIZE_IN_BYTES

#define OTA_START_MAGIC_CODE    (0x54534542)  // BEST
#define NORMAL_BOOT				0xBE57EC1C
#define COPY_NEW_IMAGE			0x5a5a5a5a

#ifndef DATA_ACK_FOR_SPP_DATAPATH_ENABLED
#define DATA_ACK_FOR_SPP_DATAPATH_ENABLED 0
#endif

#ifndef NEW_IMAGE_FLASH_OFFSET
#define NEW_IMAGE_FLASH_OFFSET  (0x200000)
#endif

#ifndef __APP_IMAGE_FLASH_OFFSET__
#define __APP_IMAGE_FLASH_OFFSET__ (0x18000)
#endif

#define OTA_FLASH_LOGIC_ADDR        (FLASH_NC_BASE)
#define OTA_FLASH_OFFSET_OF_IMAGE   __APP_IMAGE_FLASH_OFFSET__
#define OTA_FLASH_ENTRY_OFFSET	    (OTA_FLASH_LOGIC_ADDR + OTA_FLASH_OFFSET_OF_IMAGE)

#define BD_ADDR_LENGTH					6
#define NAME_LENGTH     				32
#define OTA_BLE_DATA_PACKET_MAX_SIZE    257
#define OTA_BT_DATA_PACKET_MAX_SIZE     513


/**
 * @brief The format of the otaBootInfo
 *
 */
typedef struct
{
	uint32_t magicNumber;
	uint32_t imageSize;
	uint32_t imageCrc;
}__attribute__ ((__packed__)) FLASH_OTA_BOOT_INFO_T;

typedef struct
{
    uint8_t     version_md5[32];
    uint32_t    upgradeSize[(FLASH_SECTOR_SIZE_IN_BYTES - 32) / 4];
}__attribute__ ((__packed__)) FLASH_OTA_UPGRADE_LOG_FLASH_T;

typedef struct
{
    uint8_t    isToRenameBT;
    uint8_t    newBTName[32];
    uint8_t    isToRenameBLE;
    uint8_t    newBLEName[32];
    uint8_t    isToUpdateBTAddr;
    uint8_t    newBTAddr[6];
    uint8_t    isToUpdateBLEAddr;
    uint8_t    newBLEAddr[6];
    uint32_t    startLocationToWriteImage;    // the offset of the flash to start writing the image
}__attribute__ ((__packed__))OTA_CONFIGURATION_T;

/**
 * @brief The OTA handling data structure
 *
 */
typedef struct
{
    uint8_t     dataBufferForBurning[OTA_DATA_BUFFER_SIZE_FOR_BURNING];
    uint32_t    dstFlashOffsetForNewImage;
    uint32_t    offsetInDataBufferForBurning;
    uint32_t    offsetInFlashToProgram;
    uint32_t    totalImageSize;
    uint32_t    alreadyReceivedDataSizeOfImage;
    uint32_t    offsetInFlashOfCurrentSegment;
    uint32_t    offsetOfImageOfCurrentSegment;
    uint32_t    crc32OfImage;
    uint32_t    crc32OfSegment;
    uint32_t    flasehOffsetOfUserDataPool;
    uint32_t    flasehOffsetOfFactoryDataPool;

    OTA_CONFIGURATION_T configuration;

	uint8_t     isOTAInProgress;
	uint8_t 	isPendingForReboot;

	uint16_t     dataPacketSize;

    bool        resume_at_breakpoint;
    uint32_t    breakPoint;
    uint32_t    i_log;
} __attribute__ ((__packed__))OTA_ENV_T;


extern void app_tencent_ota_handler_init(void);
extern void app_tencent_ota_disconnection_handler(void);
void tencent_ota_update_MTU(uint16_t mtu);
int app_tencent_get_ota_status();

#ifdef __cplusplus
}
#endif
#endif  // __APP_TENCENT_OTA_H__
#endif
