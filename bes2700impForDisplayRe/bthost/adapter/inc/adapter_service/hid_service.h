/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BT_HID_SERVICE_H__
#define __BT_HID_SERVICE_H__
#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

struct bt_hid_sensor_report_t {
    int16_t rx; // -32767 to 32767 represent [-314159265, 314159265] * 10^-8 rad
    int16_t ry;
    int16_t rz;
    int16_t vx; // -32767 to 32767 represent [-32, 32] rad/sec
    int16_t vy;
    int16_t vz;
    uint8_t counter;
} __attribute__ ((packed));

#ifdef HOGP_HEAD_TRACKER_PROTOCOL
bt_status_t ble_hogp_send_sensor_report(const struct bt_hid_sensor_report_t *report);
#endif

#define HID_DESCRIPTOR_TYPE     0x22 // report descriptor type

#define HID_BOOT_PROTOCOL_KEYBOARD_REPORT_ID  0x01 // 9-octets, 1-octet report id + standard 8-octet keyboard boot report
#define HID_BOOT_PROTOCOL_MOUSE_REPORT_ID     0x02 // 4-octets, 1-octet report id + standard 3-octet mouse boot report

#define HID_KEYBOARD_INPUT_REPORT_ID        0x01
#define HID_MOUSECLK_INPUT_REPORT_ID        0x02
#define HID_MEDIAKEY_INPUT_REPORT_ID        0x03
#define HID_MOUSECTL_INPUT_REPORT_ID        0x04
#define HID_CONSUMER_CONTROL_INPUT_REPORT_ID        0x01

/** Modified or Constant value
 *
 * Data indicates the item is defining report fields
 * that contain modifiable device data.
 *
 * Constant indicates the item is a static read-only
 * field in a report and cannot be modified (written)
 * by the host.
 */

#define HID_DATA 0x00 // bit0 - 0 data, 1 constant
#define HID_CNST 0x01

#define HID_ARR  0x00 // bit1 - 0 array, 1 variable
#define HID_VAR  0x02

/** Abslute or Relative value
 *
 * Indicates whether the data is absolute (based on a
 * fixed origin) or relative (indicating the change in
 * value from the last report). 
 * 
 * Mouse devices usually provide relative data, while
 * tablets usually provide absolute data.
 */

#define HID_ABS  0x00 // bit2 - 0 absolute, 1 relative
#define HID_REL  0x04

#define HID_NWRP 0x00 // bit3 - 0 no wrap, 1 wrap
#define HID_WRAP 0x08

#define HID_LINE 0x00 // bit4 - 0 linear, 1 non linear
#define HID_NLIN 0x10

#define HID_PREF 0x00 // bit5 - 0 preferred state, 1 no preferred
#define HID_NPRF 0x20

#define HID_NNUL 0x00 // bit6 - 0 no null position, 1 null state
#define HID_NULL 0x40

#define HID_NVLA 0x00 // bit7 - 0 non volatile, 1 volatile
#define HID_VOLA 0x80

#define HID_USAGE_PAGE_GENERIC_DESKTOP 0x01
#define HID_USAGE_ID_POINTER    0x01 // CP - collection-related usage type - physical collection
#define HID_USAGE_ID_MOUSE      0x02 // CA - collection-related usage type  application collection
#define HID_USAGE_ID_JOYSTICK   0x04 // CA
#define HID_USAGE_ID_GAMEPAD    0x05 // CA
#define HID_USAGE_ID_KEYBOARD   0x06 // CA
#define HID_USAGE_ID_KEYPAD     0x07 // CA
#define HID_USAGE_ID_X          0x30 // DV, a linear translation in the X direction, increase when moving from left to right
#define HID_USAGE_ID_Y          0x31 // DV, a linear translation in the Y direction, increase when moving from far to near
#define HID_USAGE_ID_Z          0x32 // DV, a linear translation in the Z direction, increase when moving from high to low

#define HID_USAGE_PAGE_KEY_CODES 0x07
#define HID_USAGE_ID_KEYCODE_A          0x04 // Sel - data-related usage type, Selector, contained in a named array
#define HID_USAGE_ID_KEYCODE_Z          0x1d // Sel
#define HID_USAGE_ID_KEYCODE_1          0x1e // Sel
#define HID_USAGE_ID_KEYCODE_9          0x26 // Sel
#define HID_USAGE_ID_KEYCODE_0          0x27 // Sel
#define HID_USAGE_ID_KEYCODE_ENTER      0x28 // Sel
#define HID_USAGE_ID_KEYCODE_ESC        0x29 // Sel
#define HID_USAGE_ID_KEYCODE_DEL        0x2a // Sel
#define HID_USAGE_ID_KEYCODE_TAB        0x2b // Sel
#define HID_USAGE_ID_KEYCODE_SPACE      0x2c // Sel
#define HID_USAGE_ID_KEYCODE_VOLUP      0x80 // Sel
#define HID_USAGE_ID_KEYCODE_VOLDN      0x81 // Sel

#define HID_USAGE_ID_KEYCODE_L_CTRL     0xe0 // DV - data-related usage type, Dynamic Value, a RW multiple-bit value
#define HID_USAGE_ID_KEYCODE_L_SHIFT    0xe1 // DV
#define HID_USAGE_ID_KEYCODE_L_ALT      0xe2 // DV
#define HID_USAGE_ID_KEYCODE_L_WIN      0xe3 // DV
#define HID_USAGE_ID_KEYCODE_R_CTRL     0xe4 // DV
#define HID_USAGE_ID_KEYCODE_R_SHIFT    0xe5 // DV
#define HID_USAGE_ID_KEYCODE_R_ALT      0xe6 // DV
#define HID_USAGE_ID_KEYCODE_R_WIN      0xe7 // DV

#define HID_USAGE_PAGE_LEDS     0x08
#define HID_USAGE_ID_NUM_LOCK           0x01 // OOC - control related usage type, On/Off Control
#define HID_USAGE_ID_CAPS_LOCK          0x02 // OOC
#define HID_USAGE_ID_SCROLL_LOCK        0x03 // OOC
#define HID_USAGE_ID_COMPOSE            0x04 // OOC
#define HID_USAGE_ID_KANA               0x05 // OOC

#define HID_USAGE_PAGE_BUTTONS  0x09
#define HID_USAGE_ID_BUTTON_1           0x01 // primary/trigger
#define HID_USAGE_ID_BUTTON_2           0x02 // secondary
#define HID_USAGE_ID_BUTTON_3           0x03 // tertiary

