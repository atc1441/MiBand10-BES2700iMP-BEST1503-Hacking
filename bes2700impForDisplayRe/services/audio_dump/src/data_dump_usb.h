#ifndef DATA_DUMP_USB_H
#define DATA_DUMP_USB_H

void usb_serial_thread_init(void);

void usb_serial_thread_destroy(void);

void usb_serial_send_data(void);

#endif