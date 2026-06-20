#include "cmsis_os.h"
#include "string.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_flash_program.h"
#include "app_tota_data_transceiver.h"

#if (TOTA_DATA_TRANSCEIVER)

typedef struct{
    bool        isRuning;

    uint8_t     handleMode;
    uint32_t    handleAddr;
    uint32_t    handleLength;

    uint32_t    processedBytes;
    osTimerId   dataHandleTimer;

    uint16_t    flushBytes;
    uint16_t    packetMaxSize;
    /* rx buff: 4k  */
    uint16_t    buffPos;
}data_handle_t;



static data_handle_t dataHandle;

/*  */
static void tota_data_transceiver_cmd_handle(APP_TOTA_CMD_CODE_E opCode, uint8_t* ptrParam, uint32_t paramLen);

static bool check_recieve_cmd_sanity(DATA_TRANSCEIVER_CTL_T * pCtl);
static DATA_TRANSCEIVER_CTL_T deal_receive_cmd(DATA_TRANSCEIVER_CTL_T * pCtl);
static void reset_data_handle(void);
static void set_data_handle(DATA_TRANSCEIVER_CTL_T * pCtl);
static handle_status_e handle_with_data(uint8_t * pdata, uint16_t dataLen);



static handle_status_e  __handle_with_data_write_flash(uint8_t * pdata, uint16_t dataLen);
static handle_status_e  __handle_with_data_read_flash(uint8_t * pdata, uint16_t dataLen);
static void             __read_flash_one_time(uint32_t readAddr, uint32_t readLen);


static void data_handle_timeout_callback(void const *arg);
osTimerDef(handle_timeout, data_handle_timeout_callback);


static void tota_data_transceiver_cmd_handle(APP_TOTA_CMD_CODE_E opCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, opCode, paramLen);

    switch ( opCode )
    {
        case OP_TOTA_DATA_CTL:
        {
            DATA_TRANSCEIVER_CTL_T rsp_data_ctl = deal_receive_cmd((DATA_TRANSCEIVER_CTL_T *)ptrParam);
            app_tota_send_rsp(opCode, TOTA_NO_ERROR, (uint8_t*)&rsp_data_ctl, sizeof(DATA_TRANSCEIVER_CTL_T));
            if (rsp_data_ctl.mode==READ_FLASH&&rsp_data_ctl.status==ACCEPT)
            {
                __read_flash_one_time(dataHandle.handleAddr, dataHandle.handleLength);
                reset_data_handle();
            }
            break;
        }
        case OP_TOTA_DATA:
        {
            handle_status_e status = handle_with_data(ptrParam, paramLen);
            if ( status == HANDLE_OK )
            {
                TOTA_LOG_DBG(0, "[__handle__] >>>>>send-data>>>>>");
                app_tota_send_rsp(opCode, TOTA_NO_ERROR,NULL,0);
                //osTimerStart(dataHandle.dataHandleTimer, 5000);
            }
            else if (status == HANDLE_ERROR)
            {
                app_tota_send_rsp(opCode, TOTA_CMD_HANDLING_FAILED,NULL,0);
            }
            else if (status == HANDLE_FINISH)
            {
                DATA_TRANSCEIVER_CTL_T rsp_data_ctl;
                rsp_data_ctl.mode = dataHandle.handleMode;
                rsp_data_ctl.status = SUCCESS;
                app_tota_send_rsp(OP_TOTA_DATA_CTL, TOTA_NO_ERROR, (uint8_t*)&rsp_data_ctl, sizeof(DATA_TRANSCEIVER_CTL_T));
            }
            break;
        }
        default:
            ;
        // case data
    }
}

TOTA_COMMAND_TO_ADD(OP_TOTA_DATA_CTL, tota_data_transceiver_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_DATA, tota_data_transceiver_cmd_handle, false, 0, NULL );