#define HID_USAGE_PAGE_CONSUMER 0x0c
#define HID_USAGE_ID_CONSUMER_CONTROL       0x01 // CA
#define HID_USAGE_ID_NUMERIC_KEY_PAD        0x02 // NAry - named array
#define HID_USAGE_ID_PROG_BUTTONS           0x03 // NAry
#define HID_USAGE_ID_MICROPHONE             0x04 // CA
#define HID_USAGE_ID_HEADPHONE              0x05 // CA
#define HID_USAGE_ID_GRAPHIC_EQUALIZER      0x06 // CA
#define HID_USAGE_ID_POWER                  0x30 // OOC - On/Off Control
#define HID_USAGE_ID_PLAY                   0xb0 // OOC
#define HID_USAGE_ID_PAUSE                  0xb1 // OOC
#define HID_USASG_ID_RECORD                 0xb2 // OOC
#define HID_USAGE_ID_FAST_FORWARD           0xb3 // OOC
#define HID_USAGE_ID_FAST_BACKWARD          0xb4 // OOC
#define HID_USAGE_ID_NEXT_TRACK             0xb5 // OSC - One Shot Control
#define HID_USAGE_ID_PREV_TRACK             0xb6 // OSC
#define HID_USAGE_ID_STOP                   0xb7 // OSC - stop scanning, streaming, or recording linear media
#define HID_USAGE_ID_EJECT                  0xb8 // OSC
#define HID_USAGE_ID_RANDOM_PLAY            0xb9 // OOC
#define HID_USAGE_ID_REPEAT_PLAY            0xbc // OSC
#define HID_USAGE_ID_PLAY_PAUSE             0xcd // OSC
#define HID_USAGE_ID_MUTE_SWITCH            0xe2 // OOC - press to mute, release to resotre prev volume
#define HID_USAGE_ID_VOLUME_INC             0xe9 // RTC - Re-trigger Control
#define HID_USAGE_ID_VOLUME_DEC             0xea // RTC
#define HID_USAGE_ID_CONSUMER_CONTROL_CONF  0x83,0x01 // Application Launch Buttons - Sel
#define HID_USAGE_ID_EMAIL_READER           0x8a,0x01 // AL - Sel
#define HID_USAGE_ID_CALCULATOR             0x92,0x01 // AL - Sel
#define HID_USAGE_ID_LOCAL_MACHINE_BROWSER  0x94,0x01 // AL - Sel
#define HID_USAGE_ID_SCREEN_SAVER           0xb1,0x01 // AL - Sel
#define HID_USAGE_ID_ALARMS                 0xb2,0x01 // AL - Sel
#define HID_USAGE_ID_CLOCK                  0xb3,0x01 // AL - Sel
#define HID_USAGE_ID_FILE_BROSWER           0xb4,0x01 // AL - Sel
#define HID_USAGE_ID_SEARCH                 0x21,0x02 // Generic GUI Application Controls - Sel
#define HID_USAGE_ID_HOME                   0x23,0x02 // AC - Sel
#define HID_USAGE_ID_BACK                   0x24,0x02 // AC - Sel
#define HID_USAGE_ID_FORWARD                0x25,0x02 // AC - Sel
#define HID_USAGE_ID_REFRESH                0x27,0x02 // AC - Sel
#define HID_USAGE_ID_BOOKMARKS              0x2a,0x02 // AC - Sel
#define HID_USAGE_ID_ZOOM_IN                0x2d,0x02 // AC - Sel
#define HID_USAGE_ID_ZOOM_OUT               0x2e,0x02 // AC - Sel
#define HID_USAGE_ID_FULLSCREEN             0x30,0x02 // AC - Sel
#define HID_USAGE_ID_NORMAL_VIEW            0x31,0x02 // AC - Sel
#define HID_USAGE_ID_SCROLL_UP              0x33,0x02 // AC - Sel
#define HID_USAGE_ID_SCROLL_DOWN            0x34,0x02 // AC - Sel
#define HID_USAGE_ID_SCROLL_AAA            0x38,0x02 // AC - Sel


#define HID_USAGE_PAGE_SIMULATION_CONTROLS      0x02
#define HID_USAGE_PAGE_VR_CONTROLS              0x03
#define HID_USAGE_PAGE_SPORT_CONTROLS           0x04
#define HID_USAGE_PAGE_GAME_CONTROLS            0x05
#define HID_USAGE_PAGE_GENERIC_DEVICE_CONTROLS  0x06
#define HID_USAGE_PAGE_ORDINAL                  0x0a
#define HID_USAGE_PAGE_TELEPHONY_DEVICE         0x0b
#define HID_USAGE_PAGE_DIGITIZERS               0x0d
#define HID_USAGE_PAGE_HAPTICS                  0x0e

/** Report Descriptor
 *
 * A Report descriptor describes each piece of data that the device generates
 * and what the data is actually measuring. A Report descriptor deinfes items
 * and each item describe the item information like a poistion or button state.
 * An item is a piece of information about the device.
 *
 * One or more fields of data from controls are defined by a Main item and
 * further described by a preceding Global and Local items. Local items
 * only describe the data fields defined by the next Main item. Global items
 * become the default attributes for all subsequent data fields in that
 * descriptor.
 *
 * A Report descriptor may contain several Main items.
 * A Report descriptor must include each of the following
 * items to describe a control's data (all other items are optional):
 * . Input (Output or Feature) - Main Item
 * . Usage                     - Local Item
 * . Usage Page                - Global Item
 * . Logical Minimum           - Global Item
 * . Logical Maximum           - Global Item
 * . Report Size               - Global Item
 * . Report Count              - Global Item
 *
 * If Report IDs are used, then a Report ID must be declared prior
 * to the first Input, Output, or Feature main item declaration in
 * a report descriptor.
 */

/** Main Item
 *
 * . Input          bTag 1000 bType 00 bSize nn | {b8_Buffered,b6_NullState,b5_NoPrefer,b4_NoLinear,b3_Wrap,b2_Relative,b1_Variable,b0_Constant}
 * . Output         bTag 1001 bType 00 bSize nn | ''+b7_Volatile
 * . Feature        bTag 1011 bType 00 bSize nn | ''''
 * . End Collection bTag 1100 bType 00 bSize 00 | N/A
 * . Collection     bTag 1010 bType 00 bSize 01 | 0x00 ~ 0xff
 *      - 0x00 Physical (group of axes)
 *      - 0x01 Application (mouse, keyboard)
 *      - 0x02 Logical (interrelated data)
 *      - 0x03 Report
 *      - 0x04 Named array
 *      - 0x05 Usage Switch
 *      - 0x06 Usage Modifier
 *      - 0x07 ~ 0x7f Reserved
 *      - 0x80 ~ 0xff Vendor-defined
 */

#define HID_MAIN_ITEM_INPUT(d)          0x81,d
#define HID_MAIN_ITEM_OUTPUT(d)         0x91,d
#define HID_MAIN_ITEM_FEATURE(d)        0xb1,d
#define HID_MAIN_ITEM_COLLECTION(c)     0xa1,c
#define HID_MAIN_ITEM_END_COLLECTION()  0xc0

#define HID_COLLECTION_PHYSICAL         0x00
#define HID_COLLECTION_APPLICATION      0x01
#define HID_COLLECTION_LOGICAL          0x02
#define HID_COLLECTION_REPORT           0x03
#define HID_COLLECTION_NAMED_ARRAY      0x04
#define HID_COLLECTION_USAGE_SWITCH     0x05
#define HID_COLLECTION_USAGE_MODIFIER   0x06

/** Global Item
 *
 * . Usage Page - 0000 bType 01 bSize 01/02
 * . Logical Min  0001 bType 01 bSize 01
 * . Logical Max  0010 bType 01 bSize 01
 * . Pysical Min  0011 bType 01 bSize 01
 * . Pysical Max  0100 bType 01 bSize 01
 * . UnitExponent 0101 bType 01 bSize 01
 * . Unit         0110 bType 01 bSize 01
 * . Report Size  0111 bType 01 bSize 01
 * . Report ID    1000 bType 01 bSize 01
 * . Report Count 1001 bType 01 bSize 01
 * . Push         1010 bType 01 bSize 00
 * . Pop          1011 bType 01 bSize 00
 */

