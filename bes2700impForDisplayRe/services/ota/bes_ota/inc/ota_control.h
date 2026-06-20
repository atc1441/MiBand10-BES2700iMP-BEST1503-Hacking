#ifndef _OTA_CONTROL_H_
#define _OTA_CONTROL_H_

#ifdef __IAG_BLE_INCLUDE__
#include "co_bt_defines.h"
#endif

#define BOOT_WORD_A                     0xAAAAAAAA
#define BOOT_WORD_B                     0xBBBBBBBB
#define BOOT_WORD_C                     0xCCCCCCCC
#define BOOT_WORD_D                     0xDDDDDDDD

#define F_PACK                      __attribute__ ((__packed__))
#define OTA_TLV_PACKET_HEADER_LEN   (OFFSETOF(OTA_PACKET_T, payload))
#define OTA_PACKET_LENGTH_FIELD_LEN sizeof(uint32_t) //!< length of length&rfu field in structure OTA_PACKET_T
#define OTA_PACKET_LENGTH_HEADER_LEN (OTA_PACKET_LENGTH_FIELD_LEN + sizeof(uint8_t))
#define OTA_TWS_INFO_SIZE 128
#define OTA_VERSION_INFO_LEN        4
#define BES_OTA_USER_INDEX_INVALID  0xFF

#define OTA_LEGACY_PACKET_HEADER_LEN (OFFSETOF(OTA_LEGACY_PACKET_T, payload))

#define ERASE_LEN_UNIT          4096

#define OTA_SW_VERSION          1
#define OTA_HW_VERSION          1

#ifndef BOOT_MAGIC_NUMBER
#define BOOT_MAGIC_NUMBER 0xBE57EC1C
#endif

#ifndef NORMAL_BOOT
#define NORMAL_BOOT BOOT_MAGIC_NUMBER
#endif

#ifndef COPY_NEW_IMAGE
#define COPY_NEW_IMAGE 0x5a5a5a5a
#endif

#ifndef NEW_IMAGE_FLASH_OFFSET
#define NEW_IMAGE_FLASH_OFFSET    0x180000
#endif

#ifdef USE_MULTI_FLASH
#define OTA_FLASH_LOGIC_ADDR        (FLASH1_NC_BASE)
#else
#define OTA_FLASH_LOGIC_ADDR        (FLASH_NC_BASE)
#endif

#define DATA_ACK_FOR_SPP_DATAPATH_ENABLED 0
#define PUYA_FLASH_ERASE_LIMIT 0

#define OTA_RS_INFO_MASTER_SEND_RS_REQ_CMD   1
#define OTA_RS_INFO_MASTER_DISCONECT_CMD     2
#define OTAUPLOG_HEADSIZE (sizeof(otaUpgradeLog.randomCode) + sizeof(otaUpgradeLog.totalImageSize) + sizeof(otaUpgradeLog.crc32OfImage))
#define FLASH_SECTOR_SIZE_IN_BYTES          4096

#ifndef OTA_OFFSET
#define OTA_OFFSET                          0x1000
#endif

#ifndef OTA_INFO_IN_OTA_BOOT_SEC
#define OTA_INFO_IN_OTA_BOOT_SEC            (FLASH_NC_BASE + OTA_OFFSET)
#endif

#define OTA_DATA_BUFFER_SIZE_FOR_BURNING    FLASH_SECTOR_SIZE_IN_BYTES

#define BES_OTA_START_MAGIC_CODE            0x54534542  // BEST

#define BD_ADDR_LEN          6
#define BES_OTA_NAME_LENGTH                 32
#define BES_OTA_BLE_DATA_PACKET_MAX_SIZE    512
#define BES_OTA_BT_DATA_PACKET_MAX_SIZE     BT_SPP_MAX_TX_MTU

#define IMAGE_RECV_FLASH_CHECK              0  // It's best to turn it on durning development and not a big deal off in the release.

#define MAX_IMAGE_SIZE                      ota_basic_get_max_image_size()

#define MIN_SEG_ALIGN                       256

