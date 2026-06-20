/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include "adapter_service.h"
#include "bes_os.h"
#include "besbt.h"
#include "cobuf.h"
#include "co_ppbuff.h"
#include "co_timer.h"
#include "gap_service.h"
#include "gatt_service.h"

#ifdef __cplusplus
extern "C" {
#endif

#define co_strncat  strncat
#define co_strcat   strcat
#define co_memcpy   memcpy
#define co_memcmp   memcmp
#define co_strncmp  strncmp
#define co_strncpy  strncpy
#define co_strcpy   strcpy
#define co_memset   memset
#define co_strlen   strlen

#define bdaddr_equal(addr1, addr2) \
            (co_memcmp((const void *)(addr1),(const void *)(addr2),6) == \
                        0 ? TRUE : FALSE)

#define bdaddr_set(dest, src) \
    do { \
        memcpy((void *)(dest),(void *)(src),6); \
    } while (0);

static inline void bdaddr_cpy(struct bdaddr_t *dst, const struct bdaddr_t *src) {
    co_memcpy(dst, src, sizeof(struct bdaddr_t));
}

#define  __set_mask(var, mask)   {var |= mask;}
#define  __test_mask(var, mask)   (var & mask)

#define BTM_NAME_MAX_LEN 248  /*include '\0'*/

U16 le_to_host16(const U8 * ptr);

U16 be_to_host16(const U8* ptr);

U32 be_to_host32(const U8* ptr);

void store_le16(U8 *buff, U16 le_value);

void store_le32(U8 *buff, U32 le_value);

void store_be16(U8 *buff, U16 be_value);

void store_be32(U8 *buff, U32 be_value);

#define BTIF_CTX_INIT(buff) \
    POSSIBLY_UNUSED unsigned int __offset = 2; \
    POSSIBLY_UNUSED unsigned char *__buff = buff;

#define BTIF_CTX_STR_BUF(buff,len) \
    memcpy(__buff+__offset, buff, len); \
    __offset += len;

#define BTIF_CTX_LDR_BUF(buff,len) \
    memcpy(buff, __buff+__offset, len); \
    __offset += len;

#define BTIF_CTX_STR_VAL8(v) \
    __buff[__offset] = v&0xFF; \
    __offset += 1;

#define BTIF_CTX_LDR_VAL8(v) \
    v = __buff[__offset]; \
    __offset += 1;

#define BTIF_CTX_STR_VAL16(v) \
    __buff[__offset] = v&0xFF; \
    __buff[__offset+1] = (v>>8)&0xFF; \
    __offset += 2;

#define BTIF_CTX_LDR_VAL16(v) \
    v = __buff[__offset]; \
    v |= __buff[__offset+1]<<8; \
    __offset += 2;

#define BTIF_CTX_STR_VAL32(v) \
    __buff[__offset] = v&0xFF; \
    __buff[__offset+1] = (v>>8)&0xFF; \
    __buff[__offset+2] = (v>>16)&0xFF; \
    __buff[__offset+3] = (v>>24)&0xFF; \
    __offset += 4;

#define BTIF_CTX_LDR_VAL32(v) \
    v = __buff[__offset]; \
    v |= __buff[__offset+1]<<8; \
    v |= __buff[__offset+2]<<16; \
    v |= __buff[__offset+3]<<24; \
    __offset += 4;

#define BTIF_CTX_GET_BUF_CURR() __buff

#define BTIF_CTX_GET_BUF_HEAD() __buff

#define BTIF_CTX_GET_OFFSET() __offset

#define BTIF_CTX_GET_DATA_LEN() (__buff[0] | __buff[1]<<8)

#define BTIF_CTX_GET_TOTAL_LEN() (BTIF_CTX_GET_DATA_LEN()+2)

#define BTIF_CTX_SAVE_UPDATE_DATA_LEN() \
   __buff[0] = (__offset-2)&0xFF; \
   __buff[1] = ((__offset-2)>>8)&0xFF;

struct btif_ctx_content {
    unsigned char *buff;
    unsigned int buff_len;
};

#define CTX_INIT(buff) \
    POSSIBLY_UNUSED unsigned int __offset = 2; \
    POSSIBLY_UNUSED unsigned char *__buff = buff;

#define CTX_STR_BUF(buff,len) \
    memcpy(__buff+__offset, buff, len); \
    __offset += len;

#define CTX_LDR_BUF(buff,len) \
    memcpy(buff, __buff+__offset, len); \
    __offset += len;

#define CTX_STR_VAL8(v) \
    __buff[__offset] = (v)&0xFF; \
    __offset += 1;

#define CTX_LDR_VAL8(v) \
    v = __buff[__offset]; \
    __offset += 1;

#define CTX_STR_VAL16(v) \
    __buff[__offset] = (v)&0xFF; \
    __buff[__offset+1] = ((v)>>8)&0xFF; \
    __offset += 2;

#define CTX_LDR_VAL16(v) \
    v = __buff[__offset]; \
    v |= __buff[__offset+1]<<8; \
    __offset += 2;

#define CTX_STR_VAL32(v) \
    __buff[__offset] = (v)&0xFF; \
    __buff[__offset+1] = ((v)>>8)&0xFF; \
    __buff[__offset+2] = ((v)>>16)&0xFF; \
    __buff[__offset+3] = ((v)>>24)&0xFF; \
    __offset += 4;

#define CTX_LDR_VAL32(v) \
    v = __buff[__offset]; \
    v |= __buff[__offset+1]<<8; \
    v |= __buff[__offset+2]<<16; \
    v |= __buff[__offset+3]<<24; \
    __offset += 4;

#define CTX_GET_BUF_CURR() __buff

#define CTX_GET_BUF_HEAD() __buff

#define CTX_GET_OFFSET() __offset

#define CTX_ADD_OFFSET(n) __offset += (n);

#define CTX_GET_DATA_LEN() (__buff[0] | __buff[1]<<8)

#define CTX_GET_TOTAL_LEN() (CTX_GET_DATA_LEN()+2)

#define CTX_SAVE_UPDATE_DATA_LEN() \
   __buff[0] = (__offset-2)&0xFF; \
   __buff[1] = ((__offset-2)>>8)&0xFF;

struct ctx_content {
    uint8 *buff;
    uint32 buff_len;
};

enum data_link_type {
    DL_TYPE_VALUE_U8 = 0,
    DL_TYPE_VALUE_U16,
    DL_TYPE_VALUE_U32,
    DL_TYPE_VALUE_S8,
    DL_TYPE_VALUE_S16,
    DL_TYPE_VALUE_S32,
    DL_TYPE_VALUE_FLOAT,
    DL_TYPE_VALUE_DOUBLE,
    DL_TYPE_BUFFER,
};

struct data_link {
    struct list_node node;
    enum data_link_type type;
    uint32 len;
    union {
        unsigned char _u8;
        unsigned short _u16;
        unsigned int _u32;
        unsigned char _s8;
        unsigned short _s16;
        unsigned int _s32;
        //float _float;
        //double _double;
        uint8 *_ptr;
    } data;
};

#define link_to_tail(data,head) \
    colist_addto_tail(data,head)

#define iter_data_link(head,node) \
    colist_iterate_entry(node,struct data_link,&head->node,node)

#define data_link_total_len(head,p) \
    { \
        uint32 _total_len = 0; \
        struct data_link *node = NULL; \
        iter_data_link(head, node) { \
            _total_len += node->len; \
        }  \
        *p = _total_len; \
    }

#define BLUETOOTH_20 1

/* Packet Type Flags     HCI:4.5.14                                                     */
#define PACKET_TYPE_DM1_FLAG    0x0008
#define PACKET_TYPE_DH1_FLAG    0x0010
#define PACKET_TYPE_DM3_FLAG    0x0400
#define PACKET_TYPE_DH3_FLAG    0x0800
#define PACKET_TYPE_DM5_FLAG    0x4000
#define PACKET_TYPE_DH5_FLAG    0x8000

#define PACKET_TYPE_EV3_FLAG    0x0001
#define PACKET_TYPE_EV4_FLAG    0x0002
#define PACKET_TYPE_EV5_FLAG    0x0004

#define PACKET_TYPE_HV1_FLAG    0x0020
#define PACKET_TYPE_HV2_FLAG    0x0040
#define PACKET_TYPE_HV3_FLAG    0x0080

#ifdef BLUETOOTH_20//used in change packet type
#define PACKET_TYPE_2_DH1_FLAG    0x0002
#define PACKET_TYPE_3_DH1_FLAG    0x0004
#define PACKET_TYPE_2_DH3_FLAG    0x0100
#define PACKET_TYPE_3_DH3_FLAG    0x0200
#define PACKET_TYPE_2_DH5_FLAG    0x1000
#define PACKET_TYPE_3_DH5_FLAG    0x2000
#define EDR_2M_PACKET_TYPE_MASK (PACKET_TYPE_2_DH1_FLAG|\
	PACKET_TYPE_2_DH3_FLAG|\
	PACKET_TYPE_2_DH5_FLAG)