#define HID_GLOBAL_ITEM_USAGE_PAGE(n)               0x05,n // specifies the current usage page
#define HID_GLOBAL_ITEM_USAGE_PAGE_2B(n)            0x06,n // specifies the current usage page
#define HID_GLOBAL_ITEM_LOGICAL_MIN(n)              0x15,n // extent value in logical units, this is the min value
#define HID_GLOBAL_ITEM_LOGICAL_MAX(n)              0x25,n // extent value in logical units, this is the max value
#define HID_GLOBAL_ITEM_LOGICAL_MIN_2B(l,h)         0x16,l,h
#define HID_GLOBAL_ITEM_LOGICAL_MAX_2B(l,h)         0x26,l,h
#define HID_GLOBAL_ITEM_PHYSICAL_MIN(n)             0x35,n // min value for the physical extent of a variable item
#define HID_GLOBAL_ITEM_PHYSICAL_MAX(n)             0x45,n // max value for the physical extent of a variable item
#define HID_GLOBAL_ITEM_PHYSICAL_MIN_2B(l,h)        0x36,l,h
#define HID_GLOBAL_ITEM_PHYSICAL_MAX_2B(l,h)        0x46,l,h
#define HID_GLOBAL_ITEM_PHYSICAL_MIN_4B(l,n,m,h)    0x37,l,n,m,h
#define HID_GLOBAL_ITEM_PHYSICAL_MAX_4B(l,n,m,h)    0x47,l,n,m,h
#define HID_GLOBAL_ITEM_UNIT_EXPONENT(n)            0x55,n // value of the unit exponent in base 10
#define HID_GLOBAL_ITEM_UNIT(n)                     0x65,n // unit values
#define HID_GLOBAL_ITEM_UNIT_2B(n)                  0x66,n // unit values
#define HID_GLOBAL_ITEM_REPORT_SIZE(n)              0x75,n // specifies the size of the report fields in bits
#define HID_GLOBAL_ITEM_REPORT_ID(n)                0x85,n // specifies the report id
#define HID_GLOBAL_ITEM_REPORT_COUNT(n)             0x95,n // specifies the number of data fields for the item

/** Local Item
 *
 * . Usage ID           0000 bType 10 bSize 01/02
 * . Usage Min          0001 bType 10 bSize 01
 * . Usage Max          0010 bType 10 bSize 01
 * . Designator Idx     0011 bType 10 bSize 01
 * . Designator Min     0100 bType 10 bSize 01
 * . Designator Max     0101 bType 10 bSize 01
 * . String Index       0111 bType 10 bSize 01
 * . String Min         1000 bType 10 bSize 01
 * . String Max         1001 bType 10 bSize 01
 * . Delimiter          1010 bType 10 bSize 01
 */

#define HID_LOCAL_ITEM_USAGE_ID(n)          0x09,n // represents a suggested usage id for the item or collection
#define HID_LOCAL_ITEM_USAGE_ID_2B(n)       0x0a,n // represents a suggested usage id for the item or collection
#define HID_LOCAL_ITEM_USAGE_MIN(n)         0x19,n // defines the starting usage associated with an array or bitmap
#define HID_LOCAL_ITEM_USAGE_MAX(n)         0x29,n // defines the ending usage associated with an array or bitmap
#define HID_LOCAL_ITEM_DESIGNATOR_INDEX(n)  0x39,n // determines the body part used for a control
#define HID_LOCAL_ITEM_DESIGNATOR_MIN(n)    0x49,n // defines the index of the starting designator
#define HID_LOCAL_ITEM_DESIGNATOR_MAX(n)    0x59,n // defines the index of the ending designator
#define HID_LOCAL_ITEM_STRING_INDEX(n)      0x79,n // string index for a string descriptor
#define HID_LOCAL_ITEM_STRING_MIN(n)        0x89,n // specifies the first string index
#define HID_LOCAL_ITEM_STRING_MAX(n)        0x99,n // specifies the last string index
#define HID_LOCAL_ITEM_DELIMITER(n)         0xa9,n // defines the beginning or end of a set of local items (1 open set, 0 close set)

/**
 * Head Tracker HID Protocol
 *
 * The head tracker HID protocol, available for devices running Android T
 * and higher, allows for a head-tracking device to be connected to an Android
 * device through USB or Bluetooth and be exposed to the Android framework
 * and apps through the sensors (/devices/sensors) framework.
 *
 * This procotol is used for controlling an audio virtualizer effect (3D audio).
 * This page uses the terms device and host in their Bluetooth sense, where
 * device means the head-tracking device and host means the Android host.
 *
 * The Android framework identifies the head tracker device as a HID device.
 * At the top level, the head tracker device is an app collection with the
 * Sensors page (0x20) and the Other: Custom usage (0xE1). Inside this
 * collection are several data fields (inputs) and properties (features).
 *
 * Property: Sensor Description (0x0308)
 *
 * The sensor description property is a read-only ASCII (8-bit) string that
 * must contain the following value: "#AndroidHeadTracker#1.0". No null-terminator
 * is expected. This property serves as a discriminator to avoid collisons
 * with other custom sensors.
 *
 * Property: Persistent Unique ID (0x0302)
 *
 * The persistent unique ID property is a read-only array of 16 elements, 8 bit
 * each (total 128 bit). No null terminator is expected. This property is
 * optional. This property allows head-tracking devices that are integrated in
 * audio devices to reference the audio device that they're attached to.
 *
 * The following schemes are supported:
 * 
 * Standalone head tracker - If the persistent unique ID property doesn't exist
 * or is set to all zeros, it means that the head tracker device isn't permanently
 * attached to an audio device and can be used separately, for example, by letting
 * the user manually associate the head tracker device with a separate audio device.
 *
 * Reference using Bluetooth MAC address - In this scheme, the first 8 octets must
 * be 0, octets 8 and 9 must contain the ASCII values B and T respectively, and
 * the following 6 octets are interpreted as a Bluetooth MAC address, assuming the
 * head tracker device applies to any audio device having this MAC address.
 *
 * Reference using UUID - Whenever the most significant bit (MSB) of octet 8 is
 * set (>=0x80), the field is interpreted as a UUID. The corresponding audio device
 * provides the same UUID, which is registered on the Android framework, through
 * an unspecified mechanism that is specific to the type of transport used.
 *
 * Property: Reporting State (0x0316)
 *
 * The reporting state property is a read/write property that has the standard
 * semantics as defined in the HID specification. The host uses this property to
 * indicate to the device which events to report. Only the values No Events (0x0840)
 * and All Events (0x0841) are used. The initial value for this field must be
 * No Events and it must never to modified by the device, only by the host.
 *
 * Property: Power State (0x0319)
 *
 * The power state property is a read/write property that has the standard
 * semantics as defined in the HID specification. The host can uses this property
 * to indicate to the device which power state it must be in. Only the values
 * Full Power (0x0851) and Power Off (0x0855) are used. The initial value for this
 * field is determined by the device and must never be modified by the device,
 * only by the host.
 *
 * Property: Report Interval (0x030E)
 *
 * The report interval property is a read/write property that has the standard
 * semantics as deinfed in the HID specification. The host uses this property
 * to indicate to the device how often to report its data readings. Units are
 * seconds. The valid range for this value is determinted by the device and
 * described using the Physical Min/Max machanism. At least 50Hz reporting rate
 * must be supported, and the recommended max reporting rate is 100Hz. Therefore,
 * the min report interval must be less than or equal to 20ms, and is recommended
 * to be greater than or equal to 10ms.
 *
 * Data field: Custom Value 1 (0x0544)
 *
 * The custom value 1 field is an input field used for reporting the actual
 * head-tracking information. It's a 3-element array, interpreted according to the
 * normal HID rules for physical values as specified in section 6.2.2.7 of the
 * HID specification. The valid range for each element is [-pi,pi] rad. Units
 * are always radians.
 *
 * The elements are interpreted as: [rx, ry, rz], it is a rotation vector,
 * representing the transform from the reference frame to the head frame.
 * Magnitude must be in the [0..pi] range. The reference frame is arbitrary,
 * but it's generally fixed and must be right handed. A small amount of drift
 * is acceptable. The head axes are:
 *      . X from left ear to right
 *      . Y from the back of the head to the nose (back to front)
 *      . Z from the neck to the top of the head
 *
 * Data field: Custom Value 2 (0x0545)
 *
 * The custom value 2 field is an input field used for reporting the actual
 * head-tracking intormation. It's a 3-element fixed-point array, interpreted
 * according to the normal HID rules for physical values. Units are always
 * radians/second. The elements are interpreted as: [vx, vy, vz], it is a
 * rotation vector, representing the angular velocity of the head frame (relative
 * to itself).
 *
 * Data field: Custom Value 3 (0x0546)
 *
 * The custom value 3 field is an input field used for tracking discontinuities
 * in the reference frame. It's scalar integer [4..32] bit in size. It must be
 * incremented (with wraparound) by the device every time the frame of reference
 * is changed, for example, if an orientation filter algorithm used for determing
 * the orientation had its state reset. This value is interpreted according to
 * the normal HID rules for physical values. However, the physical value and units
 * don't matter. The only information relevant to the host is a changed value.
 * To avoid numeric issues realted to loss of precision while converting from
 * logical to physical units, it's recommended to set the values for physical
 * min, physical max, and unit exponent to zero for this field.
 *
 * Report structure
 *
 * The grouping of peroperties into reports (by assignment of report IDs) is
 * flexible. For efficiency, we recommended separating the read-only properties
 * from the read/write properties. For the data fields, the Custom Value 1, 2,
 * and 3 fields must be in the same report and be in only one report for a
 * given device (app collection).
 *
 * The device must periodically and asynchronously (through HID INPUT messages)
 * send input reports when all the conditions below are met:
 *      . The Power State property is set to Full Power;
 *      . The Reporting State property is set to All Events;
 *      . The Reporting Interval property is non-zero;
 *
 */

