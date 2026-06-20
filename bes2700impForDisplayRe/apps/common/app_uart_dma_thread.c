#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "string.h"
#include "app_uart_dma_thread.h"

/** configurations for uart dma thread */

// enable this macro for loopback test
#define TEST_UART_LOOP_BACK 0

// could change the APP_UART_ID according to custom user case
#define APP_UART_ID HAL_UART_ID_1

// uart baudrate
#define CONFIG_UART_BAUDRATE    1152000

// uart receive buffer size
#define UART_BUF_SIZE_IN_BYTES          640
#define UART_BUF_SIZE_MARGIN_IN_BYTES   512
#define UART_REC_CHUNK_SIZE             (UART_BUF_SIZE_MARGIN_IN_BYTES + UART_BUF_SIZE_IN_BYTES)


/** module used macros */
#define SIGNAL_OF_UART_DATA_RECEIVED    0x01
#define SIGNAL_OF_UART_DATA_TRANSMITTED 0x02

#define UART_TX_FLUSH_TIME_US           1000
#define UART_TX_FLUSH_DELAY             10

#define APP_UART_TX_TIMEOUT_MS          100

static bool is_app_uart_xfer_cancled = false;
static bool is_app_uart_receiving_started = false;

static uint32_t debug_uart_rx_size, debug_uart_tx_size;
static int debug_uart_rx_dma_error, debug_uart_tx_dma_error;
static union HAL_UART_IRQ_T debug_uart_rx_dma_status;

static app_uart_data_received_callback_handler_t data_receiver_callback = NULL;

typedef struct
{
    uint16_t input_cursor;
    uint16_t output_cursor;
    uint32_t bytesInBuffer;
    uint32_t bytes_in_margin_buffer;
    uint8_t  uart_buf[UART_REC_CHUNK_SIZE];
} UART_DATA_BUF_ENV_T;

static osThreadId dma_tx_thread_id = NULL;
static UART_DATA_BUF_ENV_T uart_data_buf_env;
osThreadId app_uart_dma_thread_id;
static void app_uart_dma_thread(const void *arg);

osThreadDef(app_uart_dma_thread, osPriorityHigh, 1, 1024*3, "app_uart_dma_thread");

#if TEST_UART_LOOP_BACK
static void app_process_uart(uint8_t *ptr, uint32_t len)
{
    TRACE(0, "app_process_uart: %s", ptr);
}
#endif

static void app_uart_break_handler(void)
{
    TRACE(0, "uart break.");
    hal_uart_stop_dma_recv(APP_UART_ID);
    hal_uart_stop_dma_send(APP_UART_ID);
}

void app_uart_set_tx_thread_singal_id(osThreadId threadId)
{
    dma_tx_thread_id = threadId;
}

static void uart_dma_rx(uint32_t xfer_size, int dma_error, union HAL_UART_IRQ_T status)
{
    if (!is_app_uart_receiving_started)
    {
        return;
    }

    if (dma_error) {
        // uart_rx dma error
        debug_uart_rx_size = 0;
        app_uart_break_handler();
        return;
    } else if (status.BE || status.FE || status.OE || status.PE) {
        // uart_rx uart error
        debug_uart_rx_size = 0;
        app_uart_break_handler();
        return;
    } else {
        debug_uart_rx_size = xfer_size;
    }

    TRACE(0, "uart rx size %d", debug_uart_rx_size);

    debug_uart_rx_dma_error = dma_error;
    debug_uart_rx_dma_status = status;

    if (0 == debug_uart_rx_size)
    {
        app_uart_break_handler();
    }

    uint32_t bytes_in_buffer;
    uint32_t end_index_in_buffer;
    uint32_t lock = int_lock();
    uart_data_buf_env.bytesInBuffer += debug_uart_rx_size;
    bytes_in_buffer = uart_data_buf_env.bytesInBuffer;
    end_index_in_buffer = uart_data_buf_env.input_cursor + debug_uart_rx_size;

    if (end_index_in_buffer >= UART_BUF_SIZE_IN_BYTES)
    {
        // Only need to wrap the input cursor.
        uart_data_buf_env.input_cursor = 0;
        ASSERT(uart_data_buf_env.bytes_in_margin_buffer == 0,
               "UART data corruption: bytes_in_buffer is %d", bytes_in_buffer);
        uart_data_buf_env.bytes_in_margin_buffer =
            end_index_in_buffer - UART_BUF_SIZE_IN_BYTES;
    }
    else
    {
        uart_data_buf_env.input_cursor += debug_uart_rx_size;
    }

    int_unlock(lock);

    osSignalSet(app_uart_dma_thread_id, SIGNAL_OF_UART_DATA_RECEIVED);

    union HAL_UART_IRQ_T mask;
    mask.reg = 0;
    mask.RT = 1;

    hal_uart_dma_recv_mask(
        APP_UART_ID,
        &(uart_data_buf_env.uart_buf[uart_data_buf_env.input_cursor]),
        UART_BUF_SIZE_MARGIN_IN_BYTES, NULL, NULL, &mask);
}