#define EDR_3M_PACKET_TYPE_MASK (PACKET_TYPE_3_DH1_FLAG|\
	PACKET_TYPE_3_DH3_FLAG|\
	PACKET_TYPE_3_DH5_FLAG)
#define ALL_ACL_PACKET_MASK  (PACKET_TYPE_DH5_FLAG|\
							 PACKET_TYPE_DM5_FLAG|\
							 PACKET_TYPE_DH3_FLAG|\
							 PACKET_TYPE_DM3_FLAG|\
							 PACKET_TYPE_DH1_FLAG|\
							 PACKET_TYPE_DM1_FLAG|\
							 EDR_2M_PACKET_TYPE_MASK |\
							 EDR_3M_PACKET_TYPE_MASK)

#define ACL_1M_PACKET_MASK  (PACKET_TYPE_DH5_FLAG|\
							 PACKET_TYPE_DM5_FLAG|\
							 PACKET_TYPE_DH3_FLAG|\
							 PACKET_TYPE_DM3_FLAG|\
							 PACKET_TYPE_DH1_FLAG|\
							 PACKET_TYPE_DM1_FLAG)

#define ACL_1SLOT_PACKET_MASK  (PACKET_TYPE_DH1_FLAG|\
                                PACKET_TYPE_DM1_FLAG|\
                                PACKET_TYPE_2_DH1_FLAG|\
                                PACKET_TYPE_3_DH1_FLAG)

