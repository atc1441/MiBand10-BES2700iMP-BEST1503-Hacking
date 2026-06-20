#ifndef __RPC_SBC_FRONTEND_H__
#define __RPC_SBC_FRONTEND_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The command name,this must be same with server side
 * NOTE: don't exceed the RPC_CMD_NAME_LEN
 */
#define INIT_CMD_NAME           "init_decoder"
#define DECODE_FRAME_CMD_NAME   "start_decoder"
#define DECODE_END_CMD_NAME     "end_cmd"

/*
 * paramters used for sbc_frames_decode
 * NOTE: must same between server&client
 */
struct sbc_decord_params {
    uint8_t         *Buff;
    uint16_t         Len;
    btif_sbc_pcm_data_t *PcmData;
    uint16_t         MaxPcmData;
    float*          gains;
};

/*
 * returned value for sbc_frames_decode
 * NOTE: must same between server&client
 */

struct sbc_decode_result {
    bt_status_t result;
    uint16_t BytesParsed;
};

#ifdef __cplusplus
}
#endif


#endif /* __RPC_SBC_FRONTEND_H__ */
