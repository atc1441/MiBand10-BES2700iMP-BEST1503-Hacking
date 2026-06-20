#ifndef APP_AI_IF_CUSTOM_UI_H_
#define APP_AI_IF_CUSTOM_UI_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *            app_ai_if_custom_get_ai_cmd_transport_fifo_length
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    for custom to get cmd length in ai_cmd_transport_fifo
 *
 * Parameters:
 *    void
 *
 * Return:
 *    cmd length in fifo
 */
uint32_t app_ai_if_custom_get_ai_cmd_transport_fifo_length(void);

/*---------------------------------------------------------------------------
 *            app_ai_if_custom_send_rest_cmd_in_ai_cmd_transport_fifo
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    for custom to send the rest of cmd in ai_cmd_transport_fifo,
 *    because it is possible for ai_thread fail to send cmd due to lack of tx_credits.
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ai_if_custom_send_rest_cmd_in_ai_cmd_transport_fifo(void);

/*---------------------------------------------------------------------------
 *            app_ai_if_custom_ui_send_cmd
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    for custom to send cmd
 *
 * Parameters:
 *    cmd_buf -- cmd data buff
 *    cmd_len -- cmd data length
 *
 * Return:
 *    void
 */
bool app_ai_if_custom_ui_send_cmd(uint8_t *cmd_buf, uint16_t cmd_len);
bool app_ai_if_custom_ui_send_cmd_generic(uint8_t *cmd_buf, uint16_t cmd_len, bool isSkipWhenFifoOverflow);

/*---------------------------------------------------------------------------
 *            app_ai_if_custom_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    init custom
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ai_if_custom_init(void);

#ifdef __cplusplus
    }
#endif


#endif //APP_AI_IF_CUSTOM_UI_H_