#define HID_USAGE_PAGE_SENSOR                   0x20
#define HID_SENSOR_USAGE_ID_OTHER_CUSTOM        0xE1
#define HID_SENSOR_USAGE_ID_DESCRIPTION         0x08,0x03 // 0x0308 read-only
#define HID_SENSOR_USAGE_ID_UNIQUE_ID           0x02,0x03 // 0x0302 read-only
#define HID_SENSOR_USAGE_ID_REPORTING_STATE     0x16,0x03 // 0x0316 read/write
#define HID_SENSOR_REPORTING_STATE_NO_EVENTS    0x40,0x08 // 0x0840
#define HID_SENSOR_REPORTING_STATE_ALL_EVENTS   0x41,0x08 // 0x0841
#define HID_SENSOR_USAGE_ID_POWER_STATE         0x19,0x03 // 0x0319 read/write
#define HID_SENSOR_POWER_STATE_FULL_POWER       0x51,0x08 // 0x0851
#define HID_SENSOR_POWER_STATE_POWER_OFF        0x55,0x08 // 0x0855
#define HID_SENSOR_USAGE_ID_REPORT_INTERVAL     0x0E,0x03 // 0x030E read/write
#define HID_SENSOR_USAGE_ID_UNITS_SECOND        0x01,0x10 // 0x1001
#define HID_SENSOR_USAGE_ID_CUSTOM_DATA_1       0x44,0x05 // 0x0544 input report
#define HID_SENSOR_USAGE_ID_CUSTOM_DATA_2       0x45,0x05 // 0x0545 input report
#define HID_SENSOR_USAGE_ID_CUSTOM_DATA_3       0x46,0x05 // 0x0546 input report

#define HID_SENSOR_REPORTING_STATE_MASK     0x01
#define HID_SENSOR_POWER_STATE_MASK         0x02
#define HID_SENSOR_REPORT_INTERVAL_MASK     0xFC

#define HID_SENSOR_DESCR_FEATURE_REPORT_ID  0x05 // read-only
#define HID_SENSOR_STATE_FEATURE_REPORT_ID  0x06 // read/write
#define HID_SENSOR_VALUE_INPUT_REPORT_ID    0x07

#define HID_SENSOR_DESCR_FEATURE_DATA_LEN   (23+16)
#define HID_SENSOR_STATE_FEATURE_DATA_LEN   1
#define HID_SENSOR_VALUE_INPUT_DATA_LEN sizeof(struct bt_hid_sensor_report_t)

#define HID_HEAD_TRACKER_INPUT_REPORT_CONFIG \
    {1, { \
    {HID_SENSOR_VALUE_INPUT_REPORT_ID, HID_SENSOR_VALUE_INPUT_DATA_LEN}, \
    }}

#define HID_HEAD_TRACKER_OUTPUT_REPORT_CONFIG \
    {0, {{0}}}

#define HID_HEAD_TRACKER_FEATURE_REPORT_CONFIG \
    {2, { \
    {HID_SENSOR_DESCR_FEATURE_REPORT_ID, HID_SENSOR_DESCR_FEATURE_DATA_LEN}, \
    {HID_SENSOR_STATE_FEATURE_REPORT_ID, HID_SENSOR_STATE_FEATURE_DATA_LEN}, \
    }}

#define HID_REPORT_MODE_SENSOR_DESCRIPTOR_LEN 172
#define HID_REPORT_MODE_SENSOR_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_SENSOR), \
    HID_LOCAL_ITEM_USAGE_ID(HID_SENSOR_USAGE_ID_OTHER_CUSTOM), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_SENSOR_DESCR_FEATURE_REPORT_ID), /* read only report id */ \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_DESCRIPTION), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0xFF), \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(23), \
            HID_MAIN_ITEM_FEATURE(HID_CNST|HID_VAR|HID_ABS), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_UNIQUE_ID), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0xFF), \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(16), \
            HID_MAIN_ITEM_FEATURE(HID_CNST|HID_VAR|HID_ABS), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_SENSOR_STATE_FEATURE_REPORT_ID), /* read/write report id */ \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_REPORTING_STATE), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), /* 1-bit on/off reporting state */ \
            HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_LOGICAL), \
                HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_REPORTING_STATE_NO_EVENTS), \
                HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_REPORTING_STATE_ALL_EVENTS), \
                HID_MAIN_ITEM_FEATURE(HID_DATA|HID_ARR|HID_ABS), \
            HID_MAIN_ITEM_END_COLLECTION(), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_POWER_STATE), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), /* 1-bit on/off power state */ \
            HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_LOGICAL), \
                HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_POWER_STATE_POWER_OFF), \
                HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_POWER_STATE_FULL_POWER), \
                HID_MAIN_ITEM_FEATURE(HID_DATA|HID_ARR|HID_ABS), \
            HID_MAIN_ITEM_END_COLLECTION(), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_REPORT_INTERVAL), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0x00), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0x3F), \
            HID_GLOBAL_ITEM_PHYSICAL_MIN(10), \
            HID_GLOBAL_ITEM_PHYSICAL_MAX(100), \
            HID_GLOBAL_ITEM_REPORT_SIZE(6), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), /* 6-bit reporting interval */ \
            HID_GLOBAL_ITEM_UNIT_2B(HID_SENSOR_USAGE_ID_UNITS_SECOND), \
            HID_GLOBAL_ITEM_UNIT_EXPONENT(0xD), /* 10^-3 */ \
            HID_MAIN_ITEM_FEATURE(HID_DATA|HID_VAR|HID_ABS), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_SENSOR_VALUE_INPUT_REPORT_ID), /* input report id */ \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_CUSTOM_DATA_1), \
            HID_GLOBAL_ITEM_LOGICAL_MIN_2B(0x01, 0x80), /* -32767 */ \
            HID_GLOBAL_ITEM_LOGICAL_MAX_2B(0xFF, 0x7F), /* 32767 */ \
            HID_GLOBAL_ITEM_PHYSICAL_MIN_4B(0x60, 0x4F, 0x46, 0xED), /* -314159265 */ \
            HID_GLOBAL_ITEM_PHYSICAL_MAX_4B(0xA1, 0xB0, 0xB9, 0x12), /* 314159265 */ \
            HID_GLOBAL_ITEM_UNIT_EXPONENT(0x08), /* 10^-8 */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(16), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), /* 3 uint16 values [-pi, pi] */ \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_CUSTOM_DATA_2), \
            HID_GLOBAL_ITEM_LOGICAL_MIN_2B(0x01, 0x80), /* -32767 */ \
            HID_GLOBAL_ITEM_LOGICAL_MAX_2B(0xFF, 0x7F), /* 32767 */ \
            HID_GLOBAL_ITEM_PHYSICAL_MIN(0xE0), /* -32 */ \
            HID_GLOBAL_ITEM_PHYSICAL_MAX(0x20), /* 32 */ \
            HID_GLOBAL_ITEM_UNIT_EXPONENT(0x00), /* 10^0 */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(16), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), /* 3 uint16 values [-32, 32] rad/sec */ \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_SENSOR_USAGE_ID_CUSTOM_DATA_3), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0x00), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0xFF), \
            HID_GLOBAL_ITEM_PHYSICAL_MIN(0x00), \
            HID_GLOBAL_ITEM_PHYSICAL_MAX(0x00), \
            HID_GLOBAL_ITEM_UNIT_EXPONENT(0x00), /* 10^0 */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), /* 1-byte reference frame reset counter */ \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS), \
    HID_MAIN_ITEM_END_COLLECTION()