#define PACKET_TYPE_2_EV3_FLAG    0x0040
#define PACKET_TYPE_3_EV3_FLAG    0x0080
#define PACKET_TYPE_2_EV5_FLAG    0x0100
#define PACKET_TYPE_3_EV5_FLAG    0x0200
#endif
/* add for spec 1.2  HCI 7.1.26 */
#define PACKET_TYPE_HV1			0x0001
#define PACKET_TYPE_HV2			0x0002
#define PACKET_TYPE_HV3			0x0004
#define PACKET_TYPE_EV3			0x0008
#define PACKET_TYPE_EV4			0x0010
#define PACKET_TYPE_EV5			0x0020
#define PACKET_TYPE_NO_2_EV3    0x0040
#define PACKET_TYPE_NO_3_EV3    0x0080
#define PACKET_TYPE_NO_2_EV5    0x0100
#define PACKET_TYPE_NO_3_EV5    0x0200

#ifdef BLUETOOTH_20//used in set up synchronous link
#define PACKET_TYPE_2_EV3			0x0040
#define PACKET_TYPE_3_EV3			0x0080
#define PACKET_TYPE_2_EV5			0x0100
#define PACKET_TYPE_3_EV5			0x0200
#endif

/* Packet Boundary Flag   HCI:4.4.3                                                     */
#define CONT_HL_FRAG            0x01
#define FIRST_HL_FRAG           0x02
#define LMP_FRAG                0x03

/* Packet Broadcast Flag   HCI:4.4.3                                                    */
#define POINT_TO_POINT          0x00
#define ACTIVE_BCST             0x01
#define PICONET_BCST            0x02

/* Park mode defines LMP:3.17                                                           */
#define MACCESS_MSK             0x0F
#define ACCSCHEM_MSK            0xF0

/* Device Supported Features defines LMP:5.1.1                                          */
#define THREE_SLOT_FEATURE      0x01
#define FIVE_SLOT_FEATURE       0x02
#define ENCRYPT_FEATURE         0x04
#define SLOTOFF_FEATURE         0x08
#define TIMEACC_FEATURE         0x10
#define SWITCH_FEATURE          0x20
#define HOLD_FEATURE            0x40
#define SNIFF_FEATURE           0x80

#define PARK_FEATURE            0x01
#define RSSI_FEATURE            0x02
#define QUALITY_FEATURE         0x04
#define SCO_FEATURE             0x08
#define HV2_FEATURE             0x10
#define HV3_FEATURE             0x20
#define ULAW_FEATURE            0x40
#define ALAW_FEATURE            0x80

//#define EV3_FEATURE			0x20


