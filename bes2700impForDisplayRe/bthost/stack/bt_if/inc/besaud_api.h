#ifndef __BESAUD_API_H__
#define __BESAUD_API_H__

#include "cqueue.h"
#include "bluetooth.h"
#include "me_api.h"

typedef uint16_t btif_besaud_event;

typedef void (*btif_besaud_status_change_callback)(btif_besaud_event event);

typedef void (*btif_besaud_data_received_callback)(uint8_t*, uint16_t);

#ifdef __cplusplus
extern "C" {
#endif

uint8_t btif_besaud_is_connected(void);
uint8_t btif_besaud_is_cmd_sending(void);
void btif_besaud_clear_cmd_sending(void);
void btif_besaud_client_create(uint16_t conn_handle);
void btif_besaud_server_create(btif_besaud_status_change_callback callback);
void btif_besaud_data_received_register(btif_besaud_data_received_callback callback);
void btif_besaud_send_cmd(uint8_t* cmd, uint16_t len);
bt_status_t btif_besaud_send_cmd_no_wait(uint8_t* cmd, uint16_t len);
btif_remote_device_t *btif_besaud_get_peer_device(void);

#ifdef __cplusplus
}
#endif

#endif

