#include "hal_usb.h"
#include "hal_cmu.h"
#include "usb_cdc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "pmu.h"
#include "hwtimer_list.h"
#include "audio_dump.h"
#include <string.h>

// #define USB_SERIAL_ECHO_TEST
// #define USB_SERIAL_SINE_TEST

#if defined(USB_SERIAL_SINE_TEST)
#include "signal_generator.h"
#endif

#define USB_SERIAL_DATA_SIZE (512)

void usb_serial_thread(void const *argument);

/* thread */
#define USB_SERIAL_STACK_SIZE 2048
static osThreadId usb_serial_tid;
osThreadDef(usb_serial_thread, osPriorityAboveNormal, 1, USB_SERIAL_STACK_SIZE, "usb_serial");

static void usb_serial_state_handler(enum USB_SERIAL_STATE_EVENT_T event)
{
    TRACE(1, "usb dev state change: %d", event);
}

static void usb_serial_break_handler(uint16_t ms)
{
    TRACE(0, "Detect break!");
}

#if defined(USB_SERIAL_ECHO_TEST)
static void usb_serial_recv_timeout(void *param)
{
    usb_serial_cancel_recv();
    //TRACE("usb_serial_recv_timeout");
}
#endif

static const struct USB_SERIAL_CFG_T cdc_cfg = {
    .state_callback = usb_serial_state_handler,
    .break_callback = usb_serial_break_handler,
    .mode = USB_SERIAL_API_NONBLOCKING,
};

void usb_serial_thread_init(void)
{
    TRACE(0, "usb_serial_thread_init");

    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);

    usb_serial_open(&cdc_cfg);
    TRACE(1, "[0x%X] Open USB serial", hal_sys_timer_get());

    usb_serial_flush_recv_buffer();
    usb_serial_init_xfer();

    usb_serial_tid = osThreadCreate(osThread(usb_serial_thread), NULL);

#if defined(USB_SERIAL_SINE_TEST)
    signal_generator_init(SG_TYPE_TONE_SIN_1K, 16000, 16, 1);
#endif
}

void usb_serial_thread_destroy(void)
{
    osThreadTerminate(usb_serial_tid);

    usb_serial_close();
}

#if defined(USB_SERIAL_ECHO_TEST) || defined(USB_SERIAL_SINE_TEST)
static uint8_t usb_serial_data[USB_SERIAL_DATA_SIZE];
#endif

#if defined(USB_SERIAL_SINE_TEST)
#define TRACE_STRING "[trace] hello"
#define DUMP_STRING "[trace][audio dump]"
#endif

#define USB_AUDIO_DUMP_SIGNAL_READY   1

void usb_serial_send_data(void)
{
    osSignalSet(usb_serial_tid, 0x01 << USB_AUDIO_DUMP_SIGNAL_READY);
}

void usb_serial_thread(void const *argument)
{
    int result;

#if defined(USB_SERIAL_ECHO_TEST)
    unsigned int i;
    HWTIMER_ID timer;

    timer = hwtimer_alloc(usb_serial_recv_timeout, NULL);
    ASSERT(timer, "Failed to alloc usb serial recv timer");
#endif

    TRACE(0, "usb_serial_thread");

    // Check configuration status
    while (usb_serial_ready() == 0);
    TRACE(0, "usb serial ready");

    // Check terminal connection status
    while (usb_serial_connected() == 0);
    TRACE(0, "usb serial connected");

    while (1) {
#if defined(USB_SERIAL_ECHO_TEST)
        i = 0;

        while (i < ARRAY_SIZE(usb_serial_data)) {

            hwtimer_start(timer, MS_TO_TICKS(10));

            result = usb_serial_recv(usb_serial_data + i, 1);

            hwtimer_stop(timer);
            if (result != 0) {
                //TRACE("usb_serial_recv result: %d", result);
                // Probably timeout
                usb_serial_init_xfer();
                break;
            }
            i++;
        }

        if (i) {
            result = usb_serial_send(usb_serial_data, i);
            DUMP16("%04x ", usb_serial_data, i / 2);
            if (result != 0) {
                TRACE(1, "Failed to send to usb serial: ret=%d", result);
            }
        }
#else
        osEvent evt = osSignalWait(0x0, osWaitForever);
        uint32_t signals = evt.value.signals;

        // TRACE(2, "[%s] signals = 0x%x", __FUNCTION__, signals);

        if (evt.status == osEventSignal) {
            if (signals & (1 << USB_AUDIO_DUMP_SIGNAL_READY)) {
                uint8_t *buf = NULL;
                uint32_t len = 0;

#if defined(USB_SERIAL_SINE_TEST)
                buf = usb_serial_data;

                memcpy(usb_serial_data, TRACE_STRING, sizeof(TRACE_STRING) - 1);
                len += sizeof(TRACE_STRING) - 1;

                memcpy(&usb_serial_data[len], DUMP_STRING, sizeof(DUMP_STRING) - 1);
                len += sizeof(DUMP_STRING) - 1;

                int *data_len = (int *)&usb_serial_data[len];
                *data_len = 10 * sizeof(int16_t);
                len += sizeof(int);

                signal_generator_loop_get_data(&usb_serial_data[len], 10);
                len += 10 * sizeof(int16_t);
#else
                audio_dump_get_buff_info(&buf, &len);
#endif

                audio_dump_buffer_mutex_lock();
                result = usb_serial_send(buf, len);
                audio_dump_buffer_mutex_unlock();
                if (result != 0) {
                    TRACE(1, "Failed to send to usb serial: ret=%d", result);
                    usb_serial_init_xfer();
                }
            }
        }
#endif
    }
}