static void uart_dma_tx(uint32_t xfer_size, int dma_error)
{
    if (dma_error) {
        // uart_tx dma error
        debug_uart_tx_size = 0;
    }
    else
    {
        debug_uart_tx_size = xfer_size;
    }

    debug_uart_tx_dma_error = dma_error;

    osSignalSet(dma_tx_thread_id, SIGNAL_OF_UART_DATA_TRANSMITTED);
}

WEAK void app_uart_configuration_handler(struct HAL_UART_CFG_T* pCfg)
{
    memset(pCfg, 0, sizeof(struct HAL_UART_CFG_T));
    pCfg->parity = HAL_UART_PARITY_NONE,
    pCfg->stop = HAL_UART_STOP_BITS_1,
    pCfg->data = HAL_UART_DATA_BITS_8,
    pCfg->flow = HAL_UART_FLOW_CONTROL_NONE,
    pCfg->tx_level = HAL_UART_FIFO_LEVEL_1_2,
    pCfg->rx_level = HAL_UART_FIFO_LEVEL_1_4,
    pCfg->baud = CONFIG_UART_BAUDRATE,
    pCfg->dma_rx = true,
    pCfg->dma_tx = true,
    pCfg->dma_rx_stop_on_err = false;
}

static void uart_dma_init(void)
{
    struct HAL_UART_CFG_T uart_cfg;

    app_uart_configuration_handler(&uart_cfg);

    hal_uart_open(APP_UART_ID, &uart_cfg);

    hal_uart_irq_set_dma_handler(APP_UART_ID, uart_dma_rx, uart_dma_tx);
}

