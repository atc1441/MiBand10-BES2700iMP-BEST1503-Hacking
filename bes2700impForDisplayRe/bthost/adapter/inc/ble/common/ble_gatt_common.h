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
#ifndef __BLE_GATT_COMMON_H__
#define __BLE_GATT_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

/// Macro used to convert CPU integer define to LSB first 16-bits UUID
#if (!CPU_LE)
#define GATT_UUID_16_LSB(uuid)          (((uuid & 0xFF00) >> 8) | ((uuid & 0x00FF) << 8))
#define UUID_16_TO_ARRAY(uuid)          {(uuid & 0xFF00) >> 8, (uuid & 0x00FF)}
#else
#define GATT_UUID_16_LSB(uuid)          (uuid)
#define UUID_16_TO_ARRAY(uuid)          {(uuid & 0x00FF), (uuid & 0xFF00) >> 8}
#endif // CPU_LE

/// Length of an handle
#define GATT_HANDLE_LEN      (2)
/// Length of 16-bit UUID in octets
#define GATT_UUID_16_LEN     (2)
/// Length of 32-bit UUID in octets
#define GATT_UUID_32_LEN     (4)
/// Length of 128-bit UUID in octets
#define GATT_UUID_128_LEN    (16)
/// Length of Database Hash in octets
#define GATT_DB_HASH_LEN     (16)

/// Common 16-bit Universal Unique Identifier
enum gatt_char
{
    GATT_INVALID_UUID                            = GATT_UUID_16_LSB(0x0000), //!< Value: 0x0000

    /*----------------- SERVICES ---------------------*/
    /// Generic Access Profile
    GATT_SVC_GENERIC_ACCESS                      = GATT_UUID_16_LSB(0x1800), //!< Value: 0x1800
    /// Attribute Profile
    GATT_SVC_GENERIC_ATTRIBUTE                   = GATT_UUID_16_LSB(0x1801), //!< Value: 0x1801
    /// Immediate alert Service
    GATT_SVC_IMMEDIATE_ALERT                     = GATT_UUID_16_LSB(0x1802), //!< Value: 0x1802
    /// Link Loss Service
    GATT_SVC_LINK_LOSS                           = GATT_UUID_16_LSB(0x1803), //!< Value: 0x1803
    /// Tx Power Service
    GATT_SVC_TX_POWER                            = GATT_UUID_16_LSB(0x1804), //!< Value: 0x1804
    /// Current Time Service Service
    GATT_SVC_CURRENT_TIME                        = GATT_UUID_16_LSB(0x1805), //!< Value: 0x1805
    /// Reference Time Update Service
    GATT_SVC_REF_TIME_UPDATE                     = GATT_UUID_16_LSB(0x1806), //!< Value: 0x1806
    /// Next DST Change Service
    GATT_SVC_NEXT_DST_CHANGE                     = GATT_UUID_16_LSB(0x1807), //!< Value: 0x1807
    /// Glucose Service
    GATT_SVC_GLUCOSE                             = GATT_UUID_16_LSB(0x1808), //!< Value: 0x1808
    /// Health Thermometer Service
    GATT_SVC_HEALTH_THERMOM                      = GATT_UUID_16_LSB(0x1809), //!< Value: 0x1809
    /// Device Information Service
    GATT_SVC_DEVICE_INFO                         = GATT_UUID_16_LSB(0x180A), //!< Value: 0x180A
    /// Heart Rate Service
    GATT_SVC_HEART_RATE                          = GATT_UUID_16_LSB(0x180D), //!< Value: 0x180D
    /// Phone Alert Status Service
    GATT_SVC_PHONE_ALERT_STATUS                  = GATT_UUID_16_LSB(0x180E), //!< Value: 0x180E
    /// Battery Service
    GATT_SVC_BATTERY_SERVICE                     = GATT_UUID_16_LSB(0x180F), //!< Value: 0x180F
    /// Blood Pressure Service
    GATT_SVC_BLOOD_PRESSURE                      = GATT_UUID_16_LSB(0x1810), //!< Value: 0x1810
    /// Alert Notification Service
    GATT_SVC_ALERT_NTF                           = GATT_UUID_16_LSB(0x1811), //!< Value: 0x1811
    /// HID Service
    GATT_SVC_HID                                 = GATT_UUID_16_LSB(0x1812), //!< Value: 0x1812
    /// Scan Parameters Service
    GATT_SVC_SCAN_PARAMETERS                     = GATT_UUID_16_LSB(0x1813), //!< Value: 0x1813
    /// Running Speed and Cadence Service
    GATT_SVC_RUNNING_SPEED_CADENCE               = GATT_UUID_16_LSB(0x1814), //!< Value: 0x1814
    /// Cycling Speed and Cadence Service
    GATT_SVC_CYCLING_SPEED_CADENCE               = GATT_UUID_16_LSB(0x1816), //!< Value: 0x1816
    /// Cycling Power Service
    GATT_SVC_CYCLING_POWER                       = GATT_UUID_16_LSB(0x1818), //!< Value: 0x1818
    /// Location and Navigation Service
    GATT_SVC_LOCATION_AND_NAVIGATION             = GATT_UUID_16_LSB(0x1819), //!< Value: 0x1819
    /// Environmental Sensing Service
    GATT_SVC_ENVIRONMENTAL_SENSING               = GATT_UUID_16_LSB(0x181A), //!< Value: 0x181A
    /// Body Composition Service
    GATT_SVC_BODY_COMPOSITION                    = GATT_UUID_16_LSB(0x181B), //!< Value: 0x181B
    /// User Data Service
    GATT_SVC_USER_DATA                           = GATT_UUID_16_LSB(0x181C), //!< Value: 0x181C
    /// Weight Scale Service
    GATT_SVC_WEIGHT_SCALE                        = GATT_UUID_16_LSB(0x181D), //!< Value: 0x181D
    /// Bond Management Service
    GATT_SVC_BOND_MANAGEMENT                     = GATT_UUID_16_LSB(0x181E), //!< Value: 0x181E
    /// Continuous Glucose Monitoring Service
    GATT_SVC_CONTINUOUS_GLUCOSE_MONITORING       = GATT_UUID_16_LSB(0x181F), //!< Value: 0x181F
    /// Internet Protocol Support Service
    GATT_SVC_IP_SUPPORT                          = GATT_UUID_16_LSB(0x1820), //!< Value: 0x1820
    /// Indoor Positioning Service
    GATT_SVC_INDOOR_POSITIONING                  = GATT_UUID_16_LSB(0x1821), //!< Value: 0x1821
    /// Pulse Oximeter Service
    GATT_SVC_PULSE_OXIMETER                      = GATT_UUID_16_LSB(0x1822), //!< Value: 0x1822
    /// HTTP Proxy Service
    GATT_SVC_HTTP_PROXY                          = GATT_UUID_16_LSB(0x1823), //!< Value: 0x1823
    /// Transport Discovery Service
    GATT_SVC_TRANSPORT_DISCOVERY                 = GATT_UUID_16_LSB(0x1824), //!< Value: 0x1824
    /// Object Transfer Service
    GATT_SVC_OBJECT_TRANSFER                     = GATT_UUID_16_LSB(0x1825), //!< Value: 0x1825

    /// Mesh Provisioning Service
    GATT_SVC_MESH_PROVISIONING                   = GATT_UUID_16_LSB(0x1827), //!< Value: 0x1827
    /// Mesh Proxy Service
    GATT_SVC_MESH_PROXY                          = GATT_UUID_16_LSB(0x1828), //!< Value: 0x1828

    /// Audio Input Control Service
    GATT_SVC_AUDIO_INPUT_CONTROL                 = GATT_UUID_16_LSB(0x1843), //!< Value: 0x1843
    /// Volume Control Service
    GATT_SVC_VOLUME_CONTROL                      = GATT_UUID_16_LSB(0x1844), //!< Value: 0x1844
    /// Volume Offset Control Service
    GATT_SVC_VOLUME_OFFSET_CONTROL               = GATT_UUID_16_LSB(0x1845), //!< Value: 0x1845
    /// Microphone Control Service
    GATT_SVC_MICROPHONE_CONTROL                  = GATT_UUID_16_LSB(0x184D), //!< Value: 0x8FD4
    /// Telephone Bearer Service
    GATT_SVC_TELEPHONE_BEARER                    = GATT_UUID_16_LSB(0x184B), //!< Value: 0x184B
    /// Generic Telephone Bearer Service
    GATT_SVC_GENERIC_TELEPHONE_BEARER            = GATT_UUID_16_LSB(0x184C), //!< Value: 0x184C
    /// Media Control Service
    GATT_SVC_MEDIA_CONTROL                       = GATT_UUID_16_LSB(0x1848), //!< Value: 0x1848
    /// Generic Media Control Service
    GATT_SVC_GENERIC_MEDIA_CONTROL               = GATT_UUID_16_LSB(0x1849), //!< Value: 0x1849
    /// Common Audio Service
    GATT_SVC_COMMON_AUDIO                        = GATT_UUID_16_LSB(0x1853), //!< Value: 0x1853
     /// Hearing Access Service
    GATT_SVC_HEARING_AID                         = GATT_UUID_16_LSB(0x1854), //!< Value: 0x1854
    /// Telephony and Media Audio Service
    GATT_SVC_TELEPHONY_MEDIA_AUDIO               = GATT_UUID_16_LSB(0x1855), //!< Value: 0x1855
    /// Public Broadcast Announcement Service
    GATT_SVC_PUBLIC_BRODCAST_AUDIO               = GATT_UUID_16_LSB(0x1856), //!< Value: 0x1856
    /// Gaming Audio Service
    GATT_SVC_GAMING_AUDIO                        = GATT_UUID_16_LSB(0x1858), //!< Value: 0x1858
    /// Advanced Headphone Service (AHS) Service
    GATT_SVC_ADVANCED_HEADPHONE                  = GATT_UUID_16_LSB(0x7FA0), //!< Value: 0x7FA0