typedef void (*ota_transmit_data_t)(uint8_t* ptrData, uint16_t dataLen);
typedef void (*ota_transmission_done_t)(void);

#ifdef COMBO_CUSBIN_IN_FLASH
typedef uint8_t OTA_COMBO_STATUS;
#define OTA_COMBO_PROCESS_SUCCESS          0
#define OTA_COMBO_PROCESS_FAILED           1
#define OTA_COMBO_PROCESS_SIZE_ERROR       2
#define OTA_COMBO_PROCESS_EXPCEPTION       3
#define OTA_COMBO_PROCESS_PARAM_INVAILD    4

#define OTA_COMBO_INVAILD_INDEX        0xFF
#define COMBO_USERID_ELEM_NUM_MAX      2
#define MAIN_INFO_TYPE_IMAGE_COMBO     0x00000200
#endif

#define OTA_ONE_BUD_MESSAGE_ERR     0

typedef enum
{
    OTA_RESULT_ERR              = 0,
    OTA_RESULT_PASSED           = 1,
    OTA_RESULT_ERR_RECV_SIZE    = 2,
    OTA_RESULT_ERR_FLASH_OFFSET = 3,
    OTA_RESULT_ERR_SEG_VERIFY   = 4,
    OTA_RESULT_ERR_BREAKPOINT   = 5,
    OTA_RESULT_ERR_IMAGE_SIZE   = 6,
} OTA_REQ_RESULT_E;

enum
{
    BES_OTA_USER_INVALID            = 0,
    BES_OTA_USER_FIRMWARE           = 1,
    BES_OTA_USER_LANGUAGE_PACKAGE   = 2,
    BES_OTA_USER_COMBOFIRMWARE      = 3,
    BES_OTA_USER_BOOTUP_INFO        = 4,
    BES_OTA_USER_UPGRADE_LOG        = 5,

    BES_OTA_USER_NUM,
};

enum
{
    OTA_PACKET_STRUCTURE_LEGACY = 0,
    OTA_PACKET_STRUCTURE_TLV    = 1,
};

/**
 * @brief The control packet type
 *
 */
typedef enum
{
    OTA_SET_PACKET_INTERVAL     = 0x7F, //!< from device to centrl, to set centrl packet sending interval(ms)
    OTA_COMMAND_START           = 0x80, //!< from central to device, to let OTA start
    OTA_RSP_START               = 0x81, //!< from device to centrl, to let it know that it has been ready for OTA
    OTA_COMMAND_SEGMENT_VERIFY  = 0x82, //!< from central to device, sent per 1% of image, inform device to do CRC check on those 1%
    OTA_RSP_SEGMENT_VERIFY      = 0x83, //!< Segment CRC verify response
    OTA_RSP_RESULT              = 0x84, //!< OTA result response
    OTA_DATA_PACKET             = 0x85, //!< Data packet
    OTA_COMMAND_CONFIG_OTA      = 0x86, //!< from central to device, to configure the following OTA flow
    OTA_RSP_CONFIG              = 0x87, //!< OTA configure response
    OTA_COMMAND_GET_OTA_RESULT  = 0x88, //!< Whole image CRC verify
    OTA_READ_FLASH_CONTENT      = 0x89, //!< Read flash content
    OTA_FLASH_CONTENT_DATA      = 0x8A, //!< Flash content response
    OTA_DATA_ACK                = 0x8B, //!< Data ack for SPP
    OTA_COMMAND_RESUME_VERIFY   = 0x8C, //!< Transmission resuming at break-point check
    OTA_RSP_RESUME_VERIFY       = 0x8D, //!< Transmission resuming at break-point verify response
    OTA_COMMAND_GET_VERSION     = 0x8E, //!< get current version
    OTA_RSP_VERSION             = 0x8F, //!< read version response
    OTA_COMMAND_SIDE_SELECTION  = 0x90, //!< Transmission side selection
    OTA_RSP_SIDE_SELECTION      = 0x91, //!< Side selection response
    OTA_COMMAND_IMAGE_APPLY     = 0x92, //!< Image overwriting confirmation
    OTA_RSP_IMAGE_APPLY         = 0x93, //!< Image overwriting response

    OTA_RSP_START_ROLE_SWITCH   = 0x95,
    OTA_RSP_ROLE_SWITCH_COMPLETE= 0x96,

    OTA_CMD_SET_USER            = 0x97, //!< set OTA user
    OTA_RSP_SET_USER            = 0x98, //!< response of set OTA user command

    OTA_CMD_GET_OTA_VERSION     = 0x99, //!< get OTA version
    OTA_RSP_GET_OTA_VERSION     = 0x9A, //!< response OTA version
    OTA_GET_LOCAL_RD_RSQ        = 0x9B,
    OTA_GET_LOCAL_RD_RSP        = 0x9C,
    OTA_SET_ONE_BUD_UPGRADE_CMD = 0x7B,
    OTA_SET_ONE_BUD_UPGRADE_RSP = 0x7C,

} OTA_CONTROL_PACKET_TYPE_E;

