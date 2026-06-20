/******************************************************************************
    Copyright 2019 Sony Corporation
******************************************************************************/
#ifndef LDACBT_BCO_FOR_FLUORIDE_H__
#define LDACBT_BCO_FOR_FLUORIDE_H__

#include <stdint.h>

//#include "a2dp_codec_api.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef LDAC_BCO_API
#define LDAC_BCO_API
#endif /* LDAC_BCO_API */

/* This file contains the definitions, declarations and macros for an implimentation of
 * LDAC buffer control operation.
 */

#define LDAC_BCO_ERR_NONE   0
#define LDAC_BCO_ERR_FATAL  (-1)

/* LDAC BCO handle type */
typedef struct _ldacbt_bco_handle * HANDLE_LDAC_BCO;

typedef void (*decoded_data_callback_t)(uint8_t* buf, uint32_t len);


/* Prepare to use LDAC BCO.
 *  - Register a callback function for passing decoded data.
 *  - Allocation of LDAC BCO handle.
 *  Format
 *      HANDLE_LDAC_BCO ldac_BCO_init(decoded_data_callback_t decode_callback);
 *  Arguments
 *      decoded_data_callback_t decode_callback 
 *                                   Callback function that outputs PCM data after decoding.
 *                                   (See also a2dp_codec_api.h)
 *  Return value
 *      HANDLE_LDAC_BCO for success, NULL for failure.
 */
LDAC_BCO_API HANDLE_LDAC_BCO ldac_BCO_init(decoded_data_callback_t decode_callback);

/* End LDAC BCO.
 *  - Release of LDAC BCO handle.
 *  Format
 *      int32_t ldac_BCO_cleanup(HANDLE_LDAC_BCO hLdacBco);
 *  Arguments
 *      HANDLE_LDAC_BCO  hLdacBco    LDAC BCO handle.
 *  Return value
 *      int32_t : Processing result.
 *              LDAC_BCO_ERR_NONE:Successful completion
 *              LDAC_BCO_ERR_FATAL:Error
 *  Note
 *      The function ldac_BCO_init() shall be called before calling this function.
 */
LDAC_BCO_API int32_t ldac_BCO_cleanup(HANDLE_LDAC_BCO hLdacBco);

/* Decode LDAC packets.
 * - Perform buffer control and decode processing.
 *  Format
 *      int32_t ldac_BCO_decode_packet(HANDLE_LDAC_BCO hLdacBco, void *data, int32_t length);
 *  Arguments
 *      HANDLE_LDAC_BCO  hLdacBco    LDAC BCO handle.
 *      void *data                   LDAC packet.
 *      int32_t length               LDAC packet size.
 *  Return value
 *      int32_t : Processing result.
 *              LDAC_BCO_ERR_NONE:Successful completion
 *              LDAC_BCO_ERR_FATAL:Error
 *  Note
 *      The function ldac_BCO_init() shall be called before calling this function.
 */
LDAC_BCO_API int32_t ldac_BCO_decode_packet(HANDLE_LDAC_BCO hLdacBco, void *data, int32_t length);

/* Start decoding process.
 *  - Start or resume decoder thread.
 *  Format
 *      int32_t ldac_BCO_start(HANDLE_LDAC_BCO hLdacBco);
 *  Arguments
 *      HANDLE_LDAC_BCO  hLdacBco    LDAC BCO handle.
 *  Return value
 *      int32_t : Processing result.
 *              LDAC_BCO_ERR_NONE:Successful completion
 *              LDAC_BCO_ERR_FATAL:Error
 *  Note
 *      The function ldac_BCO_init() shall be called before calling this function.
 */
LDAC_BCO_API int32_t ldac_BCO_start(HANDLE_LDAC_BCO hLdacBco);

/* Suspend decoding process.
 *  - Suspend the decoder thread.
 *  Format
 *      int32_t ldac_BCO_suspend(HANDLE_LDAC_BCO hLdacBco);
 *  Arguments
 *      HANDLE_LDAC_BCO  hLdacBco    LDAC BCO handle.
 *  Return value
 *      int32_t : Processing result.
 *              LDAC_BCO_ERR_NONE:Successful completion
 *              LDAC_BCO_ERR_FATAL:Error
 *  Note
 *      The function ldac_BCO_init() shall be called before calling this function.
 */
LDAC_BCO_API int32_t ldac_BCO_suspend(HANDLE_LDAC_BCO hLdacBco);

/* Configure codec information.
 *  - Set sample rate, bits/sample and channel mode.
 *  Format
 *      int32_t ldac_BCO_configure(HANDLE_LDAC_BCO hLdacBco,
 *              int32_t sample_rate, int32_t bits_per_sample, int32_t channel_mode);
 *  Arguments
 *      HANDLE_LDAC_BCO  hLdacBco    LDAC BCO handle.
 *      int32_t sample_rate          sample rate.
 *      int32_t bits_per_sample      bits/sample.
 *      int32_t channel_mode         channel mode.
 *  Return value
 *      int32_t : Processing result.
 *              LDAC_BCO_ERR_NONE:Successful completion
 *              LDAC_BCO_ERR_FATAL:Error
 *  Note
 *      The function ldac_BCO_init() shall be called before calling this function.
 */
LDAC_BCO_API int32_t ldac_BCO_configure(HANDLE_LDAC_BCO hLdacBco,
        int32_t sample_rate, int32_t bits_per_sample, int32_t channel_mode);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LDACBT_BCO_FOR_FLUORIDE_H__ */
