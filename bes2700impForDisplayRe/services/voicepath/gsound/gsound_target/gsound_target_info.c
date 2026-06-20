#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"

#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "gsound_dbg.h"
#include "customparam_section.h"
#include "gsound_custom_service.h"
#include "gsound_target.h"

extern void system_get_info(uint8_t *fw_rev_0,
                            uint8_t *fw_rev_1,
                            uint8_t *fw_rev_2,
                            uint8_t *fw_rev_3);

//
// Target Info functions.
//
GSoundStatus GSoundTargetInfoGetSerial(uint8_t *output_serial_number, uint32_t max_len)
{
    uint8_t  serial_num[CUSTOM_PARAM_SERIAL_NUM_LEN];
    uint32_t serial_num_len = 0;

    if (nv_custom_parameter_section_get_entry(CUSTOM_PARAM_SERIAL_NUM_INDEX,
                                              serial_num,
                                              &serial_num_len))
    {
        gsound_util_sn_conv(serial_num,
                            serial_num_len,
                            ( uint8_t *const )output_serial_number,
                            max_len);
    }
    else
    {
        strncpy(( char * )output_serial_number, "unknown", max_len);
    }

    GLOG_D("%s, SERIAL: %s", __FUNCTION__, output_serial_number);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetInfoGetAppVersion(char *version, uint32_t max_len)
{
    ASSERT(version, "%s null pointer received", __func__);
    GLOG_D("max_len of app version:%d", max_len);

    uint8_t fw_rev_0 = 0, fw_rev_1 = 0, fw_rev_2 = 1;

#ifdef FIRMWARE_REV
    system_get_info(&fw_rev_0, &fw_rev_1, &fw_rev_2, NULL);
#endif

    snprintf(( char * )version, max_len, "%d.%d.%d", fw_rev_0, fw_rev_1, fw_rev_2);
    GLOG_D("%s, APP VER: %s", __FUNCTION__, version);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetInfoGetDeviceId(uint32_t *device_id)
{
    *device_id = Get_ModelId();
    if(*device_id == 0){
        *device_id = 0x00003E;
    }
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetInfoGetPortVersion(uint8_t *version, uint32_t max_len)
{
    strncpy(( char * )version, GSOUND_BUILD_ID, max_len);
    GLOG_D("%s, PORT VER: %s", __FUNCTION__, version);

    return GSOUND_STATUS_OK;
}