    /*------------------- UNITS ---------------------*/
    /// No defined unit
    GATT_UNIT_UNITLESS                           = GATT_UUID_16_LSB(0x2700), //!< Value: 0x2700
    /// Length Unit - Metre
    GATT_UNIT_METRE                              = GATT_UUID_16_LSB(0x2701), //!< Value: 0x2701
    ///Mass unit - Kilogram
    GATT_UNIT_KG                                 = GATT_UUID_16_LSB(0x2702), //!< Value: 0x2702
    ///Time unit - second
    GATT_UNIT_SECOND                             = GATT_UUID_16_LSB(0x2703), //!< Value: 0x2703
    ///Electric current unit - Ampere
    GATT_UNIT_AMPERE                             = GATT_UUID_16_LSB(0x2704), //!< Value: 0x2704
    ///Thermodynamic Temperature unit - Kelvin
    GATT_UNIT_KELVIN                             = GATT_UUID_16_LSB(0x2705), //!< Value: 0x2705
    /// Amount of substance unit - mole
    GATT_UNIT_MOLE                               = GATT_UUID_16_LSB(0x2706), //!< Value: 0x2706
    ///Luminous intensity unit - candela
    GATT_UNIT_CANDELA                            = GATT_UUID_16_LSB(0x2707), //!< Value: 0x2707
    ///Area unit - square metres
    GATT_UNIT_SQ_METRE                           = GATT_UUID_16_LSB(0x2710), //!< Value: 0x2710
    ///Colume unit - cubic metres
    GATT_UNIT_CUBIC_METRE                        = GATT_UUID_16_LSB(0x2710), //!< Value: 0x2710
    ///Velocity unit - metres per second
    GATT_UNIT_METRE_PER_SECOND                   = GATT_UUID_16_LSB(0x2711), //!< Value: 0x2711
    ///Acceleration unit - metres per second squared
    GATT_UNIT_METRES_PER_SEC_SQ                  = GATT_UUID_16_LSB(0x2712), //!< Value: 0x2712
    ///Wavenumber unit - reciprocal metre
    GATT_UNIT_RECIPROCAL_METRE                   = GATT_UUID_16_LSB(0x2713), //!< Value: 0x2713
    ///Density unit - kilogram per cubic metre
    GATT_UNIT_DENS_KG_PER_CUBIC_METRE            = GATT_UUID_16_LSB(0x2714), //!< Value: 0x2714
    ///Surface density unit - kilogram per square metre
    GATT_UNIT_KG_PER_SQ_METRE                    = GATT_UUID_16_LSB(0x2715), //!< Value: 0x2715
    ///Specific volume unit - cubic metre per kilogram
    GATT_UNIT_CUBIC_METRE_PER_KG                 = GATT_UUID_16_LSB(0x2716), //!< Value: 0x2716
    ///Current density unit - ampere per square metre
    GATT_UNIT_AMPERE_PER_SQ_METRE                = GATT_UUID_16_LSB(0x2717), //!< Value: 0x2717
    ///Magnetic field strength unit - Ampere per metre
    GATT_UNIT_AMPERE_PER_METRE                   = GATT_UUID_16_LSB(0x2718), //!< Value: 0x2718
    ///Amount concentration unit - mole per cubic metre
    GATT_UNIT_MOLE_PER_CUBIC_METRE               = GATT_UUID_16_LSB(0x2719), //!< Value: 0x2719
    ///Mass Concentration unit - kilogram per cubic metre
    GATT_UNIT_MASS_KG_PER_CUBIC_METRE            = GATT_UUID_16_LSB(0x271A), //!< Value: 0x271A
    ///Luminance unit - candela per square metre
    GATT_UNIT_CANDELA_PER_SQ_METRE               = GATT_UUID_16_LSB(0x271B), //!< Value: 0x271B
    ///Refractive index unit
    GATT_UNIT_REFRACTIVE_INDEX                   = GATT_UUID_16_LSB(0x271C), //!< Value: 0x271C
    ///Relative permeability unit
    GATT_UNIT_RELATIVE_PERMEABILITY              = GATT_UUID_16_LSB(0x271D), //!< Value: 0x271D
    ///Plane angle unit - radian
    GATT_UNIT_RADIAN                             = GATT_UUID_16_LSB(0x2720), //!< Value: 0x2720
    ///Solid angle unit - steradian
    GATT_UNIT_STERADIAN                          = GATT_UUID_16_LSB(0x2721), //!< Value: 0x2721
    ///Frequency unit - Hertz
    GATT_UNIT_HERTZ                              = GATT_UUID_16_LSB(0x2722), //!< Value: 0x2722
    ///Force unit - Newton
    GATT_UNIT_NEWTON                             = GATT_UUID_16_LSB(0x2723), //!< Value: 0x2723
    ///Pressure unit - Pascal
    GATT_UNIT_PASCAL                             = GATT_UUID_16_LSB(0x2724), //!< Value: 0x2724
    ///Energy unit - Joule
    GATT_UNIT_JOULE                              = GATT_UUID_16_LSB(0x2725), //!< Value: 0x2725
    ///Power unit - Watt
    GATT_UNIT_WATT                               = GATT_UUID_16_LSB(0x2726), //!< Value: 0x2726
    ///electric Charge unit - Coulomb
    GATT_UNIT_COULOMB                            = GATT_UUID_16_LSB(0x2727), //!< Value: 0x2727
    ///Electric potential difference - Volt
    GATT_UNIT_VOLT                               = GATT_UUID_16_LSB(0x2728), //!< Value: 0x2728
    ///Capacitance unit - Farad
    GATT_UNIT_FARAD                              = GATT_UUID_16_LSB(0x2729), //!< Value: 0x2729
    ///electric resistance unit - Ohm
    GATT_UNIT_OHM                                = GATT_UUID_16_LSB(0x272A), //!< Value: 0x272A
    ///Electric conductance - Siemens
    GATT_UNIT_SIEMENS                            = GATT_UUID_16_LSB(0x272B), //!< Value: 0x272B
    ///Magnetic flux unit - Weber
    GATT_UNIT_WEBER                              = GATT_UUID_16_LSB(0x272C), //!< Value: 0x272C
    ///Magnetic flux density unit - Tesla
    GATT_UNIT_TESLA                              = GATT_UUID_16_LSB(0x272D), //!< Value: 0x272D
    ///Inductance unit - Henry
    GATT_UNIT_HENRY                              = GATT_UUID_16_LSB(0x272E), //!< Value: 0x272E
    ///Temperature unit - degree Celsius
    GATT_UNIT_CELSIUS                            = GATT_UUID_16_LSB(0x272F), //!< Value: 0x272F
    ///Luminous flux unit - lumen
    GATT_UNIT_LUMEN                              = GATT_UUID_16_LSB(0x2730), //!< Value: 0x2730
    ///Illuminance unit - lux
    GATT_UNIT_LUX                                = GATT_UUID_16_LSB(0x2731), //!< Value: 0x2731
    ///Activity referred to a radionuclide unit - becquerel
    GATT_UNIT_BECQUEREL                          = GATT_UUID_16_LSB(0x2732), //!< Value: 0x2732
    ///Absorbed dose unit - Gray
    GATT_UNIT_GRAY                               = GATT_UUID_16_LSB(0x2733), //!< Value: 0x2733
    ///Dose equivalent unit - Sievert
    GATT_UNIT_SIEVERT                            = GATT_UUID_16_LSB(0x2734), //!< Value: 0x2734
    ///Catalytic activity unit - Katal
    GATT_UNIT_KATAL                              = GATT_UUID_16_LSB(0x2735), //!< Value: 0x2735
    ///Synamic viscosity unit - Pascal second
    GATT_UNIT_PASCAL_SECOND                      = GATT_UUID_16_LSB(0x2740), //!< Value: 0x2740
    ///Moment of force unit - Newton metre
    GATT_UNIT_NEWTON_METRE                       = GATT_UUID_16_LSB(0x2741), //!< Value: 0x2741
    ///surface tension unit - Newton per metre
    GATT_UNIT_NEWTON_PER_METRE                   = GATT_UUID_16_LSB(0x2742), //!< Value: 0x2742
    ///Angular velocity unit - radian per second
    GATT_UNIT_RADIAN_PER_SECOND                  = GATT_UUID_16_LSB(0x2743), //!< Value: 0x2743
    ///Angular acceleration unit - radian per second squared
    GATT_UNIT_RADIAN_PER_SECOND_SQ               = GATT_UUID_16_LSB(0x2744), //!< Value: 0x2744
    ///Heat flux density unit - Watt per square metre
    GATT_UNIT_WGATT_PER_SQ_METRE                  = GATT_UUID_16_LSB(0x2745), //!< Value: 0x2745
    ///HEat capacity unit - Joule per Kelvin
    GATT_UNIT_JOULE_PER_KELVIN                   = GATT_UUID_16_LSB(0x2746), //!< Value: 0x2746
    ///Specific heat capacity unit - Joule per kilogram kelvin
    GATT_UNIT_JOULE_PER_KG_KELVIN                = GATT_UUID_16_LSB(0x2747), //!< Value: 0x2747
    ///Specific Energy unit - Joule per kilogram
    GATT_UNIT_JOULE_PER_KG                       = GATT_UUID_16_LSB(0x2748), //!< Value: 0x2748
    ///Thermal conductivity - Watt per metre Kelvin
    GATT_UNIT_WGATT_PER_METRE_KELVIN              = GATT_UUID_16_LSB(0x2749), //!< Value: 0x2749
    ///Energy Density unit - joule per cubic metre
    GATT_UNIT_JOULE_PER_CUBIC_METRE              = GATT_UUID_16_LSB(0x274A), //!< Value: 0x274A
    ///Electric field strength unit - volt per metre
    GATT_UNIT_VOLT_PER_METRE                     = GATT_UUID_16_LSB(0x274B), //!< Value: 0x274B
    ///Electric charge density unit - coulomb per cubic metre
    GATT_UNIT_COULOMB_PER_CUBIC_METRE            = GATT_UUID_16_LSB(0x274C), //!< Value: 0x274C
    ///Surface charge density unit - coulomb per square metre
    GATT_UNIT_SURF_COULOMB_PER_SQ_METRE          = GATT_UUID_16_LSB(0x274D), //!< Value: 0x274D
    ///Electric flux density unit - coulomb per square metre
    GATT_UNIT_FLUX_COULOMB_PER_SQ_METRE          = GATT_UUID_16_LSB(0x274E), //!< Value: 0x274E
    ///Permittivity unit - farad per metre
    GATT_UNIT_FARAD_PER_METRE                    = GATT_UUID_16_LSB(0x274F), //!< Value: 0x274F
    ///Permeability unit - henry per metre
    GATT_UNIT_HENRY_PER_METRE                    = GATT_UUID_16_LSB(0x2750), //!< Value: 0x2750
    ///Molar energy unit - joule per mole
    GATT_UNIT_JOULE_PER_MOLE                     = GATT_UUID_16_LSB(0x2751), //!< Value: 0x2751
    ///Molar entropy unit - joule per mole kelvin
    GATT_UNIT_JOULE_PER_MOLE_KELVIN              = GATT_UUID_16_LSB(0x2752), //!< Value: 0x2752
    ///Exposure unit - coulomb per kilogram
    GATT_UNIT_COULOMB_PER_KG                     = GATT_UUID_16_LSB(0x2753), //!< Value: 0x2753
    ///Absorbed dose rate unit - gray per second
    GATT_UNIT_GRAY_PER_SECOND                    = GATT_UUID_16_LSB(0x2754), //!< Value: 0x2754
    ///Radiant intensity unit - watt per steradian
    GATT_UNIT_WGATT_PER_STERADIAN                 = GATT_UUID_16_LSB(0x2755), //!< Value: 0x2755
    ///Radiance unit - watt per square meter steradian
    GATT_UNIT_WGATT_PER_SQ_METRE_STERADIAN        = GATT_UUID_16_LSB(0x2756), //!< Value: 0x2756
    ///Catalytic activity concentration unit - katal per cubic metre
    GATT_UNIT_KATAL_PER_CUBIC_METRE              = GATT_UUID_16_LSB(0x2757), //!< Value: 0x2757
    ///Time unit - minute
    GATT_UNIT_MINUTE                             = GATT_UUID_16_LSB(0x2760), //!< Value: 0x2760
    ///Time unit - hour
    GATT_UNIT_HOUR                               = GATT_UUID_16_LSB(0x2761), //!< Value: 0x2761
    ///Time unit - day
    GATT_UNIT_DAY                                = GATT_UUID_16_LSB(0x2762), //!< Value: 0x2762
    ///Plane angle unit - degree
    GATT_UNIT_ANGLE_DEGREE                       = GATT_UUID_16_LSB(0x2763), //!< Value: 0x2763
    ///Plane angle unit - minute
    GATT_UNIT_ANGLE_MINUTE                       = GATT_UUID_16_LSB(0x2764), //!< Value: 0x2764
    ///Plane angle unit - second
    GATT_UNIT_ANGLE_SECOND                       = GATT_UUID_16_LSB(0x2765), //!< Value: 0x2765
    ///Area unit - hectare
    GATT_UNIT_HECTARE                            = GATT_UUID_16_LSB(0x2766), //!< Value: 0x2766
    ///Volume unit - litre
    GATT_UNIT_LITRE                              = GATT_UUID_16_LSB(0x2767), //!< Value: 0x2767
    ///Mass unit - tonne
    GATT_UNIT_TONNE                              = GATT_UUID_16_LSB(0x2768), //!< Value: 0x2768
    ///Pressure unit - bar
    GATT_UNIT_BAR                                = GATT_UUID_16_LSB(0x2780), //!< Value: 0x2780
    ///Pressure unit - millimetre of mercury
    GATT_UNIT_MM_MERCURY                         = GATT_UUID_16_LSB(0x2781), //!< Value: 0x2781
    ///Length unit - angstrom
    GATT_UNIT_ANGSTROM                           = GATT_UUID_16_LSB(0x2782), //!< Value: 0x2782
    ///Length unit - nautical mile
    GATT_UNIT_NAUTICAL_MILE                      = GATT_UUID_16_LSB(0x2783), //!< Value: 0x2783
    ///Area unit - barn
    GATT_UNIT_BARN                               = GATT_UUID_16_LSB(0x2784), //!< Value: 0x2784
    ///Velocity unit - knot
    GATT_UNIT_KNOT                               = GATT_UUID_16_LSB(0x2785), //!< Value: 0x2785
    ///Logarithmic radio quantity unit - neper
    GATT_UNIT_NEPER                              = GATT_UUID_16_LSB(0x2786), //!< Value: 0x2786
    ///Logarithmic radio quantity unit - bel
    GATT_UNIT_BEL                                = GATT_UUID_16_LSB(0x2787), //!< Value: 0x2787
    ///Length unit - yard
    GATT_UNIT_YARD                               = GATT_UUID_16_LSB(0x27A0), //!< Value: 0x27A0
    ///Length unit - parsec
    GATT_UNIT_PARSEC                             = GATT_UUID_16_LSB(0x27A1), //!< Value: 0x27A1
    ///length unit - inch
    GATT_UNIT_INCH                               = GATT_UUID_16_LSB(0x27A2), //!< Value: 0x27A2
    ///length unit - foot
    GATT_UNIT_FOOT                               = GATT_UUID_16_LSB(0x27A3), //!< Value: 0x27A3
    ///length unit - mile
    GATT_UNIT_MILE                               = GATT_UUID_16_LSB(0x27A4), //!< Value: 0x27A4
    ///pressure unit - pound-force per square inch
    GATT_UNIT_POUND_FORCE_PER_SQ_INCH            = GATT_UUID_16_LSB(0x27A5), //!< Value: 0x27A5
    ///velocity unit - kilometre per hour
    GATT_UNIT_KM_PER_HOUR                        = GATT_UUID_16_LSB(0x27A6), //!< Value: 0x27A6
    ///velocity unit - mile per hour
    GATT_UNIT_MILE_PER_HOUR                      = GATT_UUID_16_LSB(0x27A7), //!< Value: 0x27A7
    ///angular velocity unit - revolution per minute
    GATT_UNIT_REVOLUTION_PER_MINUTE              = GATT_UUID_16_LSB(0x27A8), //!< Value: 0x27A8
    ///energy unit - gram calorie
    GATT_UNIT_GRAM_CALORIE                       = GATT_UUID_16_LSB(0x27A9), //!< Value: 0x27A9
    ///energy unit - kilogram calorie
    GATT_UNIT_KG_CALORIE                         = GATT_UUID_16_LSB(0x27AA), //!< Value: 0x27AA
    /// energy unit - kilowatt hour
    GATT_UNIT_KILOWGATT_HOUR                      = GATT_UUID_16_LSB(0x27AB), //!< Value: 0x27AB
    ///thermodynamic temperature unit - degree Fahrenheit
    GATT_UNIT_FAHRENHEIT                         = GATT_UUID_16_LSB(0x27AC), //!< Value: 0x27AC
    ///percentage
    GATT_UNIT_PERCENTAGE                         = GATT_UUID_16_LSB(0x27AD), //!< Value: 0x27AD
    ///per mille
    GATT_UNIT_PER_MILLE                          = GATT_UUID_16_LSB(0x27AE), //!< Value: 0x27AE
    ///period unit - beats per minute)
    GATT_UNIT_BEATS_PER_MINUTE                   = GATT_UUID_16_LSB(0x27AF), //!< Value: 0x27AF
    ///electric charge unit - ampere hours
    GATT_UNIT_AMPERE_HOURS                       = GATT_UUID_16_LSB(0x27B0), //!< Value: 0x27B0
    ///mass density unit - milligram per decilitre
    GATT_UNIT_MILLIGRAM_PER_DECILITRE            = GATT_UUID_16_LSB(0x27B1), //!< Value: 0x27B1
    ///mass density unit - millimole per litre
    GATT_UNIT_MILLIMOLE_PER_LITRE                = GATT_UUID_16_LSB(0x27B2), //!< Value: 0x27B2
    ///time unit - year
    GATT_UNIT_YEAR                               = GATT_UUID_16_LSB(0x27B3), //!< Value: 0x27B3
    ////time unit - month
    GATT_UNIT_MONTH                              = GATT_UUID_16_LSB(0x27B4), //!< Value: 0x27B4


