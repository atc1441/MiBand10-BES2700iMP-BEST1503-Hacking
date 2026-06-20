#include <stdbool.h>
#include <string.h>
#include "plc_utils.h"

#ifndef VQE_SIMULATE
#include "bt_drv.h"
#include "hal_trace.h"
#else
#define MSBC_MUTE_PATTERN (0x55)
#endif

#ifdef VQE_SIMULATE
#include <stdio.h>
#define ASSERT(cond, str, ...)      { if (!(cond)) { fprintf(stderr, str, ##__VA_ARGS__); while(1); } }
#define TRACE(num, str, ...)             do { fprintf(stdout, str, ##__VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#endif

#define MSBC_PKTSIZE (60)

/*
 * msbc frame's last byte is a padding byte, we assume it is zero,
 * but it is not always true.
 * Do not check this by default
 */
//#define ENABLE_PAD_CHECK

/*
 * if msbc frame is filled by 10+ samples in the trail, crc maybe not detect this
 * satuation.
 * Do not check this by default
 */
//#define ENABLE_TRAILING_ZERO_CHECK

/*
 *
 */
#define ENABLE_BLE_CONFLICT_CHECK

#define ENABLE_CRC_CHECK

/* check msbc sequence number */
#define ENABLE_SEQ_CHECK

#ifdef ENABLE_CRC_CHECK

#if defined(ROM_UTILS_ON) && defined(CHIP_BEST1306)
#include "hal_chipid.h"
extern const uint8_t sbc_crc_tbl_v1[256];
extern const uint8_t sbc_crc_tbl_v2[256];
extern const uint8_t sbc_crc_tbl_v3[256];
static uint8_t* sbc_crc_tbl = NULL;
static void sbc_crc_tbl_init()
{
    if(sbc_crc_tbl == NULL){
        int metal_id = hal_get_chip_metal_id();
        if(metal_id == HAL_CHIP_METAL_ID_0){
            sbc_crc_tbl = (uint8_t *)sbc_crc_tbl_v1;
        } else if(metal_id == HAL_CHIP_METAL_ID_1) {
            sbc_crc_tbl = (uint8_t *)sbc_crc_tbl_v2;
        } else if(metal_id == HAL_CHIP_METAL_ID_2) {
            sbc_crc_tbl = (uint8_t *)sbc_crc_tbl_v3;
        }
    }
}
#else
extern const uint8_t sbc_crc_tbl[256];
static void sbc_crc_tbl_init()
{
}
#endif
#endif

// #define MONITOR_PLC_RESTART_STREAM
#define MONITOR_PLC_DUMP_PACKET

static uint32_t g_monitor_frame_cnt = 0;

#ifdef MONITOR_PLC_RESTART_STREAM
static uint32_t g_monitor_restart_stream_plc_cnt = 0;
#endif
#ifdef MONITOR_PLC_DUMP_PACKET
static uint32_t g_monitor_dump_plc_cnt = 0;
static uint32_t g_monitor_dump_frame_cnt = 0;
#endif

int32_t plc_monitor_init(void)
{
    g_monitor_frame_cnt = 0;

#ifdef MONITOR_PLC_RESTART_STREAM
    g_monitor_restart_stream_plc_cnt = 0;
#endif
#ifdef MONITOR_PLC_DUMP_PACKET
    g_monitor_dump_plc_cnt = 0;
    g_monitor_dump_frame_cnt = 0;
#endif

    return 0;
}

int32_t plc_monitor_process(plc_type_t plc_type)
{
    g_monitor_frame_cnt++;

    if (plc_type) {
#ifdef MONITOR_PLC_RESTART_STREAM
        g_monitor_restart_stream_plc_cnt++;
#endif
#ifdef MONITOR_PLC_DUMP_PACKET
        g_monitor_dump_plc_cnt++;
#endif
    }

#ifdef MONITOR_PLC_RESTART_STREAM
    // Monitor 3s, check 50% PLC
    if (g_monitor_frame_cnt % 400 == 0) {
        if (g_monitor_restart_stream_plc_cnt >= 200) {
            // Try restart bt sco stream
        }
        g_monitor_restart_stream_plc_cnt = 0;
    }
#endif
#ifdef MONITOR_PLC_DUMP_PACKET
    // Monitor 300ms, check 50% PLC, dump 6 frame packet
    if (g_monitor_frame_cnt % 40 == 0) {
        if (g_monitor_dump_plc_cnt >= 20) {

            g_monitor_dump_frame_cnt = 6;
        }
        g_monitor_dump_plc_cnt = 0;
    }
#endif

    return 0;
}

