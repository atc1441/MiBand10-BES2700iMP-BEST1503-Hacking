/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LDAC_PACKET_H__
#define LDAC_PACKET_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ldac_packet_handle;

struct ldac_packet_handle *pkt_open();
void pkt_close(struct ldac_packet_handle *pkt);

typedef void (*pkt_receive_frames)(void *context, int32_t nr_frames, void *data, int32_t length);
void pkt_write(struct ldac_packet_handle *pkt, void *data, int32_t length,
               pkt_receive_frames receive_frames, void *context);

#ifdef __cplusplus
}
#endif

#endif /* LDAC_PACKET_H__ */