    /*---------------- DECLARATIONS -----------------*/
    /// Primary service Declaration
    GATT_DECL_PRIMARY_SERVICE                     = GATT_UUID_16_LSB(0x2800), //!< Value: 0x2800
    /// Secondary service Declaration
    GATT_DECL_SECONDARY_SERVICE                   = GATT_UUID_16_LSB(0x2801), //!< Value: 0x2801
    /// Include Declaration
    GATT_DECL_INCLUDE                             = GATT_UUID_16_LSB(0x2802), //!< Value: 0x2802
    /// Characteristic Declaration
    GATT_DECL_CHARACTERISTIC                      = GATT_UUID_16_LSB(0x2803), //!< Value: 0x2803


    /*----------------- DESCRIPTORS -----------------*/
    /// Characteristic extended properties
    GATT_DESC_CHAR_EXT_PROPERTIES                 = GATT_UUID_16_LSB(0x2900), //!< Value: 0x2900
    /// Characteristic user description
    GATT_DESC_CHAR_USER_DESCRIPTION               = GATT_UUID_16_LSB(0x2901), //!< Value: 0x2901
    /// Client characteristic configuration
    GATT_DESC_CLIENT_CHAR_CFG                     = GATT_UUID_16_LSB(0x2902), //!< Value: 0x2902
    /// Server characteristic configuration
    GATT_DESC_SERVER_CHAR_CFG                     = GATT_UUID_16_LSB(0x2903), //!< Value: 0x2903
    /// Characteristic Presentation Format
    GATT_DESC_CHAR_PRES_FORMAT                    = GATT_UUID_16_LSB(0x2904), //!< Value: 0x2904
    /// Characteristic Aggregate Format
    GATT_DESC_CHAR_AGGREGATE_FORMAT               = GATT_UUID_16_LSB(0x2905), //!< Value: 0x2905
    /// Valid Range
    GATT_DESC_VALID_RANGE                         = GATT_UUID_16_LSB(0x2906), //!< Value: 0x2906
    /// External Report Reference
    GATT_DESC_EXT_REPORT_REF                      = GATT_UUID_16_LSB(0x2907), //!< Value: 0x2907
    /// Report Reference
    GATT_DESC_REPORT_REF                          = GATT_UUID_16_LSB(0x2908), //!< Value: 0x2908
    /// Environmental Sensing Configuration
    GATT_DESC_ES_CONFIGURATION                    = GATT_UUID_16_LSB(0x290B), //!< Value: 0x290B
    /// Environmental Sensing Measurement
    GATT_DESC_ES_MEASUREMENT                      = GATT_UUID_16_LSB(0x290C), //!< Value: 0x290C
    /// Environmental Sensing Trigger Setting
    GATT_DESC_ES_TRIGGER_SETTING                  = GATT_UUID_16_LSB(0x290D), //!< Value: 0x290D


