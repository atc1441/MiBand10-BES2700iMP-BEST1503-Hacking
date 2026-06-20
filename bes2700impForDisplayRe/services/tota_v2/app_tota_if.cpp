
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "app_tota_if.h"




int8_t app_tota_get_rssi_value()
{
    return bes_bt_me_get_bt_rssi();
}


// To adapt different chips