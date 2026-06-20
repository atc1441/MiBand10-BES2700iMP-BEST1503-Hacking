#include "gsound_target.h"
#include "gsound_dbg.h"

void GSoundTargetBleRxComplete(GSoundChannelType type,
                               const uint8_t *buffer,
                               uint32_t len)
{
}

GSoundStatus GSoundTargetBleTransmit(GSoundChannelType channel,
                                     const uint8_t *data,
                                     uint32_t length,
                                     uint32_t *bytes_consumed)
{
    return GSOUND_STATUS_OK;
}

uint16_t GSoundTargetBleGetMtu()
{
    return 0;
}

GSoundStatus GSoundTargetBleUpdateConnParams(bool valid_interval,
                                             uint32_t min_interval,
                                             uint32_t max_interval,
                                             bool valid_slave_latency,
                                             uint32_t slave_latency)
{
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetBleInit(const GSoundBleInterface *handlers)
{
    return GSOUND_STATUS_OK;
}