    /*--------------- CHARACTERISTICS ---------------*/
    /// Device name
    GATT_CHAR_DEVICE_NAME                        = GATT_UUID_16_LSB(0x2A00), //!< Value: 0x2A00
    /// Appearance
    GATT_CHAR_APPEARANCE                         = GATT_UUID_16_LSB(0x2A01), //!< Value: 0x2A01
    /// Privacy flag
    GATT_CHAR_PRIVACY_FLAG                       = GATT_UUID_16_LSB(0x2A02), //!< Value: 0x2A02
    /// Reconnection address
    GATT_CHAR_RECONNECTION_ADDR                  = GATT_UUID_16_LSB(0x2A03), //!< Value: 0x2A03
    /// Peripheral preferred connection parameters
    GATT_CHAR_PERIPH_PREF_CON_PARAM              = GATT_UUID_16_LSB(0x2A04), //!< Value: 0x2A04
    /// Service handles changed
    GATT_CHAR_SERVICE_CHANGED                    = GATT_UUID_16_LSB(0x2A05), //!< Value: 0x2A05
    /// Alert Level characteristic
    GATT_CHAR_ALERT_LEVEL                        = GATT_UUID_16_LSB(0x2A06), //!< Value: 0x2A06
    /// Tx Power Level
    GATT_CHAR_TX_POWER_LEVEL                     = GATT_UUID_16_LSB(0x2A07), //!< Value: 0x2A07
    /// Date Time
    GATT_CHAR_DATE_TIME                          = GATT_UUID_16_LSB(0x2A08), //!< Value: 0x2A08
    /// Day of Week
    GATT_CHAR_DAY_WEEK                           = GATT_UUID_16_LSB(0x2A09), //!< Value: 0x2A09
    /// Day Date Time
    GATT_CHAR_DAY_DATE_TIME                      = GATT_UUID_16_LSB(0x2A0A), //!< Value: 0x2A0A
    /// Exact time 256
    GATT_CHAR_EXACT_TIME_256                     = GATT_UUID_16_LSB(0x2A0C), //!< Value: 0x2A0C
    /// DST Offset
    GATT_CHAR_DST_OFFSET                         = GATT_UUID_16_LSB(0x2A0D), //!< Value: 0x2A0D
    /// Time zone
    GATT_CHAR_TIME_ZONE                          = GATT_UUID_16_LSB(0x2A0E), //!< Value: 0x2A0E
    /// Local time Information
    GATT_CHAR_LOCAL_TIME_INFO                    = GATT_UUID_16_LSB(0x2A0F), //!< Value: 0x2A0F
    /// Time with DST
    GATT_CHAR_TIME_WITH_DST                      = GATT_UUID_16_LSB(0x2A11), //!< Value: 0x2A11
    /// Time Accuracy
    GATT_CHAR_TIME_ACCURACY                      = GATT_UUID_16_LSB(0x2A12), //!< Value: 0x2A12
    ///Time Source
    GATT_CHAR_TIME_SOURCE                        = GATT_UUID_16_LSB(0x2A13), //!< Value: 0x2A13
    /// Reference Time Information
    GATT_CHAR_REFERENCE_TIME_INFO                = GATT_UUID_16_LSB(0x2A14), //!< Value: 0x2A14
    /// Time Update Control Point
    GATT_CHAR_TIME_UPDATE_CNTL_POINT             = GATT_UUID_16_LSB(0x2A16), //!< Value: 0x2A16
    /// Time Update State
    GATT_CHAR_TIME_UPDATE_STATE                  = GATT_UUID_16_LSB(0x2A17), //!< Value: 0x2A17
    /// Glucose Measurement
    GATT_CHAR_GLUCOSE_MEAS                       = GATT_UUID_16_LSB(0x2A18), //!< Value: 0x2A18
    /// Battery Level
    GATT_CHAR_BATTERY_LEVEL                      = GATT_UUID_16_LSB(0x2A19), //!< Value: 0x2A19
    /// Temperature Measurement
    GATT_CHAR_TEMPERATURE_MEAS                   = GATT_UUID_16_LSB(0x2A1C), //!< Value: 0x2A1C
    /// Temperature Type
    GATT_CHAR_TEMPERATURE_TYPE                   = GATT_UUID_16_LSB(0x2A1D), //!< Value: 0x2A1D
    /// Intermediate Temperature
    GATT_CHAR_INTERMED_TEMPERATURE               = GATT_UUID_16_LSB(0x2A1E), //!< Value: 0x2A1E
    /// Measurement Interval
    GATT_CHAR_MEAS_INTERVAL                      = GATT_UUID_16_LSB(0x2A21), //!< Value: 0x2A21
    /// Boot Keyboard Input Report
    GATT_CHAR_BOOT_KB_IN_REPORT                  = GATT_UUID_16_LSB(0x2A22), //!< Value: 0x2A22
    /// System ID
    GATT_CHAR_SYS_ID                             = GATT_UUID_16_LSB(0x2A23), //!< Value: 0x2A23
    /// Model Number String
    GATT_CHAR_MODEL_NB                           = GATT_UUID_16_LSB(0x2A24), //!< Value: 0x2A24
    /// Serial Number String
    GATT_CHAR_SERIAL_NB                          = GATT_UUID_16_LSB(0x2A25), //!< Value: 0x2A25
    /// Firmware Revision String
    GATT_CHAR_FW_REV                             = GATT_UUID_16_LSB(0x2A26), //!< Value: 0x2A26
    /// Hardware revision String
    GATT_CHAR_HW_REV                             = GATT_UUID_16_LSB(0x2A27), //!< Value: 0x2A27
    /// Software Revision String
    GATT_CHAR_SW_REV                             = GATT_UUID_16_LSB(0x2A28), //!< Value: 0x2A28
    /// Manufacturer Name String
    GATT_CHAR_MANUF_NAME                         = GATT_UUID_16_LSB(0x2A29), //!< Value: 0x2A29
    /// IEEE Regulatory Certification Data List
    GATT_CHAR_IEEE_CERTIF                        = GATT_UUID_16_LSB(0x2A2A), //!< Value: 0x2A2A
    /// CT Time
    GATT_CHAR_CT_TIME                            = GATT_UUID_16_LSB(0x2A2B), //!< Value: 0x2A2B
    /// Magnetic Declination
    GATT_CHAR_MAGN_DECLINE                       = GATT_UUID_16_LSB(0x2A2C), //!< Value: 0x2A2C
    /// Scan Refresh
    GATT_CHAR_SCAN_REFRESH                       = GATT_UUID_16_LSB(0x2A31), //!< Value: 0x2A31
    /// Boot Keyboard Output Report
    GATT_CHAR_BOOT_KB_OUT_REPORT                 = GATT_UUID_16_LSB(0x2A32), //!< Value: 0x2A32
    /// Boot Mouse Input Report
    GATT_CHAR_BOOT_MOUSE_IN_REPORT               = GATT_UUID_16_LSB(0x2A33), //!< Value: 0x2A33
    /// Glucose Measurement Context
    GATT_CHAR_GLUCOSE_MEAS_CTX                   = GATT_UUID_16_LSB(0x2A34), //!< Value: 0x2A34
    /// Blood Pressure Measurement
    GATT_CHAR_BLOOD_PRESSURE_MEAS                = GATT_UUID_16_LSB(0x2A35), //!< Value: 0x2A35
    /// Intermediate Cuff Pressure
    GATT_CHAR_INTERMEDIATE_CUFF_PRESSURE         = GATT_UUID_16_LSB(0x2A36), //!< Value: 0x2A36
    /// Heart Rate Measurement
    GATT_CHAR_HEART_RATE_MEAS                    = GATT_UUID_16_LSB(0x2A37), //!< Value: 0x2A37
    /// Body Sensor Location
    GATT_CHAR_BODY_SENSOR_LOCATION               = GATT_UUID_16_LSB(0x2A38), //!< Value: 0x2A38
    /// Heart Rate Control Point
    GATT_CHAR_HEART_RATE_CNTL_POINT              = GATT_UUID_16_LSB(0x2A39), //!< Value: 0x2A39
    /// Alert Status
    GATT_CHAR_ALERT_STATUS                       = GATT_UUID_16_LSB(0x2A3F), //!< Value: 0x2A3F
    /// Ringer Control Point
    GATT_CHAR_RINGER_CNTL_POINT                  = GATT_UUID_16_LSB(0x2A40), //!< Value: 0x2A40
    /// Ringer Setting
    GATT_CHAR_RINGER_SETTING                     = GATT_UUID_16_LSB(0x2A41), //!< Value: 0x2A41
    /// Alert Category ID Bit Mask
    GATT_CHAR_ALERT_CAT_ID_BIT_MASK              = GATT_UUID_16_LSB(0x2A42), //!< Value: 0x2A42
    /// Alert Category ID
    GATT_CHAR_ALERT_CAT_ID                       = GATT_UUID_16_LSB(0x2A43), //!< Value: 0x2A43
    /// Alert Notification Control Point
    GATT_CHAR_ALERT_NTF_CTNL_PT                  = GATT_UUID_16_LSB(0x2A44), //!< Value: 0x2A44
    /// Unread Alert Status
    GATT_CHAR_UNREAD_ALERT_STATUS                = GATT_UUID_16_LSB(0x2A45), //!< Value: 0x2A45
    /// New Alert
    GATT_CHAR_NEW_ALERT                          = GATT_UUID_16_LSB(0x2A46), //!< Value: 0x2A46
    /// Supported New Alert Category
    GATT_CHAR_SUP_NEW_ALERT_CAT                  = GATT_UUID_16_LSB(0x2A47), //!< Value: 0x2A47
    /// Supported Unread Alert Category
    GATT_CHAR_SUP_UNREAD_ALERT_CAT               = GATT_UUID_16_LSB(0x2A48), //!< Value: 0x2A48
    /// Blood Pressure Feature
    GATT_CHAR_BLOOD_PRESSURE_FEATURE             = GATT_UUID_16_LSB(0x2A49), //!< Value: 0x2A49
    /// HID Information
    GATT_CHAR_HID_INFO                           = GATT_UUID_16_LSB(0x2A4A), //!< Value: 0x2A4A
    /// Report Map
    GATT_CHAR_REPORT_MAP                         = GATT_UUID_16_LSB(0x2A4B), //!< Value: 0x2A4B
    /// HID Control Point
    GATT_CHAR_HID_CTNL_PT                        = GATT_UUID_16_LSB(0x2A4C), //!< Value: 0x2A4C
    /// Report
    GATT_CHAR_REPORT                             = GATT_UUID_16_LSB(0x2A4D), //!< Value: 0x2A4D
    /// Protocol Mode
    GATT_CHAR_PROTOCOL_MODE                      = GATT_UUID_16_LSB(0x2A4E), //!< Value: 0x2A4E
    /// Scan Interval Window
    GATT_CHAR_SCAN_INTV_WD                       = GATT_UUID_16_LSB(0x2A4F), //!< Value: 0x2A4F
    /// PnP ID
    GATT_CHAR_PNP_ID                             = GATT_UUID_16_LSB(0x2A50), //!< Value: 0x2A50
    /// Glucose Feature
    GATT_CHAR_GLUCOSE_FEATURE                    = GATT_UUID_16_LSB(0x2A51), //!< Value: 0x2A51
    /// Record access control point
    GATT_CHAR_REC_ACCESS_CTRL_PT                 = GATT_UUID_16_LSB(0x2A52), //!< Value: 0x2A52
    /// RSC Measurement
    GATT_CHAR_RSC_MEAS                           = GATT_UUID_16_LSB(0x2A53), //!< Value: 0x2A53
    /// RSC Feature
    GATT_CHAR_RSC_FEAT                           = GATT_UUID_16_LSB(0x2A54), //!< Value: 0x2A54
    /// SC Control Point
    GATT_CHAR_SC_CNTL_PT                         = GATT_UUID_16_LSB(0x2A55), //!< Value: 0x2A55
    /// CSC Measurement
    GATT_CHAR_CSC_MEAS                           = GATT_UUID_16_LSB(0x2A5B), //!< Value: 0x2A5B
    /// CSC Feature
    GATT_CHAR_CSC_FEAT                           = GATT_UUID_16_LSB(0x2A5C), //!< Value: 0x2A5C
    /// Sensor Location
    GATT_CHAR_SENSOR_LOC                         = GATT_UUID_16_LSB(0x2A5D), //!< Value: 0x2A5D
    /// PLX Spot-Check Measurement
    GATT_CHAR_PLX_SPOT_CHECK_MEASUREMENT_LOC     = GATT_UUID_16_LSB(0x2A5E), //!< Value: 0x2A5E
    /// PLX Continuous Measurement
    GATT_CHAR_PLX_CONTINUOUS_MEASUREMENT_LOC     = GATT_UUID_16_LSB(0x2A5F), //!< Value: 0x2A5F
    /// PLX Features
    GATT_CHAR_PLX_FEATURES_LOC                   = GATT_UUID_16_LSB(0x2A60), //!< Value: 0x2A60
    /// CP Measurement
    GATT_CHAR_CP_MEAS                            = GATT_UUID_16_LSB(0x2A63), //!< Value: 0x2A63
    /// CP Vector
    GATT_CHAR_CP_VECTOR                          = GATT_UUID_16_LSB(0x2A64), //!< Value: 0x2A64
    /// CP Feature
    GATT_CHAR_CP_FEAT                            = GATT_UUID_16_LSB(0x2A65), //!< Value: 0x2A65
    /// CP Control Point
    GATT_CHAR_CP_CNTL_PT                         = GATT_UUID_16_LSB(0x2A66), //!< Value: 0x2A66
    /// Location and Speed
    GATT_CHAR_LOC_SPEED                          = GATT_UUID_16_LSB(0x2A67), //!< Value: 0x2A67
    /// Navigation
    GATT_CHAR_NAVIGATION                         = GATT_UUID_16_LSB(0x2A68), //!< Value: 0x2A68
    /// Position Quality
    GATT_CHAR_POS_QUALITY                        = GATT_UUID_16_LSB(0x2A69), //!< Value: 0x2A69
    /// LN Feature
    GATT_CHAR_LN_FEAT                            = GATT_UUID_16_LSB(0x2A6A), //!< Value: 0x2A6A
    /// LN Control Point
    GATT_CHAR_LN_CNTL_PT                         = GATT_UUID_16_LSB(0x2A6B), //!< Value: 0x2A6B
    /// Elevation
    GATT_CHAR_ELEVATION                          = GATT_UUID_16_LSB(0x2A6C), //!< Value: 0x2A6C
    /// Pressure
    GATT_CHAR_PRESSURE                           = GATT_UUID_16_LSB(0x2A6D), //!< Value: 0x2A6D
    /// Temperature
    GATT_CHAR_TEMPERATURE                        = GATT_UUID_16_LSB(0x2A6E), //!< Value: 0x2A6E
    /// Humidity
    GATT_CHAR_HUMIDITY                           = GATT_UUID_16_LSB(0x2A6F), //!< Value: 0x2A6F
    /// True Wind Speed
    GATT_CHAR_TRUE_WIND_SPEED                    = GATT_UUID_16_LSB(0x2A70), //!< Value: 0x2A70
    /// True Wind Direction
    GATT_CHAR_TRUE_WIND_DIR                      = GATT_UUID_16_LSB(0x2A71), //!< Value: 0x2A71
    /// Apparent Wind Speed
    GATT_CHAR_APRNT_WIND_SPEED                   = GATT_UUID_16_LSB(0x2A72), //!< Value: 0x2A72
    /// Apparent Wind Direction
    GATT_CHAR_APRNT_WIND_DIRECTION               = GATT_UUID_16_LSB(0x2A73), //!< Value: 0x2A73
    /// Gust Factor
    GATT_CHAR_GUST_FACTOR                        = GATT_UUID_16_LSB(0x2A74), //!< Value: 0x2A74
    /// Pollen Concentration
    GATT_CHAR_POLLEN_CONC                        = GATT_UUID_16_LSB(0x2A75), //!< Value: 0x2A75
    /// UV Index
    GATT_CHAR_UV_INDEX                           = GATT_UUID_16_LSB(0x2A76), //!< Value: 0x2A76
    /// Irradiance
    GATT_CHAR_IRRADIANCE                         = GATT_UUID_16_LSB(0x2A77), //!< Value: 0x2A77
    /// Rainfall
    GATT_CHAR_RAINFALL                           = GATT_UUID_16_LSB(0x2A78), //!< Value: 0x2A78
    /// Wind Chill
    GATT_CHAR_WIND_CHILL                         = GATT_UUID_16_LSB(0x2A79), //!< Value: 0x2A79
    /// Heat Index
    GATT_CHAR_HEAT_INDEX                         = GATT_UUID_16_LSB(0x2A7A), //!< Value: 0x2A7A
    /// Dew Point
    GATT_CHAR_DEW_POINT                          = GATT_UUID_16_LSB(0x2A7B), //!< Value: 0x2A7B
    /// Descriptor Value Changed
    GATT_CHAR_DESCRIPTOR_VALUE_CHANGED           = GATT_UUID_16_LSB(0x2A7D), //!< Value: 0x2A7D
    /// Aerobic Heart Rate Lower Limit
    GATT_CHAR_AEROBIC_HEART_RATE_LOW_LIM         = GATT_UUID_16_LSB(0x2A7E), //!< Value: 0x2A7E
    /// Aerobic Threshhold
    GATT_CHAR_AEROBIC_THR                        = GATT_UUID_16_LSB(0x2A7F), //!< Value: 0x2A7F
    /// Age
    GATT_CHAR_AGE                                = GATT_UUID_16_LSB(0x2A80), //!< Value: 0x2A80
    /// Anaerobic Heart Rate Lower Limit
    GATT_CHAR_ANAERO_HEART_RATE_LOW_LIM          = GATT_UUID_16_LSB(0x2A81), //!< Value: 0x2A81
    /// Anaerobic Heart Rate Upper Limit
    GATT_CHAR_ANAERO_HEART_RATE_UP_LIM           = GATT_UUID_16_LSB(0x2A82), //!< Value: 0x2A82
    /// Anaerobic Threshhold
    GATT_CHAR_ANAERO_THR                         = GATT_UUID_16_LSB(0x2A83), //!< Value: 0x2A83
    /// Aerobic Heart Rate Upper Limit
    GATT_CHAR_AEROBIC_HEART_RATE_UP_LIM          = GATT_UUID_16_LSB(0x2A84), //!< Value: 0x2A84
    /// Date Of Birth
    GATT_CHAR_DATE_OF_BIRTH                      = GATT_UUID_16_LSB(0x2A85), //!< Value: 0x2A85
    /// Date Of Threshold Assessment
    GATT_CHAR_DATE_OF_THR_ASSESS                 = GATT_UUID_16_LSB(0x2A86), //!< Value: 0x2A86
    /// Email Address
    GATT_CHAR_EMAIL_ADDRESS                      = GATT_UUID_16_LSB(0x2A87), //!< Value: 0x2A87
    /// Fat Burn Heart Rate Lower Limit
    GATT_CHAR_FAT_BURN_HEART_RATE_LOW_LIM        = GATT_UUID_16_LSB(0x2A88), //!< Value: 0x2A88
    /// Fat Burn Heart Rate Upper Limit
    GATT_CHAR_FAT_BURN_HEART_RATE_UP_LIM         = GATT_UUID_16_LSB(0x2A89), //!< Value: 0x2A89
    /// First Name
    GATT_CHAR_FIRST_NAME                         = GATT_UUID_16_LSB(0x2A8A), //!< Value: 0x2A8A
    /// Five Zone Heart Rate Limits
    GATT_CHAR_FIVE_ZONE_HEART_RATE_LIMITS        = GATT_UUID_16_LSB(0x2A8B), //!< Value: 0x2A8B
    /// Gender
    GATT_CHAR_GENDER                             = GATT_UUID_16_LSB(0x2A8C), //!< Value: 0x2A8C
    /// Max Heart Rate
    GATT_CHAR_MAX_HEART_RATE                     = GATT_UUID_16_LSB(0x2A8D), //!< Value: 0x2A8D
    /// Height
    GATT_CHAR_HEIGHT                             = GATT_UUID_16_LSB(0x2A8E), //!< Value: 0x2A8E
    /// Hip Circumference
    GATT_CHAR_HIP_CIRCUMFERENCE                  = GATT_UUID_16_LSB(0x2A8F), //!< Value: 0x2A8F
    /// Last Name
    GATT_CHAR_LAST_NAME                          = GATT_UUID_16_LSB(0x2A90), //!< Value: 0x2A90
    /// Maximum Recommended Heart Rate
    GATT_CHAR_MAX_RECO_HEART_RATE                = GATT_UUID_16_LSB(0x2A91), //!< Value: 0x2A91
    /// Resting Heart Rate
    GATT_CHAR_RESTING_HEART_RATE                 = GATT_UUID_16_LSB(0x2A92), //!< Value: 0x2A92
    /// Sport Type For Aerobic And Anaerobic Thresholds
    GATT_CHAR_SPORT_TYPE_FOR_AERO_ANAREO_THRS    = GATT_UUID_16_LSB(0x2A93), //!< Value: 0x2A93
    /// Three Zone Heart Rate Limits
    GATT_CHAR_THREE_ZONE_HEART_RATE_LIMITS       = GATT_UUID_16_LSB(0x2A94), //!< Value: 0x2A94
    /// Two Zone Heart Rate Limit
    GATT_CHAR_TWO_ZONE_HEART_RATE_LIMIT          = GATT_UUID_16_LSB(0x2A95), //!< Value: 0x2A95
    /// Vo2 Max
    GATT_CHAR_VO2_MAX                            = GATT_UUID_16_LSB(0x2A96), //!< Value: 0x2A96
    /// Waist Circumference
    GATT_CHAR_WAIST_CIRCUMFERENCE                = GATT_UUID_16_LSB(0x2A97), //!< Value: 0x2A97
    /// Weight
    GATT_CHAR_WEIGHT                             = GATT_UUID_16_LSB(0x2A98), //!< Value: 0x2A98
    /// Database Change Increment
    GATT_CHAR_DB_CHG_INCREMENT                   = GATT_UUID_16_LSB(0x2A99), //!< Value: 0x2A99
    /// User Index
    GATT_CHAR_USER_INDEX                         = GATT_UUID_16_LSB(0x2A9A), //!< Value: 0x2A9A
    /// Body Composition Feature
    GATT_CHAR_BODY_COMPOSITION_FEATURE           = GATT_UUID_16_LSB(0x2A9B), //!< Value: 0x2A9B
    /// Body Composition Measurement
    GATT_CHAR_BODY_COMPOSITION_MEASUREMENT       = GATT_UUID_16_LSB(0x2A9C), //!< Value: 0x2A9C
    /// Weight Measurement
    GATT_CHAR_WEIGHT_MEASUREMENT                 = GATT_UUID_16_LSB(0x2A9D), //!< Value: 0x2A9D
    /// Weight Scale Feature
    GATT_CHAR_WEIGHT_SCALE_FEATURE               = GATT_UUID_16_LSB(0x2A9E), //!< Value: 0x2A9E
    /// User Control Point
    GATT_CHAR_USER_CONTROL_POINT                 = GATT_UUID_16_LSB(0x2A9F), //!< Value: 0x2A9F
    /// Flux Density - 2D
    GATT_CHAR_MAGN_FLUX_2D                       = GATT_UUID_16_LSB(0x2AA0), //!< Value: 0x2AA0
    /// Magnetic Flux Density - 3D
    GATT_CHAR_MAGN_FLUX_3D                       = GATT_UUID_16_LSB(0x2AA1), //!< Value: 0x2AA1
    /// Language string
    GATT_CHAR_LANGUAGE                           = GATT_UUID_16_LSB(0x2AA2), //!< Value: 0x2AA2
    /// Barometric Pressure Trend
    GATT_CHAR_BAR_PRES_TREND                     = GATT_UUID_16_LSB(0x2AA3), //!< Value: 0x2AA3
    /// Central Address Resolution Support
    GATT_CHAR_CTL_ADDR_RESOL_SUPP                = GATT_UUID_16_LSB(0x2AA6), //!< Value: 0x2AA6
    /// CGM Measurement
    GATT_CHAR_CGM_MEASUREMENT                    = GATT_UUID_16_LSB(0x2AA7), //!< Value: 0x2AA7
    /// CGM Features
    GATT_CHAR_CGM_FEATURES                       = GATT_UUID_16_LSB(0x2AA8), //!< Value: 0x2AA8
    /// CGM Status
    GATT_CHAR_CGM_STATUS                         = GATT_UUID_16_LSB(0x2AA9), //!< Value: 0x2AA9
    /// CGM Session Start
    GATT_CHAR_CGM_SESSION_START                  = GATT_UUID_16_LSB(0x2AAA), //!< Value: 0x2AAA
    /// CGM Session Run
    GATT_CHAR_CGM_SESSION_RUN                    = GATT_UUID_16_LSB(0x2AAB), //!< Value: 0x2AAB
    /// CGM Specific Ops Control Point
    GATT_CHAR_CGM_SPECIFIC_OPS_CTRL_PT           = GATT_UUID_16_LSB(0x2AAC), //!< Value: 0x2AAC