int32_t plc_monitor_dump_data(uint8_t *buf, uint32_t len)
{
#ifdef MONITOR_PLC_DUMP_PACKET
    if (g_monitor_dump_frame_cnt > 0) {
        DUMP16("%04x,", buf, len / 2);
        TRACE(0, " END: %d", g_monitor_dump_frame_cnt);
        g_monitor_dump_frame_cnt--;
    }
#endif

    return 0;
}

static int sco_parse_synchronization_header(uint8_t *buf, uint8_t *sn)
{
    uint8_t sn1, sn2;
#ifdef ENABLE_CRC_CHECK
    sbc_crc_tbl_init();
    uint8_t fcs = 0x0F;
    uint8_t crc = 0;
    uint8_t i, sb, bit, shift;
    uint8_t ind = 6, bitOffset = 24;
#endif
    *sn = 0xff;
#if defined(MSBC_SYNC_HACKER)
    if (((buf[0] != 0x01) && (buf[0] != 0x00)) ||
        ((buf[1] & 0x0f) != 0x08) ||
        (buf[2] != 0xad)) {
        return -1;
    }
#else
    if ((buf[0] != 0x01) ||
        ((buf[1] & 0x0f) != 0x08) ||
        (buf[2] != 0xad)) {
        return -1;
    }
#endif

    sn1 = (buf[1] & 0x30) >> 4;
    sn2 = (buf[1] & 0xc0) >> 6;
    if ((sn1 != 0) && (sn1 != 0x3)) {
        return -2;
    }
    if ((sn2 != 0) && (sn2 != 0x3)) {
        return -3;
    }

#ifdef ENABLE_CRC_CHECK
    fcs = sbc_crc_tbl[fcs ^ buf[3]];
    if (buf[3] != 0x00)
        return -4;
    fcs = sbc_crc_tbl[fcs ^ buf[4]];
    if (buf[4] != 0x00)
        return -4;
    crc = buf[5];
    for (sb = 0; sb < 8; sb++) {
        if (bitOffset % 8) {
            /* Sum the whole byte */
            fcs = sbc_crc_tbl[fcs ^ buf[ind]];
            ind = ind + 1;
        }
        else {
            if (sb == 7) {
                /* Sum the next 4 bits */

                /* Just sum the most significant 4 bits */
                shift = 7;
                for (i = 0; i < 4; i++) {
                    bit = (uint8_t)((0x01 & (buf[ind] >> shift--)) ^ (fcs >> 7));
                    if (bit) {
                        fcs = (uint8_t)(((fcs << 1) | bit) ^ 0x1C);
                    }
                    else {
                        fcs = (uint8_t)((fcs << 1));
                    }
                }
            }
        }

        bitOffset += 4;
    }
    //TRACE(2,"msbc crc:%d fcs:%d", crc,fcs);
    if (crc != fcs)
        return -4;
#endif

    *sn = (sn1 & 0x01) | (sn2 & 0x02);

#ifdef ENABLE_PAD_CHECK
    // when pad error detected, we should return sn
    if (buf[MSBC_PKTSIZE - 1] != 0x0) {
        return -5;
    }
#endif

    return 0;
}

#ifdef ENABLE_BLE_CONFLICT_CHECK
static bool memcmp_U8(uint8_t *x, uint8_t *y, uint16_t size)
{
    for (int i = 0; i < size; i++) {
        if (x[i] != y[i])
            return true;
    }

    return false;
}

// when signal is mute, msbc data remains the same except seq num. We should check history flag,
// otherwise a single conflict may be detected twice
static bool update_ble_sco_conflict(PacketLossState* st, uint8_t *last_pkt, uint8_t *pkt)
{
    // do not check padding byte as it maybe useless when msbc_offset is 1
    bool ret = (st->prev_ble_sco_conflict_flag[1] == false && memcmp_U8(last_pkt, pkt, MSBC_PKTSIZE - 1) == false);

    memcpy(&last_pkt[0], &last_pkt[MSBC_PKTSIZE], MSBC_PKTSIZE);
    memcpy(&last_pkt[MSBC_PKTSIZE], pkt, MSBC_PKTSIZE);

    return ret;
}

static bool check_ble_sco_conflict(PacketLossState* st, bool ret)
{
    st->prev_ble_sco_conflict_flag[1] = st->prev_ble_sco_conflict_flag[0];
    st->prev_ble_sco_conflict_flag[0] = ret;

    return ret;
}
#endif

static bool msbc_check_controller_mute_pattern(uint8_t *pkt, uint8_t pattern)
{
    // do not check padding byte as it maybe useless when msbc_offset is 1
    for (int i = 0; i < MSBC_PKTSIZE - 1; i++)
        if (pkt[i] != pattern)
            return false;

    return true;
}

/*
 * Some PC will send msbc packet by an offset, for example,
 * [ 0x67, 0x13, 0x2E, 0x39, 0xD3, 0x21, 0x9A, 0x80, 0x1B, 0xB3, 0x36, 0xF2, 0xCA, 0x59, 0xEC,
 *   0x24, 0xD4, 0x00, 0x01, 0xC8, 0xAD, 0x00, 0x00, 0x83, 0x98, 0x54, 0x10, 0x00, 0x5F, 0x7B,
 *   0x89, 0x69, 0x60, 0x55, 0x57, 0x88, 0x04, 0x55, 0x35, 0xED, 0x21, 0xB5, 0x83, 0x79, 0x60,
 *   0x9F, 0x95, 0x51, 0x17, 0xB1, 0x55, 0x76, 0x0E, 0x25, 0x84, 0x7E, 0x58, 0x5E, 0x5D, 0xB1 ]
 * When a packet is lost, it will affect two consequent packet, we need to find them all.
 */
static bool msbc_check_transmitter_offset(uint8_t *pkt, uint8_t pattern, uint32_t index)
{
    if (index == 0)
        return false;

    // padding is 0x00 by default
    if (index == 1 && pattern == 0x00)
        return false;

    if (pkt[0] == pattern)
    {
        for (int i = 0; i < MSBC_PKTSIZE - index; i++)
            if (pkt[i] != pattern)
                return false;
        // TRACE(0, "front part mute");
    }
    else if (pkt[MSBC_PKTSIZE - 1] == pattern)
    {
        for (int i = MSBC_PKTSIZE - index; i < MSBC_PKTSIZE; i++)
            if (pkt[i] != pattern)
                return false;
        // TRACE(0, "behind part mute");
    }
    else
    {
        return false;
    }

    return true;
}

#ifdef ENABLE_TRAILING_ZERO_CHECK
static int msbc_check_pkt_trailing_zeros(uint8_t *pkt)
{
    int idx = MSBC_PKTSIZE;

    for (int i = MSBC_PKTSIZE - 1; i >= 0; i--) {
        if (pkt[i] != 0) {
            idx = i;
            break;
        }
    }

    return (MSBC_PKTSIZE - 1 - idx);
}
#endif

static uint8_t get_next_sequence_num(uint8_t seq_num)
{
    return (seq_num + 1 == 4) ? 0 : (seq_num + 1);
}

void packet_loss_detection_init(PacketLossState *st)
{
    st->last_seq_num = 0xff;

    memset(st->last_pkt, 0, sizeof(st->last_pkt));
    memset(st->prev_ble_sco_conflict_flag, 0, sizeof(st->prev_ble_sco_conflict_flag));
    memset(st->hist, 0, sizeof(st->hist));
    plc_monitor_init();
}

plc_type_t packet_loss_detection_process(PacketLossState *st, uint8_t *sbc_buf, uint32_t index)
{
    plc_type_t plc_type = PLC_TYPE_PASS;

#ifdef ENABLE_BLE_CONFLICT_CHECK
    bool ble_sco_conflict = update_ble_sco_conflict(st, st->last_pkt, sbc_buf);
#endif

    uint8_t seq_num;
    if (msbc_check_controller_mute_pattern(sbc_buf, MSBC_MUTE_PATTERN) == true) {
        plc_type = PLC_TYPE_CONTROLLER_MUTE;
        st->last_seq_num = 0xff;
    }
    else if (msbc_check_transmitter_offset(sbc_buf, MSBC_MUTE_PATTERN, index) == true) {
        plc_type = PLC_TYPE_TRANSMITTER_OFFSET;
        st->last_seq_num = 0xff;
    }
#ifdef ENABLE_BLE_CONFLICT_CHECK
    else if (check_ble_sco_conflict(st, ble_sco_conflict) == true) {
        plc_type = PLC_TYPE_BLE_CONFLICT;
        st->last_seq_num = 0xff;
    }
#endif
    else {
        int err = sco_parse_synchronization_header(sbc_buf, &seq_num);
        if (err < 0 && err >= -3) {
            plc_type = PLC_TYPE_HEADER_ERROR;
            st->last_seq_num = 0xff;
        }
#ifdef ENABLE_CRC_CHECK
        else if (err == -4) {
            plc_type = PLC_TYPE_CRC_ERROR;
            st->last_seq_num = 0xff;
        }
#endif
#ifdef ENABLE_PAD_CHECK
        else if (err == -5) {
            plc_type = PLC_TYPE_PAD_ERROR;
            st->last_seq_num = seq_num;
        }
#endif
#ifdef ENABLE_TRAILING_ZERO_CHECK
        else if (msbc_check_pkt_trailing_zeros(sbc_buf) > 10) {
            plc_type = PLC_TYPE_DATA_MISSING;
            st->last_seq_num = 0xff;
        }
#endif
        else {
#ifdef ENABLE_SEQ_CHECK
            if (st->last_seq_num == 0xff) {
                if (seq_num == 0xff) {
                    plc_type = PLC_TYPE_SEQUENCE_DISCONTINUE;
                }
                else {
                    plc_type = PLC_TYPE_PASS;
                }
                st->last_seq_num = seq_num;
            }
            else {
                if (seq_num == 0xff) {
                    st->last_seq_num = 0xff;
                    plc_type = PLC_TYPE_SEQUENCE_DISCONTINUE;
                }
                else if (seq_num == get_next_sequence_num(st->last_seq_num)) {
                    st->last_seq_num = seq_num;
                    plc_type = PLC_TYPE_PASS;
                }
                else {
                    st->last_seq_num = 0xff;
                    plc_type = PLC_TYPE_SEQUENCE_DISCONTINUE;
                }
            }
#else
            plc_type = PLC_TYPE_PASS;
#endif
        }
    }

    packet_loss_detection_update_histogram(st, plc_type);
    plc_monitor_process(plc_type);

    return plc_type;
}

void packet_loss_detection_update_histogram(PacketLossState *st, plc_type_t plc_type)
{
    if (plc_type < 0 || plc_type >= PLC_TYPE_NUM) {
        TRACE(2,"[%s] plc type %d is invalid", __FUNCTION__, plc_type);
        return;
    }

    // The packet is detected as PLC_TYPE_PASS, but causes a decoder error.
    if (plc_type == PLC_TYPE_DECODER_ERROR) {
        st->hist[0] -= 1;
    }

    st->hist[plc_type] += 1;
}

void packet_loss_detection_report(PacketLossState *st)
{
    uint32_t packet_loss_num = 0;

    for (uint8_t i = 1; i < PLC_TYPE_NUM; i++) {
        TRACE(3,"[%s] plc type %d occurs %d times", __FUNCTION__, i, st->hist[i]);
        packet_loss_num += st->hist[i];
    }

    uint32_t packet_total_num = st->hist[0] + packet_loss_num;
    TRACE(4,"[%s] packet loss percent %d/10000(%d/%d)", __FUNCTION__,
          (int32_t)(10000.f * packet_loss_num/ packet_total_num), packet_loss_num, packet_total_num);
}