/**
 * The HID Subclass 1 defines two descriptors for Boot Devices.
 * Devices may append additional data to these boot reports, but
 * the first 8-bytes of keyboard reports and the first 3 bytes of
 * mouse reports must conform to the format defined by the Boot
 * Report descriptor in order for the data to be correctly
 * interpreted by the BIOS.
 */

/** Protocol 1 (Keyboard) Report Descriptor for a boot interface
 *
 * Keyboard input report (8-byte):
 * . byte-0 Modifier keys
 * . byte-1 Reserved
 * . byte-2 Keycode 1
 * . byte-3 Keycode 2
 * . byte-4 Keycode 3
 * . byte-5 Keycode 4
 * . byte-6 Keycode 5
 * . byte-7 Keycode 6
 *
 * Keyboard output report (1-byte):
 * . bit-0 NUM LOCK
 * . bit-1 CAPS LOCK
 * . bit-2 SCROLL LOCK
 * . bit-3 COMPOSE
 * . bit-4 KANA
 * . bit-5 to 7: CONSTANT
 *
 * Synchronization between LED states and CAPS LOCK, NUM LOCK,
 * SCROLL LOCK, COMPOSE, and KANA events is maintained by the
 * HID Host and NOT the keyboard. If using the boot protocol
 * mode keyboard descriptor, LED states are set by sending a
 * 5-bit absolute report to the keyboard via a SET_REPOT(OUTPUT)
 * request.
 */

#define HID_KEYBOARD_MOD_KEY_L_CTRL     0x01
#define HID_KEYBOARD_MOD_KEY_L_SHIFT    0x02
#define HID_KEYBOARD_MOD_KEY_L_ALT      0x04
#define HID_KEYBOARD_MOD_KEY_L_WIN      0x08
#define HID_KEYBOARD_MOD_KEY_R_CTRL     0x10
#define HID_KEYBOARD_MOD_KEY_R_SHIFT    0x20
#define HID_KEYBOARD_MOD_KEY_R_ALT      0x40
#define HID_KEYBOARD_MOD_KEY_R_WIN      0x80

typedef struct {
    uint8_t modifier_keys;
    uint8_t reserved;
    uint8_t keycode[6];
} __attribute__ ((packed)) hid_keyboard_boot_input_report_t;

#define HID_KEYBOARD_LED_KEY_NUM_LOCK       0x01
#define HID_KEYBOARD_LED_KEY_CAPS_LOCK      0x02
#define HID_KEYBOARD_LED_KEY_SCROLL_LOCK    0x04
#define HID_KEYBOARD_LED_KEY_COMPOSE        0x08
#define HID_KEYBOARD_LED_KEY_KANA           0x10

typedef struct {
    uint8_t led_lock_keys;
} __attribute__ ((packed)) hid_keyboard_boot_output_report_t;

#define HID_KEYBOARD_CTRL_KEY_POWER      0x01
#define HID_KEYBOARD_CTRL_KEY_PLAY       0x02
#define HID_KEYBOARD_CTRL_KEY_PAUSE      0x04
#define HID_KEYBOARD_CTRL_KEY_STOP       0x08
#define HID_KEYBOARD_CTRL_KEY_EJECT      0x10
#define HID_KEYBOARD_CTRL_KEY_PLAY_PAUSE 0x20
#define HID_KEYBOARD_CTRL_KEY_VOLUME_INC 0x40
#define HID_KEYBOARD_CTRL_KEY_VOLUME_DEC 0x80

struct keyboard_control_key_t {
    uint8_t ctrl_key;
};

#define HID_BOOT_MODE_KEYBOARD_DESCRIPTOR_LEN 63
#define HID_BOOT_MODE_KEYBOARD_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_KEYBOARD), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(8), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_KEY_CODES), \
        HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_KEYCODE_L_CTRL), \
        HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_KEYCODE_R_WIN), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),          /* modifier byte */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_MAIN_ITEM_INPUT(HID_CNST),                          /* reserved byte */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(5), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_LEDS), \
        HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_NUM_LOCK), \
        HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_KANA), \
            HID_MAIN_ITEM_OUTPUT(HID_DATA|HID_VAR|HID_ABS),         /* LED report */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(3), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_MAIN_ITEM_OUTPUT(HID_CNST),                         /* LED report padding */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_GLOBAL_ITEM_REPORT_COUNT(6), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_KEY_CODES), \
        HID_LOCAL_ITEM_USAGE_MIN(0x00), \
        HID_LOCAL_ITEM_USAGE_MAX(0xff), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0x00), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(0xff), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_ARR|HID_ABS),          /* Keycode array 6-byte */ \
    HID_MAIN_ITEM_END_COLLECTION()

#define HID_REPORT_MODE_KEYBOARD_INPUT_REPORT_CONFIG \
    {1, { \
    {HID_MEDIAKEY_INPUT_REPORT_ID, sizeof(struct keyboard_control_key_t)}, \
    }}

#define HID_REPORT_MODE_KEYBOARD_OUTPUT_REPORT_CONFIG \
    {0, {{0}}}

#define HID_REPORT_MODE_KEYBOARD_FEATURE_REPORT_CONFIG \
    {0, {{0}}}

#define HID_REPORT_MODE_MEDIAKEY_DESCRIPTOR_LEN 49
#define HID_REPORT_MODE_MEDIAKEY_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_CONSUMER_CONTROL), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MEDIAKEY_INPUT_REPORT_ID), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POWER), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_PLAY), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_PAUSE), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_STOP), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_EJECT), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_PLAY_PAUSE), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_INC), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_DEC), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL),          /* control cmd byte */ \
    HID_MAIN_ITEM_END_COLLECTION()

#define HID_CONSUMER_CTRL_KEY_NULL       0x00
#define HID_CONSUMER_CTRL_KEY_PLAY       0x01
#define HID_CONSUMER_CTRL_KEY_CONFIG     0x02
#define HID_CONSUMER_CTRL_KEY_NEXT       0x04
#define HID_CONSUMER_CTRL_KEY_PREVIOUS   0x08
#define HID_CONSUMER_CTRL_KEY_VOLUME_DEC 0x10
#define HID_CONSUMER_CTRL_KEY_VOLUME_INC 0x20
#define HID_CONSUMER_CTRL_KEY_FORWARD    0x40
#define HID_CONSUMER_CTRL_KEY_BACKWARD   0x80

