#if defined(A2DP_STREAM_DETECT_NO_DECODE)
#include "app_a2dp_stream_detect.h"

#ifdef A2DP_AAC_ON
extern bool aac_is_mute(char* aac_data, unsigned int bytes, short threshold);
extern int aac_get_max_scale(char* aac_data, uint32_t bytes);
#endif // A2DP_AAC_ON

static uint16_t sample_per_sbc_frame = 128;
static uint16_t sample_per_aac_frame = 1024;

stream_detct_param_t detect_param = {0, 25};

void app_a2dp_stream_set_detect_threshold(codec_type_t codec, uint8_t threshold)
{
    TRACE(0, "set_detect_threshold codec %d threshold %d", codec, threshold);
    if (codec == SBC_CODEC)
    {
        detect_param.sbc_detect_threshold = threshold;
    }
    else if (codec == AAC_CODEC)
    {
        detect_param.aac_detect_threshold = threshold;
    }
}

detect_result_t app_a2dp_detect_sbc_stream(U8 * media_payload)
{
    detect_result_t ret = {UNKNOWN_STREAM, 0};
    uint8_t num_of_frames = 0;
    uint8_t channel_mod = 0;
    uint8_t nrof_channels = 0;
    uint8_t nrof_subbands = 0;
    uint8_t bitpool = 0;
    uint8_t length_of_scale_factors = 0;
    uint8_t blocks = 0;
    uint8_t nrof_blocks = 0;
    uint16_t frame_length = 0;
    U8 * all_zero[8] = {0};
    bool is_mute_stream = true;

    num_of_frames = media_payload[0] & 0x0F;  // Get Number of frames
    TRACE(0, "mp 0x%x mp1 0x%x nof %d", media_payload[0], media_payload[1], num_of_frames);
    media_payload = media_payload + 1;        // Skip media_payload_header(1 Byte)
    // Todo: add length check to avoid some crash
    // Todo: check the offset value
    for (int i = 0; i < num_of_frames; i++)
    {
        uint8_t offset = 1;     // Skip syncword(1 Byte)
        blocks = (media_payload[offset]>>4) & 0x03;
        nrof_blocks = 4 * (blocks + 1);

        channel_mod = (media_payload[offset]>>2) & 0x03;
        if (channel_mod == 0)
        {
            nrof_channels = 1;
        }
        else
        {
            nrof_channels = 2;
        }

        if ((media_payload[offset] & 0x01) == 0)
        {
            nrof_subbands = 4;
        }
        else
        {
            nrof_subbands = 8;
        }

        offset++;
        bitpool = media_payload[offset];
        offset++; //Skip crc_check
        if (channel_mod == 3)
        {
            offset++; //Skip join
        }
        length_of_scale_factors = nrof_channels * nrof_subbands / 2;

        if (memcmp(&(media_payload[offset]), all_zero, length_of_scale_factors) != 0)
        {
            is_mute_stream = false;
            break;
        }

        if (channel_mod == 0 || channel_mod == 1)
        {
            frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 + (nrof_blocks * nrof_channels * bitpool) / 8;
            if ((nrof_blocks * nrof_channels * bitpool) % 8 != 0)
            {
                 frame_length++;
            }
        }
        else if (channel_mod == 2)
        {
            frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 + (nrof_blocks * bitpool) / 8;
            if ((nrof_blocks * bitpool) % 8 != 0)
            {
                 frame_length++;
            }
        }
        else
        {
            frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 + (nrof_subbands + nrof_blocks * bitpool) / 8;
            if ((nrof_subbands + nrof_blocks * bitpool) % 8 != 0)
            {
                frame_length++;
            }
        }

        // update media_payload
        media_payload = media_payload + frame_length;
        TRACE(0, "nsb 0x%x nc %d nb 0x%x bp 0x%x fl %d i %d", nrof_subbands, nrof_channels, nrof_blocks, bitpool, frame_length, i);
    }

    ret.result = is_mute_stream ? MUTE_STREAM : NORMAL_STREAM;

    ret.sample_count = num_of_frames * sample_per_sbc_frame;

    return ret;
}

#ifdef A2DP_AAC_ON
detect_result_t app_a2dp_detect_aac_stream(U8 * media_payload, uint8_t media_payload_len)
{
    detect_result_t ret = {UNKNOWN_STREAM, 0};
    uint8_t is_mute = true;
    TRACE(0, "%x %x %x %x", media_payload[0], media_payload[1], media_payload[2], media_payload[3]);
    int max_scale_fator = aac_get_max_scale((char*)media_payload, media_payload_len);

    if (max_scale_fator == (-1))
    {
        TRACE(0, "AAC error frame, ignore it");
        return ret;
    }

    if (max_scale_fator > detect_param.aac_detect_threshold)
    {
        is_mute = false;
    }

    ret.result = is_mute ? MUTE_STREAM : NORMAL_STREAM;
    ret.sample_count = sample_per_aac_frame;

    return ret;
}
#endif //A2DP_AAC_ON

#endif //A2DP_STREAM_DETECT_NO_DECODE