typedef struct
{
    uint32_t lengthOfFollowingData;
    uint32_t startLocationToWriteImage; //!< the offset of the flash to start writing the image
    uint32_t isToClearUserData  : 1;
    uint32_t isToRenameBT       : 1;
    uint32_t isToRenameBLE      : 1;
    uint32_t isToUpdateBTAddr   : 1;
    uint32_t isToUpdateBLEAddr  : 1;
    uint32_t reserve            : 27;
    uint8_t newBTName[BES_OTA_NAME_LENGTH];
    uint8_t newBLEName[BES_OTA_NAME_LENGTH];
    uint8_t newBTAddr[BD_ADDR_LEN];
    uint8_t newBLEAddr[BD_ADDR_LEN];
    uint32_t crcOfConfiguration; // CRC of data from lengthOfFollowingData to newBLEAddr
} F_PACK OTA_FLOW_CONFIGURATION_T;

/**
 * @brief The format of the OTA reading flash content command
 *
 */
typedef struct
{
    uint8_t     packetType;     // should be OTA_READ_FLASH_CONTENT
    uint8_t     isToStart;      // true to start, false to stop
    uint32_t    startAddr;
    uint32_t    lengthToRead;
} __attribute__ ((__packed__)) OTA_READ_FLASH_CONTENT_REQ_T;

/**
 * @brief The format of the OTA reading flash content command
 *
 */
typedef struct
{
    uint8_t     packetType;     // should be OTA_READ_FLASH_CONTENT
    uint8_t     isReadingReqHandledSuccessfully;
} __attribute__ ((__packed__)) OTA_READ_FLASH_CONTENT_RSP_T;

/*****************************************************************************/
//                      OTA protocol version: 0.0.0.1                        //
/*****************************************************************************/

/**
 * @brief Common data structure of OTA packet, used to parse the received data,
 *        this data structure is in legacy manner.
 *
 */