    /// Object Transfer Service - Feature characteristic
    GATT_CHAR_OTS_FEATURE                        = GATT_UUID_16_LSB(0x2ABD), //!< Value: 0x2ABD
    /// Object Transfer Service - Object Name characteristic
    GATT_CHAR_OTS_OBJECT_NAME                    = GATT_UUID_16_LSB(0x2ABE), //!< Value: 0x2ABE
    /// Object Transfer Service - Object Type characteristic
    GATT_CHAR_OTS_OBJECT_TYPE                    = GATT_UUID_16_LSB(0x2ABF), //!< Value: 0x2ABF
    /// Object Transfer Service - Object Size characteristic
    GATT_CHAR_OTS_OBJECT_SIZE                    = GATT_UUID_16_LSB(0x2AC0), //!< Value: 0x2AC0
    /// Object Transfer Service - Object First-Created characteristic
    GATT_CHAR_OTS_OBJECT_FIRST_CRAETED           = GATT_UUID_16_LSB(0x2AC1), //!< Value: 0x2AC1
    /// Object Transfer Service - Object Last-Modified characteristic
    GATT_CHAR_OTS_OBJECT_LAST_MODIFIED           = GATT_UUID_16_LSB(0x2AC2), //!< Value: 0x2AC2
    /// Object Transfer Service - Object ID characteristic
    GATT_CHAR_OTS_OBJECT_ID                      = GATT_UUID_16_LSB(0x2AC3), //!< Value: 0x2AC3
    /// Object Transfer Service - Object Properties characteristic
    GATT_CHAR_OTS_OBJECT_PROPERTIES              = GATT_UUID_16_LSB(0x2AC4), //!< Value: 0x2AC4
    /// Object Transfer Service - Object Action Control Point characteristic
    GATT_CHAR_OTS_OACP                           = GATT_UUID_16_LSB(0x2AC5), //!< Value: 0x2AC5
    /// Object Transfer Service - Object List Control Point characteristic
    GATT_CHAR_OTS_OLCP                           = GATT_UUID_16_LSB(0x2AC6), //!< Value: 0x2AC6
    /// Object Transfer Service - Object List Filter characteristic
    GATT_CHAR_OTS_OBJECT_LIST_FILTER             = GATT_UUID_16_LSB(0x2AC7), //!< Value: 0x2AC7
    /// Object Transfer Service - Object Changed characteristic
    GATT_CHAR_OTS_OBJECT_CHANGED                 = GATT_UUID_16_LSB(0x2AC8), //!< Value: 0x2AC8