#define CVSD_FEATURE            0x01
#define PAGING_FEATURE          0x02
#define POWER_FEATURE           0x04
#define TRANSPARENT_FEATURE		0x08
#define FLOWLEAST_FEATURE		0x10
#define FLOWMID_FEATURE			0x20
#define FLOWMOST_FEATURE		0x40
#define BCENCRYPT_FEATURE		0x80

#define ACL_2M_FEATURE           0x02
#define ACL_3M_FEATURE           0x04

#define ENHANCEINQSCAN_FEATURE	0x08
#define INTERINQSCAN_FEATURE	0x10
#define INTERPGSCAN_FEATURE  	0x20
#define RSSIINQ_FEATURE		 	0x40
#define EV3_FEATURE			    0x80

#define EV4_FEATURE				0x01
#define EV5_FEATURE				0x02
#define AFHCAPSL_FEATURE		0x08
#define AFHCLASSSL_FEATURE		0x10
#define ACL_3SLOTS_FEATURE      0x80


#define ACL_5SLOTS_FEATURE      0x01
#define SNIFF_SUBRATING_FEATURE	0x02
#define PAUSE_ENCRYPTION_FEATURE    0x04
#define AFHCAPMS_FEATURE		0x08
#define AFHCLASSMS_FEATURE		0x10
#ifdef BLUETOOTH_20
#define ESCO_2M_FEATURE         0x20
#define ESCO_3M_FEATURE         0x40
#define ESCO_3SLOTS_FEATURE     0x80
#else
#define ESCO_2M_FEATURE         0x00
#define ESCO_3M_FEATURE         0x00
#define ESCO_3SLOTS_FEATURE     0x00
#endif


/* Packet and buffer sizes. These sizes do not include payload header (except for FHS   */
/* packet where there is no payload header) since payload header is written or read by  */
/* the BOOST in a different control structure part (TX/RXPHDR)                          */
#define FHS_PACKET_SIZE         18
#define DM1_PACKET_SIZE         17
#define DH1_PACKET_SIZE         27
#define DV_ACL_PACKET_SIZE      9
#define DM3_PACKET_SIZE         121
#define DH3_PACKET_SIZE         183
#define DM5_PACKET_SIZE         224
#define DH5_PACKET_SIZE         339
#define AUX1_PACKET_SIZE        29

#define HV1_PACKET_SIZE         10
#define HV2_PACKET_SIZE         20
#define HV3_PACKET_SIZE         30

//add by yfwen for BT1.2 eSco packet
#define EV3_PACKET_SIZE         30
#define EV4_PACKET_SIZE         120
#define EV5_PACKET_SIZE         180

#ifdef BLUETOOTH_20
#define DH1_2_PACKET_SIZE	  54
#define DH1_3_PACKET_SIZE	  83
#define DH3_2_PACKET_SIZE	  367
#define DH3_3_PACKET_SIZE	  552
#define DH5_2_PACKET_SIZE	  679
#define DH5_3_PACKET_SIZE	  1021

#define EV3_2_PACKET_SIZE         60
#define EV3_3_PACKET_SIZE         90
#define EV5_2_PACKET_SIZE         360
#define EV5_3_PACKET_SIZE         540
#endif


/* SCO Packet coding LMP:5.1                                                            */
#define SCO_PACKET_HV1          0x00
#define SCO_PACKET_HV2          0x01
#define SCO_PACKET_HV3          0x02
/* ESCO Packet type coding LMP 5 in spec 1.2 page 437 */
#define ESCO_PACKET_EV3          0x07
#define ESCO_PACKET_EV4          0x0C
#define ESCO_PACKET_EV5          0x0D
#define ESCO_PACKET_2EV3			0x26
#define ESCO_PACKET_3EV3          0x37
#define ESCO_PACKET_2EV5          0x2C
#define ESCO_PACKET_3EV5          0x3D

#ifdef BLUETOOTH_20
#define ESCO_PACKET_2_EV3          0x26
#define ESCO_PACKET_3_EV3          0x37
#define ESCO_PACKET_2_EV5          0x2C
#define ESCO_PACKET_3_EV5          0x3D
#endif

#define MAX_EV3_LENGTH    0x001E
#define MAX_EV4_LENGTH    0x0078
#define MAX_EV5_LENGTH    0x00B4
#ifdef BLUETOOTH_20
#define MAX_2_EV3_LENGTH    0x003C
#define MAX_2_EV5_LENGTH    0x0168
#define MAX_3_EV3_LENGTH    0x005A
#define MAX_3_EV5_LENGTH    0x021C
#endif