static void data_handle_timeout_callback(void const *arg)
{
    DATA_TRANSCEIVER_CTL_T rsp_data_ctl;
    rsp_data_ctl.mode = dataHandle.handleMode;
    rsp_data_ctl.status = TIMEOUT;
    reset_data_handle();
    app_tota_send_rsp(OP_TOTA_DATA_CTL, TOTA_NO_ERROR, (uint8_t*)&rsp_data_ctl, sizeof(DATA_TRANSCEIVER_CTL_T));
}


static DATA_TRANSCEIVER_CTL_T deal_receive_cmd(DATA_TRANSCEIVER_CTL_T * pCmdCtl)
{
    DATA_TRANSCEIVER_CTL_T rsp = *pCmdCtl;
    rsp.status = REJECT;
    if ( !check_recieve_cmd_sanity(pCmdCtl) )
    {
        rsp.status = REJECT;
        return rsp;
    }

    if ( dataHandle.isRuning )
    {
        if ((dataHandle.handleMode==READ_FLASH)&&(pCmdCtl->mode == READ_FLASH && pCmdCtl->status == READ_DONE))
        {
            if (dataHandle.processedBytes == dataHandle.handleLength)
            {
                rsp.status = SUCCESS;
            }
            else
            {
                rsp.status = FAILED;
            }
            reset_data_handle();
            return rsp;
        }
        else
        {
            rsp.status = REJECT;
            return rsp;
        }
    }

    switch (pCmdCtl->status)
    {
        case REQUEST:
        {
            if ( dataHandle.isRuning )
            {
                rsp.status = REJECT;
            }
            else
            {
                rsp.status = ACCEPT;
                set_data_handle(pCmdCtl);
            }
            break;
        }
        default:
            ;
    }
    return rsp;
}

static void reset_data_handle(void)
{
    dataHandle.isRuning = false;
    osTimerDelete(dataHandle.dataHandleTimer);
}

static void set_data_handle(DATA_TRANSCEIVER_CTL_T * pCtl)
{

    dataHandle.handleMode       = pCtl->mode;
    dataHandle.handleLength     = pCtl->data_length;
    dataHandle.handleAddr       = pCtl->addr;
    dataHandle.packetMaxSize    = pCtl->max_packet_size;
    dataHandle.isRuning         = true;
    dataHandle.processedBytes   = 0;
    dataHandle.buffPos          = 0;
    dataHandle.flushBytes       = 4096 - (pCtl->addr%4096);
    dataHandle.dataHandleTimer  = osTimerCreate(osTimer(handle_timeout), osTimerOnce, NULL);
    TOTA_LOG_DBG(4, "[get] mode:%d, data length:%d, addr:0x%x, pdu:%d", dataHandle.handleMode, dataHandle.handleLength, dataHandle.handleAddr, dataHandle.packetMaxSize);
    //osTimerStart(dataHandle.dataHandleTimer, 5000);
}

static bool check_recieve_cmd_sanity(DATA_TRANSCEIVER_CTL_T * pCtl)
{
    if ( pCtl->mode != WRITE_FLASH && pCtl->mode != WRITE_RAM && pCtl->mode != READ_FLASH && pCtl->mode != READ_RAM )
    {
        TOTA_LOG_DBG(0, "[check cmd sanity]: data mode is unkonwn!");
        return false;
    }
    if ( pCtl->status != REQUEST )
    {
        TOTA_LOG_DBG(0, "[check cmd sanity]: data status is not REQUEST!");
        return false;
    }
    TOTA_LOG_DBG(0, "[check cmd sanity]: pass");
    return true;
}


static handle_status_e handle_with_data(uint8_t * pdata, uint16_t dataLen)
{
    if (!dataHandle.isRuning)
    {
        return HANDLE_ERROR;
    }

    switch (dataHandle.handleMode)
    {
        case WRITE_FLASH:
            TOTA_LOG_DBG(0, "[handle] WRITE FLASH");
            return __handle_with_data_write_flash(pdata, dataLen);
        case WRITE_RAM:
            TOTA_LOG_DBG(0, "[handle] WRITE_RAM not define...");
            return HANDLE_ERROR;
        case READ_FLASH:
            TOTA_LOG_DBG(0, "[handle] READ FLASH");
            return __handle_with_data_read_flash(pdata, dataLen);
        case READ_RAM:
            TOTA_LOG_DBG(0, "[handle] READ RAM not define...");
            break;
        default:
            TOTA_LOG_DBG(0, "unknown dataHandle Mode");

    }
    return HANDLE_ERROR;
}


