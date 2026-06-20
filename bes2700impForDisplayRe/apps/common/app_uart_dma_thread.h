#ifndef __APP_UART_DMA_THREAD_H__
#define __APP_UART_DMA_THREAD_H__

#include <stdint.h>
#include <stddef.h>

typedef void (*app_uart_data_received_callback_handler_t)(uint8_t*, uint32_t);

#ifdef __cplusplus
extern "C" {
#endif
    void app_uart_init(void);
    void app_uart_register_data_receiver(app_uart_data_received_callback_handler_t func);
    void app_uart_send_data(uint8_t* buf, uint32_t len);
    void app_uart_set_tx_thread_singal_id(osThreadId threadId);
    bool app_uart_start_receiving_data(void);
    void app_uart_close(void);
    void app_uart_stop_uart_rx(void);

#ifdef __cplusplus
}
#endif

#endif//__APP_UART_DMA_THREAD_H__