/* Tsco (ScoInterval) BaseBand:4.4.2.1                                                 */
#define TSCO_HV1                2
#define TSCO_HV2                4
#define TSCO_HV3                6

//add by yfwen for BT1.2 eSco packet
#define TESCO_EV3                6 //
#define TESCO_EV4                16
#define TESCO_EV5                16

/* Inquiry train repetition length , Baseband :Table 10.4                               */
/*      - 256 repetitions if no SCO                                                     */
/*      - 512 repetitions if 1 SCO                                                      */
/*      - 768 repetitions if 2 SCO                                                      */
#define INQ_TRAIN_LENGTH_NO_SCO 256
#define INQ_TRAIN_LENGTH_1_SCO  512
#define INQ_TRAIN_LENGTH_2_SCO  768

/* Counter for train length, Npage (N*16 slots) depends on the slave page scan mode and */
/* the number of active SCO:                                                            */
/*    | SR mode |  no SCO  |  one SCO  |  two SCO |                                     */
/*    |   R0    |   >=1    |    >=2    |    >=3   |                                     */
/*    |   R1    |   >=128  |    >=256  |    >=384 |                                     */
/*    |   R2    |   >=256  |    >=512  |    >=768 |                                     */
#define PAGE_TRAIN_LENGTH_R0    1
#define PAGE_TRAIN_LENGTH_R1    128
#define PAGE_TRAIN_LENGTH_R2    256

/* Baseband timeout default value, Baseband timers: 1.1                                 */
#define PAGE_RESP_TO_DEF        8
#define INQ_RESP_TO_DEF         128
#define NEW_CONNECTION_TO_DEF   32

/* Voice mute pattern defines                                                           */
#define MU_LAW_MUTE             0xFF
#define ALAW_CVSD_MUTE          0x55

/* Air Mode  LMP:5.1                                                                    */
#define MU_LAW_MODE             0
#define A_LAW_MODE              1
#define CVSD_MODE               2
#define TRANSPARENT_MODE		3

/* Timing Control Flags  LMP:5.1                                                        */
#define TIM_CHANGE_FLAG         0x01
#define INIT2_FLAG              0x02
#define ACCESS_WIN_FLAG         0x04

/* Data Rate defines    LMP:5.1                                                         */
#define FEC_RATE_MSK            0x01
#define USE_FEC_RATE            0x00
#define NO_FEC_RATE             0x01
#define PREF_PACK_MSK           0x06
#define NO_PREF_PACK_SIZE       0x00
#define USE_1_SLOT_PACKET       0x02
#define USE_3_SLOT_PACKET       0x04
#define USE_5_SLOT_PACKET       0x06
#define USE_SLOT_MASK           0x06

#define USE_DM1_PACKET          0x00
#define USE_2M_PACKET           0x08
#define USE_3M_PACKET           0x10
#define EDR_NO_PREF_PACK_SIZE   0x00
#define EDR_USE_1_SLOT_PACKET   0x20
#define EDR_USE_3_SLOT_PACKET   0x40
#define EDR_USE_5_SLOT_PACKET   0x60
#define EDR_USE_SLOT_MASK       0x60

/* Bluetooth bandwidth               */

#define BT_8000_BANDWIDTH      0x00001F40
#define NOT_CARE_WIDTH         0xFFFFFFFF

/* Voice setting HCI:4.7.29 & 4.7.30                                                    */
#define INPUT_COD_LIN           0x0000
#define INPUT_COD_MULAW         0x0100
#define INPUT_COD_ALAW          0x0200
#define INPUT_COD_MSK           0x0300
#define INPUT_DATA_1COMP        0x0000
#define INPUT_DATA_2COMP        0x0040
#define INPUT_DATA_SMAG         0x0080
#define INPUT_DATAFORM_MSK      0x00C0
#define INPUT_SAMP_8BIT         0x0000
#define INPUT_SAMP_16BIT        0x0020
#define INPUT_SAMPSIZE_MSK      0x0020
#define LIN_PCM_BIT_POS_MSK     0x001C
#define AIR_COD_CVSD            0x0000
#define AIR_COD_MULAW           0x0001
#define AIR_COD_ALAW            0x0002
#define AIR_TRANSPARENT_DATA    0x0003
#define AIR_COD_MSK             0x0003

#ifdef __cplusplus
}
#endif                          /*  */
#endif /*__BLUETOOTH_H__*/