static handle_status_e __handle_with_data_write_flash(uint8_t * pdata, uint16_t dataLen)
{
    TOTA_LOG_DBG(0, "[__handle__] <<<<<recv-data<<<<<");
    TOTA_LOG_DBG(2, "[__handle__] recv dataLen:%d, buffPos:%d", dataLen, dataHandle.buffPos);

    if (!tota_write_flash(dataHandle.handleAddr+dataHandle.buffPos, pdata, dataLen))
    {
        TOTA_LOG_DBG(0, "[__handle__] error! how to deal with write flash error");
        reset_data_handle();
        return HANDLE_ERROR;
    }
    dataHandle.buffPos += dataLen;
    TOTA_LOG_DBG(0, "[__handle__] handle success...");
    reset_data_handle();
    return HANDLE_FINISH;
}
static handle_status_e __handle_with_data_read_flash(uint8_t * pdata, uint16_t dataLen)
{
    uint32_t readAddr = dataHandle.handleAddr;
    uint16_t readLen  = dataHandle.packetMaxSize;
    TOTA_FLASH_DATA_T *flashData = NULL;

    TOTA_LOG_DBG(2, "[__handle__] %d / %d", dataHandle.processedBytes, dataHandle.handleLength);

    if ( dataHandle.processedBytes + readLen > dataHandle.handleLength )
    {
        readLen = dataHandle.handleLength - dataHandle.processedBytes;
        TOTA_LOG_DBG(0, "[__handle__] read success");
        TOTA_LOG_DBG(2, "[__handle__] addr: 0x%x, len: %d", dataHandle.handleAddr, readLen);
        flashData = tota_read_flash(readAddr, readLen);
        TOTA_LOG_DBG(0, "[__handle__] >>>>>send-data>>>>>");
        app_tota_send((uint8_t*)flashData->address, readLen, OP_TOTA_DATA);
        dataHandle.handleAddr += readLen;
        dataHandle.processedBytes += readLen;
        return HANDLE_SEND_DATA_DONE;
    }

    TOTA_LOG_DBG(0, "[__handle__] read a packet");
    TOTA_LOG_DBG(2, "[__handle__] addr: 0x%x, len: %d", dataHandle.handleAddr, readLen);
    flashData = tota_read_flash(readAddr, readLen);
    TOTA_LOG_DBG(0, "[__handle__] >>>>>send-data>>>>>");
    app_tota_send((uint8_t*)flashData->address, readLen, OP_TOTA_DATA);


    dataHandle.handleAddr += readLen;
    dataHandle.processedBytes += readLen;
    return HANDLE_SEND_DATA_DONE;
}

static void __read_flash_one_time(uint32_t readAddr, uint32_t readLen)
{
    uint16_t readMax = 640;
    TOTA_FLASH_DATA_T *flashData = NULL;

    while ( readLen > 640 )
    {
        TOTA_LOG_DBG(2, "[__handle__] addr: 0x%x, len: %d, rest: %d", readAddr, readMax, readLen);
        flashData = tota_read_flash(readAddr, readMax);
        app_tota_send((uint8_t*)flashData->address, readMax, OP_TOTA_DATA_ENCODE);
        TOTA_LOG_DBG(0, "[__handle__] >>>>>send-data>>>>>");
        readAddr += readMax;
        readLen -= readMax;
    }
    TOTA_LOG_DBG(2, "[__handle__] addr: 0x%x, len: %d", readAddr, readLen);
    flashData = tota_read_flash(readAddr, readLen);
    app_tota_send((uint8_t*)flashData->address, readLen, OP_TOTA_DATA_ENCODE);
    TOTA_LOG_DBG(0, "[__handle__] >>>>>send-data>>>>>");
}
#endif