    /// Resolvable Private Address only
    GATT_CHAR_RSLV_PRIV_ADDR_ONLY                = GATT_UUID_16_LSB(0x2AC9), //!< Value: 0x2AC9

    /// Mesh Provisioning Data In
    GATT_CHAR_MESH_PROV_DATA_IN                  = GATT_UUID_16_LSB(0x2ADB), //!< Value: 0x2ADB
    /// Mesh Provisioning Data Out
    GATT_CHAR_MESH_PROV_DATA_OUT                 = GATT_UUID_16_LSB(0x2ADC), //!< Value: 0x2ADC
    /// Mesh Proxy Data In
    GATT_CHAR_MESH_PROXY_DATA_IN                 = GATT_UUID_16_LSB(0x2ADD), //!< Value: 0x2ADD
    /// Mesh Proxy Data Out
    GATT_CHAR_MESH_PROXY_DATA_OUT                = GATT_UUID_16_LSB(0x2ADE), //!< Value: 0x2ADE

    /// Volume Control Service - Volume State characteristic
    GATT_CHAR_VOLUME_STATE                       = GATT_UUID_16_LSB(0x2B7D), //!< Value: 0x2B7D
    /// Volume Control Service - Volume Control Point characteristic
    GATT_CHAR_VOLUME_CP                          = GATT_UUID_16_LSB(0x2B7E), //!< Value: 0x2B7E
    /// Volume Control Service - Volume Flags characteristic
    GATT_CHAR_VOLUME_FLAGS                       = GATT_UUID_16_LSB(0x2B7F), //!< Value: 0x2B7F

