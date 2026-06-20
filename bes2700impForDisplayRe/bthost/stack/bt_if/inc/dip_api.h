#ifndef __DIP_API__H__
#define __DIP_API__H__

#include "bluetooth.h"
#include "sdp_api.h"
#include "dip_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

bt_dip_pnp_info_t* btif_dip_get_device_info(bt_bdaddr_t *remote);

typedef void (*DipApiCallBack)(bt_bdaddr_t *_addr, bool ios_flag);

void btif_dip_init(DipApiCallBack callback);
void btif_dip_clear(const bt_bdaddr_t *remote);
bt_status_t btif_dip_query_for_service(uint16_t conn_handle, btif_dip_client_t *client_t);
bool btif_dip_check_is_ios_device(const bt_bdaddr_t *remote);
void btif_dip_get_remote_info(btif_remote_device_t *btDevice);
bool btif_dip_get_process_status(bt_bdaddr_t *remote);
bool btif_dip_check_is_ios_by_vend_id(uint16_t vend_id, uint16_t vend_id_source);

#ifdef __cplusplus
}
#endif

#endif

