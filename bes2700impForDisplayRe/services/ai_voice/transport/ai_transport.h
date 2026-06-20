#ifndef _AI_TRANSPORT_H_
#define _AI_TRANSPORT_H_

#include "stdint.h"
#include "cqueue.h"
#include "app_ai_if_config.h"
#include "ai_spp.h"

#ifdef __GMA_VOICE__
#define AI_TRANS_DEINIT_MTU_SIZE   (255)
#else
#define AI_TRANS_DEINIT_MTU_SIZE (255)
#endif

#ifndef AI_CMD_TRANSPORT_BUFF_FIFO_SIZE
#define AI_CMD_TRANSPORT_BUFF_FIFO_SIZE (1024)
#endif

#define MAX_TX_BUFF_SIZE    (BT_SPP_MAX_TX_MTU) // for spp/ble, it is a medium value
#ifndef AI_REV_BUF_SIZE
#define AI_REV_BUF_SIZE     1024*4
#endif
#define AI_CMD_CODE_SIZE    2
#define APP_AI_REV_QUEUE_RESET_TIMEOUT_INTERVEL    1000

typedef struct
{
    uint16_t    data_len;
    uint8_t     data[MAX_TX_BUFF_SIZE];
} AI_TRANSPORT_DATA_T;

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int transport_data_index_in;
extern unsigned int transport_data_index_out;
extern unsigned int transport_data_len[];
extern CQueue ai_rev_buf_queue;
extern osTimerId app_ai_rev_queue_reset_timeout_timer_id;

void ai_transport_fifo_init(void);
void ai_transport_fifo_deinit(uint8_t ai_index);
void ai_transport_fifo_clean(void);
void ai_data_transport_init(bool  (* cb)(uint8_t*,uint32_t,uint8_t,uint8_t), uint8_t ai_index, uint8_t transType);
void ai_cmd_transport_init(bool (* cb)(uint8_t*,uint32_t,uint8_t,uint8_t), uint8_t ai_index, uint8_t transType);
bool ai_transport_cmd_put(uint8_t *in_buf,uint16_t len);
bool ai_transport_cmd_put_generic(uint8_t *in_buf, uint16_t len, bool isSkipWhenFifoOverflow);
bool ai_transport_has_cmd_to_send(void);
bool ai_transport_handler(uint8_t ai_index, uint8_t device_id);
uint32_t ai_get_ai_cmd_transport_fifo_length(void);

void ai_ble_handler_init(uint32_t ai_spec);

#ifdef __cplusplus
}
#endif
#endif

