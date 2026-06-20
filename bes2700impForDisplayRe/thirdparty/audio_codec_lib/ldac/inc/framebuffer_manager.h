/******************************************************************************
    Copyright 2019 Sony Corporation
******************************************************************************/
#ifndef FRAMEBUFFER_MANAGER_H__
#define FRAMEBUFFER_MANAGER_H__

#include <stdint.h>
#include <stdbool.h>
//#include <pthread.h>

#define pthread_mutex_t uint32_t

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* This file contains the definitions, declarations and macros for an implimentation of
 * frame buffer manager.
 */

#define FM_PACKET_MAX_SIZE      660
#define FM_PACKET_MAX_NUMBER    50
#define FM_FRAME_MAX_NUMBER     256
#define FM_MAX_FRAMES_IN_PACKET 15

#define FM_FRAME_BUFFER_SIZE (FM_PACKET_MAX_SIZE * FM_PACKET_MAX_NUMBER)
#define FM_REF_TABLE_SIZE (sizeof(struct fm_reference) * FM_FRAME_MAX_NUMBER)
#define FM_HANDLE_SIZE (sizeof(struct fm_handle))

#define FM_RES_OK   0
#define FM_RES_NG   (-1)

#define FM_CHECK_RATE_192000	0
#define FM_CHECK_RATE_176400    1
#define FM_CHECK_RATE_96000     2
#define FM_CHECK_RATE_88200     3
#define FM_CHECK_RATE_48000     4
#define FM_CHECK_RATE_44100     5
#define FM_CHECK_CH_STEREO      1
#define FM_CHECK_CH_DUAL        2
#define FM_CHECK_CH_MONO        4

struct fm_reference {       /*  */
    uint16_t status;        /*  */
    uint16_t size;          /*  */
    uint8_t *frame;         /*  */
};

struct fm_handle {          /*  */
    pthread_mutex_t mutex;  /*  */
    uint32_t frame_number;  /*  */
    int32_t ref_rp_idx;     /*  */
    int32_t ref_wp_idx;     /*  */
    uint8_t *buf_wp;        /*  */

    struct fm_reference *ref_table; /*  */
    uint8_t *frame_buffer;  /*  */
};

typedef struct fm_handle *tFM_HANDLE;

struct fm_frame_check {     /*  */
    int32_t rate;           /*  */
    int32_t channel;        /*  */
};

struct fm_vstatus {         /*  */
    int32_t index;          /*  */
    int32_t status;         /*  */
};

/* Allocation of FM handle.
 *  Format
 *      tFM_HANDLE fm_get_handle(void);
 *  Arguments
 *      None.
 *  Return value
 *      tFM_HANDLE : Handler memory address.
 *                   (Error if NULL)
 */
tFM_HANDLE fm_get_handle(void);

/* FM Handle initialization.
 * Initialize the handler and start frame buffer management.
 * Do not allocate handler memory.
 *  Format
 *      int32_t fm_init_handle(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE  handle.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_init_handle(tFM_HANDLE hFM);

/* Closing of initialized handle.
 * Does not release handler memory.
 *  Format
 *      int32_t fm_close_handle(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE  handle.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_close_handle(tFM_HANDLE hFM);

/* FM handle memory free.
 *  Format
 *      void fm_free_handle(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE  handle.
 *  Return value
 *      None.
 */
void fm_free_handle(tFM_HANDLE hFM);

/* Register a frames.
 *  Format
 *      int32_t fm_push_frames(tFM_HANDLE hFM, uint8_t *frames, int32_t frame_number,
 *                             uint32_t frames_size, struct fm_frame_check *check);
 *  Arguments
 *      hFM  tFM_HANDLE      handle.
 *      frames  uint8_t*     Frame buffer.(Multiple frames possible)
 *      frame_number int32_t Number of frames in frame buffer.
 *      frames_size  uint32_t frames buffer size
 *      check struct fm_frame_check* Frame integrity check
 *                           (Do not check if value is NULL.)
 *  Return value
 *      int32_t : Processing result.
 *              1~:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_push_frames(tFM_HANDLE hFM, uint8_t *frames, int32_t frame_number,
                       uint32_t frames_size, struct fm_frame_check *check);

/* Get top frame data.
 *  Format
 *      int32_t fm_peek_frame(tFM_HANDLE hFM, uint8_t *buff, uint32_t buff_size);
 *  Arguments
 *      hFM  tFM_HANDLE      handle.
 *      buff uint8_t*        Frame data storage destination.
 *                           Caller allocates area.
 *      buff_size int32_t    Size of buff.
 *  Return value
 *      int32_t : Processing result.
 *              1~:Size of frame
 *              FM_RES_NG:Error
 */
int32_t fm_peek_frame(tFM_HANDLE hFM, uint8_t *buff, uint32_t buff_size);

/* Get top frame status.
 *  Format
 *      int32_t fm_get_frame_status(tFM_HANDLE hFM, uint32_t *status);
 *  Arguments
 *      hFM  tFM_HANDLE      handle.
 *      status uint32_t*     Frame status.
 *                           Caller allocates area.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_get_frame_status(tFM_HANDLE hFM, uint32_t *status);

/* Get status and index of frame to verify.
 *  - If the top frame status is 0.
 *  - If the top frame status is not 0, the maximum status.
 *  Format
 *      int32_t fm_verify_status(tFM_HANDLE hFM, struct fm_vstatus *result);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *      result struct fm_vstatus*  Status value and index to be verified.
 *                                 Caller allocates area.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_verify_status(tFM_HANDLE hFM, struct fm_vstatus *result);

/* Get the number of registered frames.
 *  Format
 *      int32_t fm_get_frame_number(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *  Return value
 *      int32_t : Processing result.
 *              0~:Number of registered frames
 *              FM_RES_NG:Error
 */
int32_t fm_get_frame_number(tFM_HANDLE hFM);

/* Advances the registered frame index to the next frame.
 *  Format
 *      int32_t fm_pop_frame(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_pop_frame(tFM_HANDLE hFM);

/* Clear all registered frames.
 *  Format
 *      int32_t fm_clear(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_clear(tFM_HANDLE hFM);

/* Get the sum of all registered frame sizes.
 *  Format
 *      int32_t fm_get_all_size(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *  Return value
 *      int32_t : Processing result.
 *              0~: Registration frame size total[Byte]
 *              FM_RES_NG:Error
 */
int32_t fm_get_all_size(tFM_HANDLE hFM);

/* Display information of registered frames.
 *  Format
 *      int32_t fm_dump(tFM_HANDLE hFM);
 *  Arguments
 *      hFM  tFM_HANDLE            handle.
 *  Return value
 *      int32_t : Processing result.
 *              FM_RES_OK:Successful completion
 *              FM_RES_NG:Error
 */
int32_t fm_dump(tFM_HANDLE hFM);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FRAMEBUFFER_MANAGER_H__ */