    /// Microphone Control Service - Mute characteristic
    GATT_CHAR_MUTE                               = GATT_UUID_16_LSB(0x2BC3), //!< Value: 0x2BC3

    /// Volume Offset Control Service - Volume Offset State characteristic
    GATT_CHAR_OFFSET_STATE                       = GATT_UUID_16_LSB(0x2B80), //!< Value: 0x2B80
    /// Volume Offset Control Service - Audio Location characteristic
    GATT_CHAR_AUDIO_LOC                          = GATT_UUID_16_LSB(0x2B81), //!< Value: 0x2B81
    /// Volume Offset Control Service - Volume Offset Control Point characteristic
    GATT_CHAR_OFFSET_CP                          = GATT_UUID_16_LSB(0x2B82), //!< Value: 0x2B82
    /// Volume Offset Control Service - Audio Output Description characteristic
    GATT_CHAR_OUTPUT_DESC                        = GATT_UUID_16_LSB(0x2B83), //!< Value: 0x2B83

    /// Audio Input Control Service - Audio Input State characteristic
    GATT_CHAR_INPUT_STATE                        = GATT_UUID_16_LSB(0x2B77), //!< Value: 0x2B77
    /// Audio Input Control Service - Gain Setting Properties characteristic
    GATT_CHAR_GAIN_PROP                          = GATT_UUID_16_LSB(0x2B78), //!< Value: 0x2B78
    /// Audio Input Control Service - Audio Input Type characteristic
    GATT_CHAR_INPUT_TYPE                         = GATT_UUID_16_LSB(0x2B79), //!< Value: 0x2B79
    /// Audio Input Control Service - Audio Input Status characteristic
    GATT_CHAR_INPUT_STATUS                       = GATT_UUID_16_LSB(0x2B7A), //!< Value: 0x2B7A
    /// Audio Input Control Service - Audio Input Control Point characteristic
    GATT_CHAR_INPUT_CP                           = GATT_UUID_16_LSB(0x2B7B), //!< Value: 0x2B7B
    /// Audio Input Control Service - Audio Input Description characteristic
    GATT_CHAR_INPUT_DESC                         = GATT_UUID_16_LSB(0x2B7C), //!< Value: 0x2B7C  = GATT_UUID_16_LSB(0x8FA3), //!< Value: 0x8FA3

    /// Telephone Bearer Service - Bearer Provider Name characteristic
    GATT_CHAR_TBS_PROV_NAME                      = GATT_UUID_16_LSB(0x2BB3), //!< Value: 0x2BB3
    /// Telephone Bearer Service - Bearer UCI characteristic
    GATT_CHAR_TBS_UCI                            = GATT_UUID_16_LSB(0x2BB4), //!< Value: 0x2BB4
    /// Telephone Bearer Service - Bearer Technology characteristic
    GATT_CHAR_TBS_TECHNO                         = GATT_UUID_16_LSB(0x2BB5), //!< Value: 0x2BB5
    /// Telephone Bearer Service - Bearer URI Schemes Supported List characteristic
    GATT_CHAR_TBS_URI_SCHEMES_LIST               = GATT_UUID_16_LSB(0x2BB6), //!< Value: 0x2BB6
    /// Telephone Bearer Service - Bearer Signal Strength characteristic
    GATT_CHAR_TBS_SIGN_STRENGTH                  = GATT_UUID_16_LSB(0x2BB7), //!< Value: 0x2BB7
    /// Telephone Bearer Service - Bearer Signal Strength Reporting Interval characteristic
    GATT_CHAR_TBS_SIGN_STRENGTH_INTV             = GATT_UUID_16_LSB(0x2BB8), //!< Value: 0x2BB8
    /// Telephone Bearer Service - Bearer List Current Calls characteristic
    GATT_CHAR_TBS_CURR_CALLS_LIST                = GATT_UUID_16_LSB(0x2BB9), //!< Value: 0x2BB9
    /// Telephone Bearer Service - Status Flags characteristic
    GATT_CHAR_TBS_STATUS_FLAGS                   = GATT_UUID_16_LSB(0x2BBB), //!< Value: 0x2BBB
    /// Telephone Bearer Service - Incoming Call Target Bearer URI characteristic
    GATT_CHAR_TBS_IN_TGT_CALLER_ID               = GATT_UUID_16_LSB(0x2BBC), //!< Value: 0x2BBC
    /// Telephone Bearer Service - Call State characteristic
    GATT_CHAR_TBS_CALL_STATE                     = GATT_UUID_16_LSB(0x2BBD), //!< Value: 0x2BBD
    /// Telephone Bearer Service - Call Control Point characteristic
    GATT_CHAR_TBS_CALL_CP                        = GATT_UUID_16_LSB(0x2BBE), //!< Value: 0x2BBE
    /// Telephone Bearer Service - Call Control Point Optional Opcodes characteristic
    GATT_CHAR_TBS_CALL_CP_OPT_OPCODES            = GATT_UUID_16_LSB(0x2BBF), //!< Value: 0x2BBF
    /// Telephone Bearer Service - Termination Reason characteristic
    GATT_CHAR_TBS_TERM_REASON                    = GATT_UUID_16_LSB(0x2BC0), //!< Value: 0x2BC0
    /// Telephone Bearer Service - Incoming Call characteristic
    GATT_CHAR_TBS_INCOMING_CALL                  = GATT_UUID_16_LSB(0x2BC1), //!< Value: 0x2BC1
    /// Telephone Bearer Service - Call Friendly Name characteristic
    GATT_CHAR_TBS_CALL_FRIENDLY_NAME             = GATT_UUID_16_LSB(0x2BC2), //!< Value: 0x2BC2