typedef struct
{
    uint8_t packetType;
    union {
        struct
        {
            uint32_t interval; //!< device set packet sending interval for app to use
        } F_PACK cmdOtaInterval;
        struct
        {
            uint8_t version[4]; //!< OTA version supported on APP
        } F_PACK cmdOtaVersion; //!< for OTA_CMD_GET_OTA_VERSION
        struct
        {
            uint8_t version[4]; //!< OTA version supported on firmware
        } F_PACK rspOtaVersion; //!< for OTA_RSP_GET_OTA_VERSION
        struct
        {
            uint8_t user;       //!< set OTA user
        } F_PACK cmdSetUser;    //!< for OTA_CMD_SET_USER
        struct
        {
            uint8_t result;     //!< result of set OTA user operation
        } F_PACK rspSetUser;    //!< OTA_RSP_SET_USER
        struct
        {
            uint32_t magicCode;     //!< should be BES_OTA_START_MAGIC_CODE
            uint32_t imageSize;     //!< total image size
            uint32_t crc32OfImage;  //!< crc32 of the whole image
        } F_PACK cmdOtaStart;       //!< for OTA_COMMAND_START
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
            uint16_t swVersion; //!< software version of current user
            uint16_t hwVersion; //!< hardware version of current user
            uint16_t MTU;       //!< MTU exchange result, central will send images in the unit of MTU
        } F_PACK rspOtaStart;   //!< for OTA_RSP_START
        struct
        {
            OTA_FLOW_CONFIGURATION_T config;
        } F_PACK cmdFlowControl;
        struct
        {
            uint8_t isConfigurationDone;
        } F_PACK rspFlowControl;
        struct
        {
            uint32_t magicCode;      //!< should be BES_OTA_START_MAGIC_CODE
            uint32_t crc32OfSegment; //!< crc32 of the 1% segment
        } F_PACK cmdSegmentVerify;
        struct
        {
            uint8_t isVerificationPassed;
        } F_PACK rspSegmentVerify;
        struct
        {
            uint8_t isVerificationPassed;
        } F_PACK rspOtaResult;  //!< for OTA_RSP_RESULT
        struct
        {
            uint8_t isToStart;  //!< true to start, false to stop
            uint32_t startAddr;
            uint32_t lengthToRead;
        } F_PACK cmdReadFlash;
        struct
        {
            uint8_t isReadingReqHandledSuccessfully;
        } F_PACK rspReadFlash;
        struct
        {
            uint32_t magicCode;         //!< should be BES_OTA_START_MAGIC_CODE
            uint8_t randomCode[32];
            uint32_t segmentSize;
            uint32_t crc32;             //!< CRC32 of randomCode and segment size
        } F_PACK cmdResumeBreakpoint;   //!< for OTA_COMMAND_RESUME_VERIFY
        struct
        {
            uint32_t breakPoint;
            uint8_t randomCode[32];
            uint32_t crc32;             //!< CRC32 of breakPoint and randomCode
        } F_PACK rspResumeBreakpoint;   //!< for OTA_RSP_RESUME_VERIFY
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdGetVision;  //!< for OTA_COMMAND_GET_VERSION
        struct
        {
            uint32_t magicCode;         //!< should be BES_OTA_START_MAGIC_CODE
            uint8_t deviceType;
            uint8_t leftVersion[4];     //!< or stereo version
            uint8_t rightVersion[4];    //!< stereo not needed
            uint8_t gitVersion[255];
        } F_PACK rspGetVersion;         //!< for OTA_RSP_VERSION
        struct
        {
            uint8_t success;
        } F_PACK rspSideSelect;
        struct
        {
            uint8_t selectOneBud;
        } F_PACK cmdOneBudSelect;
        struct
        {
            uint8_t success;
        } F_PACK rspOneBudSelect;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdForceReboot;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdApply;
        struct
        {
            uint8_t success;
        } F_PACK rspApply;
        struct
        {
            uint8_t localRandom[2];
        } F_PACK rspLocalRandom;
    } payload;
} F_PACK OTA_LEGACY_PACKET_T;

/**
 * @brief Common data structure of OTA packet, used to parse the received data
 *
 */