struct consumer_control_key_t {
    uint8_t ctrl_key;
};

#define HID_DESCRIPTOR_MOUSE_BOOT_MODE_DATA_LEN                           (52 + 66 + 63)
#define HID_DESCRIPTOR_MOUSE_BOOT_MODE_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_CONSUMER_CONTROL), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_KEYBOARD_INPUT_REPORT_ID), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_PLAY_PAUSE), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_CONSUMER_CONTROL_CONF), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_NEXT_TRACK), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_PREV_TRACK), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_DEC), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_INC), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_FORWARD), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_BACK), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
    HID_MAIN_ITEM_END_COLLECTION(), \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_KEYBOARD), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECLK_INPUT_REPORT_ID), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(8), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_ID_KEYPAD), \
        HID_LOCAL_ITEM_USAGE_MIN(0xe0), \
        HID_LOCAL_ITEM_USAGE_MAX(0xe7), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
        HID_MAIN_ITEM_INPUT(HID_VAR), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_MAIN_ITEM_INPUT(HID_CNST|HID_VAR), \
        HID_GLOBAL_ITEM_REPORT_COUNT(5), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_LEDS), \
        HID_LOCAL_ITEM_USAGE_MIN(0x01), \
        HID_LOCAL_ITEM_USAGE_MAX(0x05), \
        HID_MAIN_ITEM_OUTPUT(HID_VAR), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
        HID_GLOBAL_ITEM_REPORT_SIZE(3), \
        HID_MAIN_ITEM_OUTPUT(HID_CNST|HID_VAR), \
        HID_GLOBAL_ITEM_REPORT_COUNT(6), \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        0x26, 0xff, 0x00, \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_KEY_CODES), \
        HID_LOCAL_ITEM_USAGE_MIN(0x00), \
        HID_LOCAL_ITEM_USAGE_MAX(0xff), \
        HID_MAIN_ITEM_INPUT(HID_DATA), \
    HID_MAIN_ITEM_END_COLLECTION(), \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_MOUSE), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MEDIAKEY_INPUT_REPORT_ID), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POINTER), \
        HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_PHYSICAL), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_BUTTONS), \
            HID_LOCAL_ITEM_USAGE_MIN(0x01), \
            HID_LOCAL_ITEM_USAGE_MAX(0x08), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(8), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
                HID_MAIN_ITEM_INPUT(HID_VAR), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
            HID_LOCAL_ITEM_USAGE_ID(0x30), \
            HID_LOCAL_ITEM_USAGE_ID(0x31), \
            HID_LOCAL_ITEM_USAGE_ID(0x38), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0x81), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0x7f), \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER), \
            HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_SCROLL_AAA), \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0x81), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(0x7f), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_MAIN_ITEM_END_COLLECTION(), \
    HID_MAIN_ITEM_END_COLLECTION()

#if 1
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_LEN HID_REPORT_MODE_MEDIAKEY_DESCRIPTOR_LEN
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_DATA HID_REPORT_MODE_MEDIAKEY_DESCRIPTOR_DATA
#elif 0
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_LEN 65
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_KEYBOARD), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_KEYBOARD_INPUT_REPORT_ID), \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(8), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_KEY_CODES), \
        HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_KEYCODE_L_CTRL), \
        HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_KEYCODE_R_WIN), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),          /* modifier byte */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_MAIN_ITEM_INPUT(HID_CNST),                          /* reserved byte */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(1), \
        HID_GLOBAL_ITEM_REPORT_COUNT(5), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_LEDS), \
        HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_NUM_LOCK), \
        HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_KANA), \
            HID_MAIN_ITEM_OUTPUT(HID_DATA|HID_VAR|HID_ABS),         /* LED report */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(3), \
        HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_MAIN_ITEM_OUTPUT(HID_CNST),                         /* LED report padding */ \
        HID_GLOBAL_ITEM_REPORT_SIZE(8), \
        HID_GLOBAL_ITEM_REPORT_COUNT(6), \
        HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_KEY_CODES), \
        HID_LOCAL_ITEM_USAGE_MIN(0x00), \
        HID_LOCAL_ITEM_USAGE_MAX(0xff), \
        HID_GLOBAL_ITEM_LOGICAL_MIN(0x00), \
        HID_GLOBAL_ITEM_LOGICAL_MAX(0xff), \
            HID_MAIN_ITEM_INPUT(HID_DATA|HID_ARR|HID_ABS),          /* Keycode array 6-byte */ \
    HID_MAIN_ITEM_END_COLLECTION()
#elif 0
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_LEN 143
#define HID_REPORT_MODE_KEYBOARD_DESCRIPTOR_DATA \
    0x05,0x01,      // USAGE_PAGE(Generic Desktop)
    0x09,0x06,      // USAGE(Keyboard)
    0xa1,0x01,      // COLLECTION(Application)
    0x85,0x01,      //      REPORT_ID(1)
    0x75,0x01,      //      REPORT_SIZE(1)
    0x95,0x08,      //      REPORT_COUNT(8)
    0x05,0x07,      //      USAGE_PAGE(Key Codes)
    0x19,0xe0,      //      USAGE_MIN(L_CTRL)
    0x29,0xe7,      //      USAGE_MAX(R_WIN)
    0x15,0x00,      //      LOGICAL_MIN(0)
    0x25,0x01,      //      LOGICAL_MAX(1)
    0x81,0x02,      //          INPUT(Data,Var,Abs)
    0x95,0x01,      //      REPORT_COUNT(1)
    0x75,0x08,      //      REPORT_SIZE(8)
    0x81,0x03,      //          INPUT(Cnst,Var,Abs)
    0x95,0x05,      //      REPORT_COUNT(5)
    0x75,0x01,      //      REPORT_SIZE(1)
    0x05,0x08,      //      USAGE_PAGE(LEDS)
    0x19,0x01,      //      USAGE_MIN(NUM_LOCK)
    0x29,0x05,      //      USAGE_MAX(KANA)
    0x91,0x02,      //          OUTPUT(Data,Var,Abs)
    0x95,0x01,      //      REPORT_COUNT(1)
    0x75,0x03,      //      REPORT_SIZE(3)
    0x91,0x03,      //          OUTPUT(Cnst,Var,Abs)
    0x95,0x06,      //      REPORT_COUNT(6)
    0x75,0x08,      //      REPORT_SIZE(8)
    0x15,0x00,      //      LOGICAL_MIN(0x00)
    0x25,0xff,      //      LOGICAL_MAX(0xff)
    0x05,0x07,      //      USAGE_PAGE(Key Codes)
    0x19,0x00,      //      USAGE_MIN(0x00)
    0x29,0xff,      //      USAGE_MAX(0xff)
    0x81,0x00,      //          INPUT(Data,Arr,Abs)
    0xc0,           // END_COLLECTION()
    0x05,0x0c,      // USAGE_PAGE(CONSUMER)
    0x09,0x01,      // USAGE(CONSUMER_CONTROL)
    0xa1,0x01,      // COLLECTION(Application)
    0x85,0x02,      //      REPORT_ID(2)
    0x15,0x00,      //      LOGICAL_MIN(0)
    0x25,0x01,      //      LOGICAL_MAX(1)
    0x75,0x01,      //      REPORT_SIZE(1)
    0x95,0x15,      //      REPORT_COUNT(21)
    0x0a,0x94,0x01, //      USAGE(LOCAL_MACHINE_BROWSER)
    0x0a,0x92,0x01, //      USAGE(CALCULATOR)
    0x0a,0x83,0x01, //      USAGE(CONSUMER_CONTROL_CONF)
    0x0a,0x23,0x02, //      USAGE(HOME)
    0x0a,0x8a,0x01, //      USAGE(EMAIL_READER)
    0x0a,0xb1,0x01, //      USAGE(SCREEN_SAVER)
    0x0a,0x21,0x02, //      USAGE(SEARCH)
    0x0a,0x24,0x02, //      USAGE(BACK)
    0x0a,0x25,0x02, //      USAGE(FORWARD)
    0x0a,0x2a,0x02, //      USAGE(BOOKMARKS)
    0x0a,0x27,0x02, //      USAGE(REFRESH)
    0x09,0xb6,      //      USAGE(PREV_TRACK)
    0x09,0xb5,      //      USAGE(NEXT_TRACK)
    0x09,0xb7,      //      USAGE(STOP)
    0x09,0xb0,      //      USAGE(PLAY)
    0x09,0xe9,      //      USAGE(VOLUME_INC)
    0x09,0xea,      //      USAGE(VOLUME_DEC)
    0x09,0xe2,      //      USAGE(MUTE)
    0x09,0xcd,      //      USAGE(PLAY_PAUSE)
    0x09,0x30,      //      USAGE(POWER)
    0x09,0xb8,      //      USAGE(EJECT)
    0x81,0x02,      //          INPUT(Data,Var,Abs)
    0x95,0x01,      //      REPORT_COUNT(1)
    0x75,0x03,      //      REPORT_SIZE(3)
    0x81,0x03,      //          INPUT(Cnst,Var,Abs)
    0xc0,           // END_COLLECTION()