    /// Media Control Service - Media Player Name characteristic
    GATT_CHAR_MCS_PLAYER_NAME                    = GATT_UUID_16_LSB(0x2B93), //!< Value: 0x2B93
    /// Media Control Service - Media Player Icon Object ID characteristic
    GATT_CHAR_MCS_PLAYER_ICON_OBJ_ID             = GATT_UUID_16_LSB(0x2B94), //!< Value: 0x2B94
    /// Media Control Service - Media Player Icon URL characteristic
    GATT_CHAR_MCS_PLAYER_ICON_URL                = GATT_UUID_16_LSB(0x2B95), //!< Value: 0x2B95
    /// Media Control Service - Track Changed characteristic
    GATT_CHAR_MCS_TRACK_CHANGED                  = GATT_UUID_16_LSB(0x2B96), //!< Value: 0x2B96
    /// Media Control Service - Track Title characteristic
    GATT_CHAR_MCS_TRACK_TITLE                    = GATT_UUID_16_LSB(0x2B97), //!< Value: 0x2B97
    /// Media Control Service - Track Duration characteristic
    GATT_CHAR_MCS_TRACK_DURATION                 = GATT_UUID_16_LSB(0x2B98), //!< Value: 0x2B98
    /// Media Control Service - Track Position characteristic
    GATT_CHAR_MCS_TRACK_POSITION                 = GATT_UUID_16_LSB(0x2B99), //!< Value: 0x2B99
    /// Media Control Service - Playback Speed characteristic
    GATT_CHAR_MCS_PLAYBACK_SPEED                 = GATT_UUID_16_LSB(0x2B9A), //!< Value: 0x2B9A
    /// Media Control Service - Seeking Speed characteristic
    GATT_CHAR_MCS_SEEKING_SPEED                  = GATT_UUID_16_LSB(0x2B9B), //!< Value: 0x2B9B
    /// Media Control Service - Current Track Segments Object ID characteristic
    GATT_CHAR_MCS_CUR_TRACK_SEG_OBJ_ID           = GATT_UUID_16_LSB(0x2B9C), //!< Value: 0x2B9C
    /// Media Control Service - Current Track Object ID characteristic
    GATT_CHAR_MCS_CUR_TRACK_OBJ_ID               = GATT_UUID_16_LSB(0x2B9D), //!< Value: 0x2B9D
    /// Media Control Service - Next Track Object ID characteristic
    GATT_CHAR_MCS_NEXT_TRACK_OBJ_ID              = GATT_UUID_16_LSB(0x2B9E), //!< Value: 0x2B9E
    /// Media Control Service - Current Group Object ID characteristic
    GATT_CHAR_MCS_CUR_GROUP_OBJ_ID               = GATT_UUID_16_LSB(0x2BA0), //!< Value: 0x2BA0
    /// Media Control Service - Parent Group Object ID characteristic
    GATT_CHAR_MCS_PARENT_GROUP_OBJ_ID            = GATT_UUID_16_LSB(0x2B9F), //!< Value: 0x2B9F
    /// Media Control Service - Playing Order characteristic
    GATT_CHAR_MCS_PLAYING_ORDER                  = GATT_UUID_16_LSB(0x2BA1), //!< Value: 0x2BA1
    /// Media Control Service - Playing Order Supported characteristic
    GATT_CHAR_MCS_PLAYING_ORDER_SUPP             = GATT_UUID_16_LSB(0x2BA2), //!< Value: 0x2BA2
    /// Media Control Service - Media State characteristic
    GATT_CHAR_MCS_MEDIA_STATE                    = GATT_UUID_16_LSB(0x2BA3), //!< Value: 0x2BA3
    /// Media Control Service - Media Control Point characteristic
    GATT_CHAR_MCS_MEDIA_CP                       = GATT_UUID_16_LSB(0x2BA4), //!< Value: 0x2BA4
    /// Media Control Service - Media Control Point Opcodes Supported characteristic
    GATT_CHAR_MCS_MEDIA_CP_OPCODES_SUPP          = GATT_UUID_16_LSB(0x2BA5), //!< Value: 0x2BA5
    /// Media Control Service - Search Results Object ID characteristic
    GATT_CHAR_MCS_SEARCH_RESULTS_OBJ_ID          = GATT_UUID_16_LSB(0x2BA6), //!< Value: 0x2BA6
    /// Media Control Service - Search Control Point characteristic
    GATT_CHAR_MCS_SEARCH_CP                      = GATT_UUID_16_LSB(0x2BA7), //!< Value: 0x2BA7

    /// Content Control ID characteristic
    GATT_CHAR_CCID                               = GATT_UUID_16_LSB(0x2BBA), //!< Value: 0x2BBA

    /// Telephony and Media Audio Service - TMAP Role characteristic
    GATT_CHAR_TMAS_ROLE                          = GATT_UUID_16_LSB(0x8FC9), //!< Value: 0x8FC9

    /// Hearing Aid Service - Presets characteristic
    GATT_CHAR_HAS_PRESETS                        = GATT_UUID_16_LSB(0x8FCA), //!< Value: 0x8FCA
    /// Hearing Aid Service - Active Preset characteristic
    GATT_CHAR_HAS_ACTIVE_PRESET                  = GATT_UUID_16_LSB(0x8FCB), //!< Value: 0x8FCB
    /// Hearing Aid Service - Preset Control Point characteristic
    GATT_CHAR_HAS_CP                             = GATT_UUID_16_LSB(0x8FCC), //!< Value: 0x8FCC
    /// Hearing Aid Service - Flags
    GATT_CHAR_HAS_FLAGS                          = GATT_UUID_16_LSB(0x8FCD), //!< Value: 0x8FCD

    /// Gaming Audio Service - GMAP Role characteristic
    GATT_CHAR_GMAS_ROLE                          = GATT_UUID_16_LSB(0x8FCE), //!< Value: 0x8FCE
    /// Gaming Audio Service - GMAP UGT Feature characteristic
    GATT_CHAR_GMAS_UGT_FEAT                      = GATT_UUID_16_LSB(0x8FCF), //!< Value: 0x8FCF

    /// Client Supported Features
    GATT_CHAR_CLI_SUP_FEAT                       = GATT_UUID_16_LSB(0x2B29), //!< Value: 0x2B29
    /// Database Hash
    GATT_CHAR_DB_HASH                            = GATT_UUID_16_LSB(0x2B2A), //!< Value: 0x2B2A
    /// Server Supported Features
    GATT_CHAR_SRV_SUP_FEAT                       = GATT_UUID_16_LSB(0x2B3A), //!< Value: 0x2B3A

    /// Published Audio Capabilities Service
    GATT_SVC_PUBLISHED_AUDIO_CAPA                = GATT_UUID_16_LSB(0x1850),
    /// Broadcast Audio Scan Service
    GATT_SVC_BCAST_AUDIO_SCAN                    = GATT_UUID_16_LSB(0x184F),
    /// Audio Stream Control Service
    GATT_SVC_AUDIO_STREAM_CTRL                   = GATT_UUID_16_LSB(0x184E),
    /// Broadcast Audio Announcement UUID
    GATT_SVC_BCAST_AUDIO_ANNOUNCEMENT            = GATT_UUID_16_LSB(0x1852),
    /// Basic Audio Announcement UUID
    GATT_SVC_BASIC_AUDIO_ANNOUNCEMENT            = GATT_UUID_16_LSB(0x1851),
    ///public Broadcast Announcement UUID
    GATT_SVC_PUBILC_BCAST_ANNOUNCEMENT           = GATT_UUID_16_LSB(0x1856),

    /// Published Audio Capabilities Service - Sink PAC characteristic
    GATT_CHAR_PAC_SINK                          = GATT_UUID_16_LSB(0x2BC9),
    /// Published Audio Capabilities Service - Sink Audio Location characteristic
    GATT_CHAR_LOC_SINK                          = GATT_UUID_16_LSB(0x2BCA),
    /// Published Audio Capabilities Service - Source PAC characteristic
    GATT_CHAR_PAC_SRC                           = GATT_UUID_16_LSB(0x2BCB),
    /// Published Audio Capabilities Service - Source Audio Location characteristic
    GATT_CHAR_LOC_SRC                           = GATT_UUID_16_LSB(0x2BCC),
    /// Published Audio Capabilities Service - Available Audio Contexts
    GATT_CHAR_CONTEXT_AVA                       = GATT_UUID_16_LSB(0x2BCD),
    /// Published Audio Capabilities Service - Supported Audio Contexts
    GATT_CHAR_CONTEXT_SUPP                      = GATT_UUID_16_LSB(0x2BCE),

    /// Broadcast Audio Scan Service - Broadcast Audio Scan Control Point characteristic
    GATT_CHAR_BCAST_AUDIO_SCAN_CP               = GATT_UUID_16_LSB(0x2BC7),
    /// Broadcast Audio Scan Service - Broadcast Receive State characteristic
    GATT_CHAR_BCAST_RX_STATE                    = GATT_UUID_16_LSB(0x2BC8),

    /// Audio Stream Control Service - Sink ASE characteristic
    GATT_CHAR_ASE_SINK                           = GATT_UUID_16_LSB(0x2BC4),
    /// Audio Stream Control Service - Source ASE characteristic
    GATT_CHAR_ASE_SRC                            = GATT_UUID_16_LSB(0x2BC5),
    /// Audio Stream Control Service - ASE Control Point characteristic
    GATT_CHAR_ASE_CP                             = GATT_UUID_16_LSB(0x2BC6),

    /// Coordinated Set Identification Service - Set Identity Resolving Key characteristic
    GATT_CHAR_CSIS_SIRK                         = GATT_UUID_16_LSB(0x2B84),
    /// Coordinated Set Identification Service - Size characteristic
    GATT_CHAR_CSIS_SIZE                         = GATT_UUID_16_LSB(0x2B85),
    /// Coordinated Set Identification Service - Lock characteristic
    GATT_CHAR_CSIS_LOCK                         = GATT_UUID_16_LSB(0x2B86),
    /// Coordinated Set Identification Service - Rank characteristic
    GATT_CHAR_CSIS_RANK                         = GATT_UUID_16_LSB(0x2B87),

    /// Coordinated Set Identification Service
    GATT_SVC_COORD_SET_IDENTIFICATION           = GATT_UUID_16_LSB(0x1846),
};

void bes_ble_gatt_bearer_mtu_set(uint8_t conidx, uint8_t bearer_lid, uint16_t mtu);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_DP_COMMON_H__ */