typedef struct
{
    uint8_t packetType;
    uint32_t length : 10;
    uint32_t rfu : 22;
    union {
        struct
        {
            uint32_t interval; //!< device set packet sending interval for app to use
        } F_PACK cmdOtaInterval;
        struct
        {
            uint8_t version[4]; //!< OTA version supported on APP
        } F_PACK cmdOtaVersion;
        struct
        {
            uint8_t version[4]; //!< OTA version supported on firmware
        } F_PACK rspOtaVersion;
        struct
        {
            uint8_t user; //!< set OTA user
        } F_PACK cmdSetUser;
        struct
        {
            uint8_t result; //!< result of set OTA user operation
        } F_PACK rspSetUser;
        struct
        {
            uint32_t magicCode;    //!< should be BES_OTA_START_MAGIC_CODE
            uint32_t imageSize;    //!< total image size
            uint32_t crc32OfImage; //!< crc32 of the whole image
        } F_PACK cmdOtaStart;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
            uint16_t swVersion; //!< software version of current user
            uint16_t hwVersion; //!< hardware version of current user
            uint16_t MTU;       //!< MTU exchange result, central will send images in the unit of MTU
        } F_PACK rspOtaStart;
        struct
        {
            OTA_FLOW_CONFIGURATION_T config;
        } F_PACK cmdFlowControl;
        struct
        {
            uint8_t isConfigurationDone;
        } F_PACK rspFlowControl;
        struct
        {
            uint32_t magicCode;      //!< should be BES_OTA_START_MAGIC_CODE
            uint32_t crc32OfSegment; //!< crc32 of the 1% segment
        } F_PACK cmdSegmentVerify;
        struct
        {
            uint8_t isVerificationPassed;
            uint32_t crc32OfSegment; //!< crc32 of the 1% segment
        } F_PACK rspSegmentVerify;
        struct
        {
            uint8_t isVerificationPassed;
        } F_PACK rspOtaResult;
        struct
        {
            uint8_t isToStart; //!< true to start, false to stop
            uint32_t startAddr;
            uint32_t lengthToRead;
        } F_PACK cmdReadFlash;
        struct
        {
            uint8_t isReadingReqHandledSuccessfully;
        } F_PACK rspReadFlash;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
            uint8_t randomCode[32];
            uint32_t segmentSize;
            uint32_t crc32; //!< CRC32 of randomCode and segment size
        } F_PACK cmdResumeBreakpoint;
        struct
        {
            uint32_t breakPoint;
            uint8_t randomCode[32];
            uint32_t crc32; //!< CRC32 of breakPoint and randomCode
        } F_PACK rspResumeBreakpoint;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdGetVision;
        struct
        {
            uint32_t magicCode;
            uint8_t deviceType;
            uint8_t leftVersion[4];  //!< or stereo version
            uint8_t rightVersion[4]; //!< stereo not needed
            uint8_t gitVersion[255];
        } F_PACK rspGetVersion;
        struct
        {
            uint8_t selectOneBud;
        } F_PACK cmdOneBudSelect;
        struct
        {
            uint8_t success;
        } F_PACK rspOneBudSelect;
        struct
        {
            uint8_t success;
        } F_PACK rspSideSelect;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdForceReboot;
        struct
        {
            uint32_t magicCode; //!< should be BES_OTA_START_MAGIC_CODE
        } F_PACK cmdApply;
        struct
        {
            uint8_t success;
        } F_PACK rspApply;
        struct
        {
            uint8_t localRandom[2];
        } F_PACK rspLocalRandom;
    } payload;
} F_PACK OTA_PACKET_T;

/*****************************************************************************/
//                      OTA combo bin structure                        //
/*****************************************************************************/
#ifdef COMBO_CUSBIN_IN_FLASH
enum combin_sub_id
{
    COMBO_SOC_FW         = 0x00000101,
    COMBO_MCU_FW         = 0x00000102,
    COMBO_SENSOR_FW_1    = 0x00000103,
    COMBO_TZ_SB          = 0x00000104,
    COMBO_TZ_NSB         = 0x00000105,
    COMBO_PROMPT         = 0x00000106,
};

enum combin_process_evt
{
    COMBIN_PARASE_HEADER_EVT  = 0,
    COMBIN_SET_START_ADDRESS  = 1,
    COMBIN_GET_START_ADDRESS  = 2,
};

enum
{
    SEGMENT_CRC_TYPE = 0,
    WHOLE_CRC_TYPE = 1,
};

typedef struct
{
    uint8_t  ModuleId;
    uint32_t FwId;
    uint32_t StartAddr;
    uint32_t EndAddr;
}FLASH_MODULE_INFO_T;
typedef uint8_t (*PROCESS_CB_T)(uint8_t event,uint8_t* parm0, uint32_t* parm1);
#endif

typedef void (*APPLY_CB_T)(uint32_t startAddr, uint32_t imageLen);
typedef void (*START_CB_T)(void);
typedef void (*RECEPTION_DONE_CB_T)(void);