#else
    0x05,0x0c,
    0x09,0x01,
    0xA1,0x01,
    0x85,0x02,
    0x09,0xCD,    //usage playpause
    0x09,0xB7,    //usage stop
    0x09,0xB6,    //usage previous
    0x09,0xB5,    //usage next
    0x09,0xE9,    //usage vol+
    0x09,0xEA,    //usage vol-
    0x0A,0xB1,0x01, // usage screen saver
    0x09,0x30,    //usage power
    0x95,0x01,
    0x75,0x10,
    0x15,0x01,
    0x26,0x9C,0x02,
    0x19,0x01,
    0x2A,0x9C,0x02,
    0x81,0x00,
    0xC0,
    0x05,0x01,
    0x09,0x06,
    0xA1,0x01,
    0x85,0x03,
    0x05,0x07,
    0x19,0xE0,
    0x29,0xE7,
    0x15,0x00,
    0x25,0x01,
    0x75,0x01,
    0x95,0x08,
    0x81,0x02,
    0x75,0x08,
    0x95,0x01,
    0x15,0x00,
    0x25,0xF4,
    0x05,0x07,
    0x19,0x00,
    0x29,0xF4,
    0x81,0x00,
    0xC0
#endif

/** Protocl 2 (Mouse) Report Descriptor for a boot interface
 *
 * Mouse input report (3-byte):
 * . byte-0 bit-0 Button 1
 *          bit-1 Button 2
 *          bit-2 Button 3
 *          bit-3 to 7: device-specific
 * . byte-1     X displacement
 * . byte-2     Y displacement
 * . byte-3...  device-specific (optional)
 *
 */

#define HID_MOUSE_BUTTON_1 0x01
#define HID_MOUSE_BUTTON_2 0x02
#define HID_MOUSE_BUTTON_3 0x04

typedef struct {
    uint8_t button_bits;
    int8_t x_position;
    int8_t y_position;
} __attribute__ ((packed)) hid_mouse_boot_input_report_t;

struct mouse_input_report_t {
    uint8_t report_id;
    uint8_t button_bits;
    int8_t x_position;
    int8_t y_position;
};

struct mouse_control_report_t {
    uint8_t report_id;
    uint8_t control_bits;
};

typedef struct {
    uint8_t control_bits;
} __attribute__ ((packed)) hid_mousectl_report_t;

#define HID_BOOT_MODE_MOUSE_DESCRIPTOR_LEN 50
#define HID_BOOT_MODE_MOUSE_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_MOUSE), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POINTER), \
        HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_PHYSICAL), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_BUTTONS), \
            HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_BUTTON_1), \
            HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_BUTTON_3), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),      /* button report */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(5), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
                HID_MAIN_ITEM_INPUT(HID_CNST),                      /* button report padding */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(2), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_X), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_Y), \
            HID_GLOBAL_ITEM_LOGICAL_MIN((uint8_t)(int8_t)(-127)), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(127), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_MAIN_ITEM_END_COLLECTION(), \
    HID_MAIN_ITEM_END_COLLECTION()

#define HID_REPORT_MODE_MOUSE_INPUT_REPORT_CONFIG \
    {2, { \
    {HID_MOUSECLK_INPUT_REPORT_ID, sizeof(hid_mouse_boot_input_report_t)}, \
    {HID_MOUSECTL_INPUT_REPORT_ID, sizeof(hid_mousectl_report_t)}, \
    }}

#define HID_REPORT_MODE_MOUSE_OUTPUT_REPORT_CONFIG \
    {0, {{0}}}

#define HID_REPORT_MODE_MOUSE_FEATURE_REPORT_CONFIG \
    {0, {{0}}}


#if 1
#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_LEN (52+26)
#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_MOUSE), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECLK_INPUT_REPORT_ID), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POINTER), \
        HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_PHYSICAL), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_BUTTONS), \
            HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_BUTTON_1), \
            HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_BUTTON_3), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),      /* button report */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(5), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
                HID_MAIN_ITEM_INPUT(HID_CNST),                      /* button report padding */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(2), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_X), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_Y), \
            HID_GLOBAL_ITEM_LOGICAL_MIN((uint8_t)(int8_t)(-127)), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(127), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_MAIN_ITEM_END_COLLECTION(), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECTL_INPUT_REPORT_ID), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_INC), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_DEC), \
            HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_BACK), \
            HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_HOME), \
            HID_GLOBAL_ITEM_REPORT_COUNT(4), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_MAIN_ITEM_INPUT(HID_VAR), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_GLOBAL_ITEM_REPORT_SIZE(4), \
            HID_MAIN_ITEM_INPUT(HID_CNST), \
        HID_MAIN_ITEM_END_COLLECTION()
#else
#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_LEN 52
#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_MOUSE), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECLK_INPUT_REPORT_ID), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POINTER), \
        HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_PHYSICAL), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_BUTTONS), \
            HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_BUTTON_1), \
            HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_BUTTON_3), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),      /* button report */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(5), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
                HID_MAIN_ITEM_INPUT(HID_CNST),                      /* button report padding */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(2), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_X), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_Y), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(-127), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(127), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_MAIN_ITEM_END_COLLECTION(), \
    HID_MAIN_ITEM_END_COLLECTION()