static bool uart_dma_send_data(const unsigned char *buf, unsigned int len)
{
#if 0
    int outputLength = hal_trace_output(buf, len);
    if (outputLength > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
#else

    osEvent evt;
    debug_uart_tx_size = 0;

    app_uart_set_tx_thread_singal_id(osThreadGetId());
    hal_uart_dma_send(APP_UART_ID, buf, len, NULL, NULL);
    evt = osSignalWait(SIGNAL_OF_UART_DATA_TRANSMITTED, APP_UART_TX_TIMEOUT_MS);

    if (evt.status == osEventTimeout)
    {
        return false;
    }
    else
    {
        return true;
    }
#endif
}

static void app_uart_dma_thread(const void *arg)
{
   while (true)
   {
      osEvent evt = osSignalWait(0x0, osWaitForever);

      int32_t signals = evt.value.signals;
      if(evt.status == osEventSignal && signals == SIGNAL_OF_UART_DATA_RECEIVED)
      {
        uint32_t bytesInBuffer, end_index_in_buffer, bytes_in_margin_buffer;
        uint32_t lock;
        lock = int_lock();
        bytesInBuffer = uart_data_buf_env.bytesInBuffer;
        bytes_in_margin_buffer = uart_data_buf_env.bytes_in_margin_buffer;
        int_unlock(lock);

        while (bytesInBuffer > 0) {
          end_index_in_buffer = uart_data_buf_env.output_cursor + bytesInBuffer;
          uint8_t *ptr = &(uart_data_buf_env.uart_buf[uart_data_buf_env.output_cursor]);
          if (bytes_in_margin_buffer > 0) {
            uint32_t end_index =
                UART_BUF_SIZE_IN_BYTES + bytes_in_margin_buffer;
            if (end_index_in_buffer != end_index) {
              bytesInBuffer = end_index - uart_data_buf_env.output_cursor;
              end_index_in_buffer = end_index;
            }
          }

#if TEST_UART_LOOP_BACK
          if (bytesInBuffer > 0)
          {
              uart_dma_send_data(ptr, bytesInBuffer);
          }
#endif
          if (data_receiver_callback)
          {
              data_receiver_callback(ptr, bytesInBuffer);
          }
          lock = int_lock();
          uart_data_buf_env.output_cursor =
              end_index_in_buffer >= UART_BUF_SIZE_IN_BYTES ?
              0 : end_index_in_buffer;
          uart_data_buf_env.bytesInBuffer -= bytesInBuffer;
          bytesInBuffer = uart_data_buf_env.bytesInBuffer;
          if (bytes_in_margin_buffer > 0) {
            uart_data_buf_env.bytes_in_margin_buffer -= bytes_in_margin_buffer;
            bytes_in_margin_buffer = uart_data_buf_env.bytes_in_margin_buffer;
          }
          int_unlock(lock);
        }
      }
   }
}

static void app_uart_configure(void)
{
    is_app_uart_xfer_cancled = false;
    uart_dma_init();
}

void app_uart_init(void)
{
    TRACE(0, "app_uart_init");

    // Wait for Tx queue to flush
    for (int i = 0; i < UART_TX_FLUSH_TIME_US / UART_TX_FLUSH_DELAY ; i++)
    {
        if (hal_trace_busy())
        {
            hal_sys_timer_delay_us(UART_TX_FLUSH_DELAY);
        }
        else
        {
            break;
        }
    }
    if (APP_UART_ID == HAL_UART_ID_0)
    {
        hal_iomux_set_uart0();
    }
#ifdef UART1_BASE
    else if (APP_UART_ID == HAL_UART_ID_1)
    {
        hal_iomux_set_uart1();
    }
#endif
#ifdef UART2_BASE
    else if (APP_UART_ID == HAL_UART_ID_2)
    {
        hal_iomux_set_uart2();
    }
#endif
    else
    {
        TRACE(0, "Invalid application uart id %d", APP_UART_ID);
    }

    app_uart_configure();

#if TEST_UART_LOOP_BACK
    // shall be called by specifc module
    app_uart_register_data_receiver(app_process_uart);
    app_uart_start_receiving_data();
#endif

    app_uart_dma_thread_id = osThreadCreate(osThread(app_uart_dma_thread), NULL);

}

void app_uart_close(void)
{
    uint32_t lock = int_lock();
    hal_uart_close(APP_UART_ID);
    is_app_uart_receiving_started = false;
    int_unlock(lock);
}

void app_uart_send_data(uint8_t* buf, uint32_t len)
{
    if (is_app_uart_xfer_cancled)
    {
        return;
    }

    bool ret = uart_dma_send_data(buf, len);
    if (!ret)
    {
        TRACE(0, "app uart sending data failed!");
        app_uart_configure();
    }
}

bool app_uart_start_receiving_data(void)
{
    bool ret = true;

    if (is_app_uart_xfer_cancled)
    {
        return false;
    }

    if (is_app_uart_receiving_started)
    {
        TRACE(0, "app uart receiving is already started.");
        return false;
    }

    union HAL_UART_IRQ_T mask;
    mask.reg = 0;

    mask.BE = 1;
    mask.FE = 1;
    mask.OE = 1;
    mask.PE = 1;
    mask.RT = 1;
    mask.RX = 0;
    mask.TX = 0;

    uart_data_buf_env.bytesInBuffer = 0;
    uart_data_buf_env.bytes_in_margin_buffer = 0;
    uart_data_buf_env.input_cursor = 0;
    uart_data_buf_env.output_cursor = 0;

    is_app_uart_xfer_cancled = false;

    // start non-blocking uart dma receiving
    int retVal = hal_uart_dma_recv_mask(APP_UART_ID, &(uart_data_buf_env.uart_buf[0]), UART_REC_CHUNK_SIZE, NULL, NULL, &mask);
    if (retVal)
    {
        ret = false;
        goto _exit;
    }

_exit:
    if (!ret)
    {
        TRACE(0, "app uart start receiving failed!");
        app_uart_configure();
    }
    else
    {
        is_app_uart_receiving_started = true;
    }

    return ret;
}

void app_uart_register_data_receiver(app_uart_data_received_callback_handler_t func)
{
    data_receiver_callback = func;
}

void app_uart_stop_uart_rx(void)
{
    uint32_t lock = int_lock();
    if (is_app_uart_receiving_started)
    {
        union HAL_UART_IRQ_T mask;
        mask.reg = 0;
        hal_uart_irq_set_mask(APP_UART_ID, mask);
        hal_uart_stop_dma_recv(APP_UART_ID);
        is_app_uart_receiving_started = false;
    }

    int_unlock(lock);
}