typedef struct
{
    uint8_t user;
    uint8_t norflashId;
    uint32_t startAddr;
    uint32_t length;
    START_CB_T start;
    RECEPTION_DONE_CB_T reception_done;
    APPLY_CB_T apply;
#ifdef COMBO_CUSBIN_IN_FLASH
    PROCESS_CB_T process;
#endif
} BES_OTA_USER_INFO_T;
/*****************************************************************************/
//                      OTA protocol version: 0.0.0.1                        //
/*****************************************************************************/


/**
 * @brief The OTA handling data structure
 *
 */
typedef struct
{
    uint8_t *dataBufferForBurning;
    uint32_t dstFlashOffsetForNewImage;
    uint32_t offsetInDataBufferForBurning;
    uint32_t offsetInFlashToProgram;
    uint32_t totalImageSize;
    uint32_t alreadyReceivedDataSizeOfImage;
    uint32_t offsetInFlashOfCurrentSegment;
    uint32_t offsetOfImageOfCurrentSegment;
    uint32_t crc32OfImage;
    uint32_t crc32OfSegment;
    uint8_t isOTAInProgress;
    uint32_t receivedCrc32OfSegment;
    uint8_t isPendingForReboot;
    uint32_t flasehOffsetOfUserDataPool;
    uint32_t flasehOffsetOfFactoryDataPool;
    OTA_FLOW_CONFIGURATION_T configuration; //!< configuration of the OTA
    ota_transmit_data_t transmitHander;
    uint32_t offsetInFlashToRead;
    uint32_t leftSizeOfFlashContentToRead;
    uint8_t dataPathType;
    bool resume_at_breakpoint;
    bool isOneBudOTA;
    uint32_t breakPoint;
    uint32_t i_log;
#ifdef COMBO_CUSBIN_IN_FLASH
    uint32_t lastdstFlashOffsetForNewImage;
    uint32_t currentlengthlimit;
    uint32_t combinedSize;
    uint32_t crc32OfHeader;
    uint8_t  blockid;
    bool  is_crossregion;   // true: if the received data contains multiple regions
    bool  is_started;
#endif
/*****************************************************************************/
//                      OTA protocol version: 0.0.0.1                        //
/*****************************************************************************/
    uint8_t packetStructureType;
    uint8_t currentUser;
    uint8_t userNum;
    BES_OTA_USER_INFO_T userInfo[BES_OTA_USER_NUM];
/*****************************************************************************/
//                      OTA protocol version: 0.0.0.1                        //
/*****************************************************************************/
} OTA_CONTROL_ENV_T;

typedef struct
{
    uint32_t magicNumber;    // NORMAL_BOOT or COPY_NEW_IMAGE
    uint32_t imageSize;
    uint32_t imageCrc;
    uint32_t newImageFlashOffset;
    uint32_t boot_word;
    uint32_t tzNsImageSize;
    uint32_t tzNsImageCrc;
    uint32_t tzNsNewImageFlashOffset;
} FLASH_OTA_BOOT_INFO_T;

typedef struct
{
    uint8_t     randomCode[32];
    uint32_t    totalImageSize;
    uint32_t    crc32OfImage;
    uint32_t    upgradeSize[(ERASE_LEN_UNIT - 32 - 2*sizeof(uint32_t)) / 4];
}FLASH_OTA_UPGRADE_LOG_FLASH_T;

typedef struct
{
    uint8_t     typeCode;
    uint16_t    rsp_seq;
    uint16_t    length;
    uint8_t     p_buff[OTA_TWS_INFO_SIZE];
}OTA_IBRT_TWS_CMD_EXECUTED_RESULT_FROM_SLAVE_T;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format of the data xfer handler function, to send data to central
 *
 * @param ptrData    Pointer of the data to send
 * @param dataLen     Length of the data to send
 *
 */