/**
     *  --------------------------------------------------------------------------
     *  Bit      |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
     *  --------------------------------------------------------------------------
     *  Byte 0   |               Not Used                | Middle| Right | Left  |
     *  --------------------------------------------------------------------------
     *  Byte 1   |                     X Axis Relative Movement                  |
     *  --------------------------------------------------------------------------
     *  Byte 2   |                     Y Axis Relative Movement                  |
     *  --------------------------------------------------------------------------
     *  Byte 3   |                     Wheel Relative Movement                   |
     *  --------------------------------------------------------------------------
     */

    0x05, 0x01,     /// USAGE PAGE (Generic Desktop)
    0x09, 0x02,     /// USAGE (Mouse)
    0xA1, 0x01,     /// COLLECTION (Application)
    0x85, 0x01,     /// REPORT ID (1) - MANDATORY
    0x09, 0x01,     ///     USAGE (Pointer)
    0xA1, 0x00,     ///     COLLECTION (Physical)

    /**
     * ----------------------------------------------------------------------------
     * BUTTONS
     * ----------------------------------------------------------------------------
     */
    0x05, 0x09,     ///         USAGE PAGE (Buttons)
    0x19, 0x01,     ///         USAGE MINIMUM (1)
    0x29, 0x08,     ///         USAGE MAXIMUM (8)
    0x15, 0x00,     ///         LOGICAL MINIMUM (0)
    0x25, 0x01,     ///         LOGICAL MAXIMUM (1)
    0x75, 0x01,     ///         REPORT SIZE (1)
    0x95, 0x08,     ///         REPORT COUNT (8)
    0x81, 0x02,     ///         INPUT (Data, Variable, Absolute)

    /**
     * ----------------------------------------------------------------------------
     * MOVEMENT DATA
     * ----------------------------------------------------------------------------
     */
    0x05, 0x01,     ///         USAGE PAGE (Generic Desktop)
    0x16, 0x08, 0xFF, ///       LOGICAL MINIMUM (-255)
    0x26, 0xFF, 0x00, ///       LOGICAL MAXIMUM (255)
    0x75, 0x10,     ///         REPORT SIZE (16)
    0x95, 0x02,     ///         REPORT COUNT (2)
    0x09, 0x30,     ///         USAGE (X)
    0x09, 0x31,     ///         USAGE (Y)
    0x81, 0x06,     ///         INPUT (Data, Variable, Relative)

    0x15, 0x81,     ///         LOGICAL MINIMUM (-127)
    0x25, 0x7F,     ///         LOGICAL MAXIMUM (127)
    0x75, 0x08,     ///         REPORT SIZE (8)
    0x95, 0x01,     ///         REPORT COUNT (1)
    0x09, 0x38,     ///         USAGE (Wheel)
    0x81, 0x06,     ///         INPUT (Data, Variable, Relative)

    0xC0,           ///     END COLLECTION (Physical)
    0xC0            /// END COLLECTION (Application)
#endif

#ifdef BT_HID_DEVICE

/**
 * standard hid (device role) interface
 *
 */

#define HID_CTRL_KEY_NULL       0x00
#define HID_CTRL_KEY_POWER      0x01
#define HID_CTRL_KEY_PLAY       0x02
#define HID_CTRL_KEY_PAUSE      0x04
#define HID_CTRL_KEY_STOP       0x08
#define HID_CTRL_KEY_EJECT      0x10
#define HID_CTRL_KEY_PLAY_PAUSE 0x20
#define HID_CTRL_KEY_VOLUME_INC 0x40
#define HID_CTRL_KEY_VOLUME_DEC 0x80

#define HID_MOD_KEY_NULL    0x00
#define HID_MOD_KEY_L_CTRL  0x01
#define HID_MOD_KEY_L_SHIFT 0x02
#define HID_MOD_KEY_L_ALT   0x04
#define HID_MOD_KEY_L_WIN   0x08
#define HID_MOD_KEY_R_CTRL  0x10
#define HID_MOD_KEY_R_SHIFT 0x20
#define HID_MOD_KEY_R_ALT   0x40
#define HID_MOD_KEY_R_WIN   0x80

#define HID_KEY_CODE_NULL   0x00
#define HID_KEY_CODE_A      0x04
#define HID_KEY_CODE_Z      0x1d
#define HID_KEY_CODE_1      0x1e
#define HID_KEY_CODE_9      0x26
#define HID_KEY_CODE_0      0x27
#define HID_KEY_CODE_ENTER  0x28
#define HID_KEY_CODE_ESC    0x29
#define HID_KEY_CODE_DEL    0x2a
#define HID_KEY_CODE_TAB    0x2b
#define HID_KEY_CODE_SPACE  0x2c
#define HID_KEY_CODE_VOLUP  0x80
#define HID_KEY_CODE_VOLDN  0x81

#define HID_MOUSE_BUTTON_1 0x01
#define HID_MOUSE_BUTTON_2 0x02
#define HID_MOUSE_BUTTON_3 0x04

#define HID_MOUSE_CTL_VOL_IND 0x01
#define HID_MOUSE_CTL_VOL_DEC 0x02
#define HID_MOUSE_CTL_BACK    0x04
#define HID_MOUSE_CTL_HOME    0x08

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
} bt_hid_opened_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
} bt_hid_closed_t;

typedef enum {
    HID_INTERRUPT_CHANNEL = 0,
    HID_CONTROL_CHANNEL,
} bt_hid_txchannel_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
} bt_hid_txdone_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
    uint8_t sensor_state;
} bt_hid_sensor_state_changed_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
    uint8_t len;
    uint8_t* data;
} bt_hid_ctl_data_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
    uint8_t len;
    uint8_t* data;
} bt_hid_int_data_t;

typedef struct {
    void *channel;
    uint8_t error_code;
    uint8_t device_id;
    uint16_t hid_version;
    uint16_t parser_version;
    uint8_t device_subclass;
    uint8_t contry_code;
    bool virtual_cable;
    bool reconnect_initiate;
    bool boot_device;
    uint8_t decrp_count;
    hid_descriptorlist_t decrp[MAX_DESCRIPTOR_COUNT];
} bt_hid_sdp_rec_t;

typedef union {
    void *param_ptr;
    bt_hid_opened_t *opened;
    bt_hid_closed_t *closed;
    bt_hid_txdone_t *txdone;
    bt_hid_sensor_state_changed_t *changed;
    bt_hid_ctl_data_t *ctl_data;
    bt_hid_int_data_t *int_data;
    bt_hid_sdp_rec_t *sdp_data;
} bt_hid_callback_param_t;

typedef enum {
    BT_HID_EVENT_OPENED = BT_EVENT_HID_OPENED,
    BT_HID_EVENT_CLOSED,
    BT_HID_EVENT_SENSOR_STATE_CHANGED,
    BT_HID_EVENT_TXDONE,
    BT_HID_EVENT_RCV_CTL_DATA,
    BT_HID_EVENT_RCV_INT_DATA,
    BT_HID_EVENT_RCV_SDP_DATA,
    BT_HID_EVENT_END,
} bt_hid_event_t;

typedef struct {
    uint8_t device_subclass;
    uint8_t descriptor_list_len;
    const uint8_t *descriptor_list;
} bt_hid_custom_descriptor_t;

#if BT_HID_EVENT_END != BT_EVENT_HID_END
#error "bt_hid_event_t error define"
#endif

typedef int (*bt_hid_callback_t)(const bt_bdaddr_t *bd_addr, bt_hid_event_t event, bt_hid_callback_param_t param);

bt_status_t bt_hid_init(bt_hid_callback_t callback);
bt_status_t bt_hid_cleanup(void);
bt_status_t bt_hid_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hid_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hid_virtual_cable_unplug(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hid_capture(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hid_send_sensor_report(const bt_bdaddr_t *bd_addr, const struct bt_hid_sensor_report_t *report);
bt_status_t bt_hid_send_keyboard_report(const bt_bdaddr_t *remote, uint8_t modifier_key, uint8_t key_code);
bt_status_t bt_hid_send_mouse_report(const bt_bdaddr_t *remote, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons);
bt_status_t bt_hid_send_mouse_control_report(const bt_bdaddr_t *remote, uint8_t ctl_buttons);

#endif /* BT_HID_DEVICE */

#ifdef __cplusplus
}
#endif
#endif /* __BT_HID_SERVICE_H__ */

