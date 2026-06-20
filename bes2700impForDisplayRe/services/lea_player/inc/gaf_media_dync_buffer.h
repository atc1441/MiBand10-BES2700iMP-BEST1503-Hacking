/**
 *
 * @copyright Copyright (c) 2015-2023 BES Technic.
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
 */

#ifndef __GAF_MEDIA_DYNC_BUFFER_H__
#define __GAF_MEDIA_DYNC_BUFFER_H__


/// Start simulating the interference environment
void gaf_media_stream_start_interference();
/// Stop simulating the interference environment
void gaf_media_stream_stop_interference();
bool gaf_dync_is_interference_start();
void gaf_dync_buffer_init(GAF_AUDIO_DYNC_BUFFER_T *dync_buffer);
void gaf_dync_monitor_recv_pkt_status(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer, gaf_media_data_t* decoder_frame_p);
void gaf_dync_set_timestamp_by_variable_ft(gaf_media_data_t *pkt, GAF_AUDIO_DYNC_BUFFER_T* dync_buffer, uint32_t iso_interval);
void gaf_dync_buffer_info_reset(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer);

#endif