void ota_control_register_transmitter(ota_transmit_data_t transmit_handle);
void ota_update_info(void);
uint32_t ota_common_get_boot_info_flash_offset(void);
void ota_check_and_reboot_to_use_new_image(void);
void ota_control_reset_env(bool resetUser);
void ota_execute_the_final_operation(void);
bool ota_is_in_progress(void);
void ota_control_set_datapath_type(uint8_t datapathType);
void ibrt_ota_send_start_response(bool isViaBle);
void ibrt_ota_send_configuration_response(bool isDone);
void ota_control_image_apply_rsp(uint8_t success);
void ota_control_send_result_response(uint8_t isSuccessful);
void ibrt_ota_send_segment_verification_response(bool isPass);
void ibrt_ota_send_result_response(uint8_t isSuccessful);
void ota_randomCode_log(uint8_t randomCode[]);
void ota_control_send_resume_response(uint32_t breakPoint, uint8_t randomCode[]);
void ota_control_handle_received_data(uint8_t *otaBuf, bool isViaBle, uint16_t len);
void ota_control_side_selection_rsp(uint8_t success);
uint8_t ota_control_get_datapath_type(void);
void Bes_exit_ota_state(void);
void ibrt_ota_send_version_rsp(void);
void ota_upgradeLog_destroy(void);
void ota_status_change(bool status);
void ota_control_send_start_role_switch(void);
void ota_control_send_role_switch_complete(void);
uint8_t app_get_bes_ota_state(void);
bool app_check_is_ota_role_switch_initiator(void);
void app_set_ota_role_switch_initiator(bool is_initiate);
void bes_ota_send_role_switch_req(void);
void app_statistics_get_ota_pkt_in_role_switch(void);
bool app_check_user_can_role_switch_in_ota(void);
void erase_segment(uint32_t addr, uint32_t length);
uint32_t ota_get_current_start_addr(void);
uint32_t ota_get_current_length_to_check(void);
void ota_restore_offset(void);
void ota_control_send_get_random_response(uint32_t ota_rv);
void ota_update_flash_offset_after_segment_crc(bool flag);

#ifdef IBRT
void ota_tws_send_handle(uint8_t typeCode, uint8_t *buff, uint16_t length, uint32_t cmd_dode);
#endif

#ifdef PROMPT_IN_FLASH
void ota_data_reception_languange_check(void);
void ota_check_and_reboot_to_copy_language(void);
void ota_language_copy_prompt(uint32_t prompt_start, uint32_t prompt_end, uint32_t offset, uint32_t packetSize);
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
uint8_t _combo_bin_start_load(uint8_t* buf,uint32_t* len);
uint8_t _combo_bin_get_module_id(uint32_t offset);
uint8_t _combo_bin_check_is_exchange_area(uint8_t* buf,uint32_t len, uint32_t* offset);
void _combo_bin_calc_crc32(void);
void _combo_bin_erase_segment(uint32_t addr, uint32_t length);
void _combo_bin_package_handler(uint8_t *buf, bool isViaBle,uint16_t length);
void  ota_combo_bin_data_check_image_crc(void);
void _combo_bin_breakpoint_recovery(uint32_t breakpoint);
uint32_t ota_bin_breakpoint_update(uint32_t breakpoint);
#endif



/**
 * @brief Send response to OTA set user command
 *
 * @param isSuccessful  The result of set_user command execution, true for success; false for fail
 */
void ibrt_ota_send_set_user_response(uint8_t isSuccessful);

/**
 * @brief Send response to get OTA version command
 *
 * @param buf           pointer of peer OTA version info
 * @param len           length of peer OTA version info
 */
void ibrt_ota_send_ota_version_rsp(uint8_t *buf, uint16_t len);

/**
 * @brief Register OTA user
 *
 * @param info          OTA user info to be registered
 */
void ota_control_register_ota_user(BES_OTA_USER_INFO_T info);

/**
 * @brief Start callback for all OTA user
 *
 */
void ota_start(void);

/**
 * @brief Callback when OTA data reception done
 *
 */
void ota_data_reception_done(void);

/**
 * @brief Apply callback for all OTA users
 *
 */
void ota_apply(void);

/**
 * @brief Update the flash offset of image to program
 *
 * @param addr          the flash address to update
 */
void ota_control_update_flash_offset(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif
