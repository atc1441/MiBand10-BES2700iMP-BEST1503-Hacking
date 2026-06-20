#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "bt_sco_codec.h"
#include "bt_drv.h"
#include "speech_cfg.h"
// Customer Codec Header File
#include "sbc_api.h"
#include "sbc_error_code.h"

/**
 * NOTE:
 *  1. This example shows MSBC encoder, decoder and PLC
 **/

#define CODEC_SAMPLE_RATE           (16000)
#define CODEC_PACKET_SIZE           (57)
#define SCO_PACKET_SIZE             (60)
#define CODEC_PCM_FRAME_SIZE        (240)

#define SCO_PACKET_HEADER               (0x01)
#define SCO_PACKET_SEQ_NUM              (4)
#define SCO_PACKET_SEQ_INDEX_INVALID    (0xff)
static const uint8_t g_sco_packet_seq[SCO_PACKET_SEQ_NUM] = {0x08, 0x38, 0xc8, 0xf8};
#define SCO_PACKET_PADDING              (0x00)

typedef struct {
    uint8_t     header;
    uint8_t     seq;
    uint8_t     payload[CODEC_PACKET_SIZE];
    uint8_t     padding;
} sco_xxxx_packet_t;

typedef struct {
    uint32_t    total;
    uint32_t    bt_mute;
    uint32_t    parser;
    uint32_t    seq;
    uint32_t    repair;
} xxxx_plc_histogram_t;

static uint8_t g_encoder_seq_index = 0;
static uint8_t g_decoder_seq_index = SCO_PACKET_SEQ_INDEX_INVALID;

static xxxx_plc_histogram_t g_xxxx_plc_histogram;

// Encoder and Decoder State
static sbc_encoder_t *g_enc_state = NULL;
static sbc_decoder_t *g_dec_state = NULL;
static float dec_eq_band_gain[8]= {0,0,0,0,0,0,0,0};

// PLC State
#include "lpc_plc_api.h"
static LpcPlcState *plc_st = NULL;

static uint32_t get_packet_index(uint8_t seq)
{
    for (uint32_t index = 0; index < SCO_PACKET_SEQ_NUM; index++) {
        if (g_sco_packet_seq[index] == seq) {
            return index;
        }
    }

    return SCO_PACKET_SEQ_INDEX_INVALID;
}

static bool check_controller_mute_pattern(uint8_t *pkt, uint8_t pattern)
{
    // do not check padding byte as it maybe useless when msbc_offset is 1
    for (int i = 0; i < SCO_PACKET_SIZE - 1; i++)
        if (pkt[i] != pattern)
            return false;

    return true;
}    

static void generate_xxxx_plc_histogram(void)
{
    uint32_t packet_loss_num =  g_xxxx_plc_histogram.bt_mute + 
                                g_xxxx_plc_histogram.parser + 
                                g_xxxx_plc_histogram.seq;

    TRACE(0, "------------------------ XXXX PLC Histogram ------------------------");
    TRACE(0, "* BT Mute:  %d", g_xxxx_plc_histogram.bt_mute);
    TRACE(0, "* Parser:   %d", g_xxxx_plc_histogram.parser);
    TRACE(0, "* Seq:      %d", g_xxxx_plc_histogram.seq);
    TRACE(0, "* Repair:   %d", g_xxxx_plc_histogram.repair);
    TRACE(0, "* Packet Loss Percent:    %d/10000(%d/%d)", 
        (int32_t)(10000.f * packet_loss_num / g_xxxx_plc_histogram.total),
        packet_loss_num, g_xxxx_plc_histogram.total);
    TRACE(0, "* Repair Packet Percent:  %d/10000(%d/%d)", 
        (int32_t)(10000.f * g_xxxx_plc_histogram.repair / g_xxxx_plc_histogram.total),
        g_xxxx_plc_histogram.repair, g_xxxx_plc_histogram.total);
    TRACE(0, "-------------------------------- End --------------------------------");
}

int32_t bt_sco_codec_xxxx_init(uint32_t sample_rate)
{
    // ASSERT(0, "[%s] Please implement this function! Then remove this ASSERT!", __func__);
    TRACE(0, "[%s] ...", __func__);

    ASSERT(SCO_PACKET_SIZE == sizeof(sco_xxxx_packet_t),
        "[%s] SCO_PACKET_SIZE != sizeof(sco_xxxx_packet_t)", __func__);

    g_encoder_seq_index = 0;
    g_decoder_seq_index = SCO_PACKET_SEQ_INDEX_INVALID;

    memset(&g_xxxx_plc_histogram, 0, sizeof(g_xxxx_plc_histogram));

    g_enc_state = (sbc_encoder_t *)speech_calloc(1, sizeof(sbc_encoder_t));  // 2176 Bytes
    g_dec_state = (sbc_decoder_t *)speech_calloc(1, sizeof(sbc_decoder_t));  // 5376 Bytes

    // Init Encoder
    sbc_stream_info_t info;
    info.flags = SBC_FLAGS_MSBC;
    info.num_channels = 1;
    info.channel_mode = SBC_CHANNEL_MODE_MONO;
    info.bit_pool     = 26;
    info.sample_rate  = SBC_SAMPLERATE_16K;
    info.alloc_method = SBC_ALLOC_METHOD_LOUDNESS;
    info.num_blocks   = MSBC_BLOCKS;
    info.num_subbands = 8;
    sbc_encoder_open(g_enc_state, info);

    // Init Decoder
    sbc_decoder_open(g_dec_state);
    sbc_decoder_set(g_dec_state, SBC_FLAGS, &info.flags);

    for (uint8_t i=0; i<sizeof(dec_eq_band_gain)/sizeof(float); i++) {
        dec_eq_band_gain[i] = 1.0;
    }

    // Init PLC
    plc_st = lpc_plc_create(sample_rate);

    return 0;
}

int32_t bt_sco_codec_xxxx_deinit(void)
{
    // ASSERT(0, "[%s] Please implement this function! Then remove this ASSERT!", __func__);
    TRACE(0, "[%s] ...", __func__);

    generate_xxxx_plc_histogram();

    // Deinit PLC
    lpc_plc_destroy(plc_st);

    // Free Encoder and Decoder State
    speech_free(g_enc_state);
    speech_free(g_dec_state);

    return 0;
}

// xxxx_size = 120, pcm_size = 480
int32_t bt_sco_codec_xxxx_encoder(uint8_t *xxxx_buf, uint32_t xxxx_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // ASSERT(0, "[%s] Please implement this function! Then remove this ASSERT!", __func__);

    ASSERT(xxxx_size / SCO_PACKET_SIZE == pcm_size / CODEC_PCM_FRAME_SIZE,
        "[%s] xxxx_size = %d, pcm_size = %d", __func__, xxxx_size, pcm_size);

    pcm_frame_t pcm_data;
    sbc_frame_t sbc_data;
    uint32_t loop_cnt = xxxx_size / SCO_PACKET_SIZE;
    sco_xxxx_packet_t *sco_packet_ptr = (sco_xxxx_packet_t *)xxxx_buf;

    // TRACE(0, "[%s] xxxx_size = %d, pcm_size = %d", __func__, xxxx_size, pcm_size);

    for (uint32_t i=0; i<loop_cnt; i++) {
        sco_packet_ptr->header  = SCO_PACKET_HEADER;
        sco_packet_ptr->padding = SCO_PACKET_PADDING;
        sco_packet_ptr->seq     = g_sco_packet_seq[g_encoder_seq_index];

        // Encoder
        pcm_data.pcm_data = (int_least16_t *)pcm_buf;
        pcm_data.valid_size = CODEC_PCM_FRAME_SIZE;
        sbc_data.sbc_data = sco_packet_ptr->payload;
        sbc_data.valid_size = 0;
        sbc_data.buffer_size = SCO_PACKET_SIZE;
        sbc_encoder_process_frame(g_enc_state, &pcm_data, &sbc_data);

        sco_packet_ptr++;
        pcm_buf += CODEC_PCM_FRAME_SIZE;
        g_encoder_seq_index = (g_encoder_seq_index + 1) % SCO_PACKET_SEQ_NUM;
    }

    return 0;
}

// xxxx_size = 120, pcm_size = 480
int32_t bt_sco_codec_xxxx_decoder(uint8_t *xxxx_buf, uint32_t xxxx_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // ASSERT(0, "[%s] Please implement this function! Then remove this ASSERT!", __func__);

    ASSERT(xxxx_size / SCO_PACKET_SIZE == pcm_size / CODEC_PCM_FRAME_SIZE,
        "[%s] xxxx_size = %d, pcm_size = %d", __func__, xxxx_size, pcm_size);

    pcm_frame_t dec_pcm_data;

    int16_t plc_flag = false;
    uint32_t loop_cnt = xxxx_size / SCO_PACKET_SIZE;
    sco_xxxx_packet_t *sco_packet_ptr = (sco_xxxx_packet_t *)xxxx_buf;

#if 0
    TRACE(0, "[%s] xxxx_size = %d, pcm_size = %d", __func__, xxxx_size, pcm_size);
    TRACE(0, "Decoder: 0x%2x, 0x%2x, 0x%2x, 0x%2x", xxxx_buf[0],
                                                    xxxx_buf[1],
                                                    xxxx_buf[2],
                                                    xxxx_buf[3]);
#endif

    for (uint32_t i=0; i<loop_cnt; i++) {
        // TRACE(0, "[%s] index: %d; seq: 0x%2x, 0x%2x", __func__,
        //     g_decoder_seq_index,
        //     g_sco_packet_seq[g_decoder_seq_index],
        //     sco_packet_ptr->seq);

        // Check PLC Status
        if (check_controller_mute_pattern((uint8_t *)sco_packet_ptr, MSBC_MUTE_PATTERN)) { // Check PLC
            TRACE(0, "[%s] Decoder PLC: Mute pattern!", __func__);
            g_xxxx_plc_histogram.bt_mute++;
            plc_flag = true;
        } else if ( (sco_packet_ptr->header != SCO_PACKET_HEADER) || 
                    (sco_packet_ptr->padding != SCO_PACKET_PADDING)) { // Check header and padding
            TRACE(0, "[%s] Decoder PLC: Parser packet error: 0x%x, 0x%x!", __func__, sco_packet_ptr->header, sco_packet_ptr->padding);
            g_xxxx_plc_histogram.parser++;
            plc_flag = true;
        } else { // Check index
            uint32_t index = get_packet_index(sco_packet_ptr->seq);
            if (g_decoder_seq_index == SCO_PACKET_SEQ_INDEX_INVALID) { // First frame
                if (index == SCO_PACKET_SEQ_INDEX_INVALID) { // Can not match valid seq
                    TRACE(0, "[%s] Decoder PLC: First seq error!", __func__);
                    g_xxxx_plc_histogram.seq++;
                    plc_flag = true;
                } else {
                    TRACE(0, "[%s] Decoder: First seq OK!", __func__);
                    g_decoder_seq_index = index;
                    plc_flag = false;
                }
            } else if (g_decoder_seq_index != index) {
                TRACE(0, "[%s] Decoder PLC seq error!", __func__);
                g_xxxx_plc_histogram.seq++;
                if (index < SCO_PACKET_SEQ_NUM) {
                    TRACE(0, "[%s] Decoder PLC: Fix seq!", __func__);
                    g_decoder_seq_index = index;
                }
                plc_flag = true;
            } else {
                plc_flag = false;
            }
        }

        if (plc_flag == false) {
            dec_pcm_data.pcm_data = (int16_t *)pcm_buf;
            dec_pcm_data.buffer_size = CODEC_PCM_FRAME_SIZE*2;
            dec_pcm_data.valid_size = 0;
            sbc_frame_t sbc_data;
            sbc_data.sbc_data = sco_packet_ptr->payload;
            sbc_data.valid_size = SCO_PACKET_SIZE;
            int ret = sbc_decoder_process_frame(g_dec_state, &sbc_data, &dec_pcm_data);
            if (ret != SBC_OK) {
                TRACE(0, "[%s] ERROR: Decoder failed!", __func__);
            }
            // PLC: Save good data
            lpc_plc_save(plc_st, (int16_t *)pcm_buf);
        } else {
            // PLC: Generate data
            lpc_plc_generate(plc_st, (int16_t *)pcm_buf, NULL);
        }


        g_xxxx_plc_histogram.total++;

        if (g_decoder_seq_index != SCO_PACKET_SEQ_INDEX_INVALID) {
            g_decoder_seq_index = (g_decoder_seq_index + 1) % SCO_PACKET_SEQ_NUM;
        }
        
        sco_packet_ptr++;
        pcm_buf += CODEC_PCM_FRAME_SIZE;
    }

    return 0;
}

bt_sco_codec_t bt_sco_codec_xxxx = {
    .init       = bt_sco_codec_xxxx_init,
    .deinit     = bt_sco_codec_xxxx_deinit,
    .encoder    = bt_sco_codec_xxxx_encoder,
    .decoder    = bt_sco_codec_xxxx_decoder
};