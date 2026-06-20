/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LDAC_BUFFER_CONTROL_H__
#define LDAC_BUFFER_CONTROL_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BC_NUM_FRAME_OBSERVE 256
#define BC_NUM_FRAME_THRESHOULD_UPPER 5
#define BC_NUM_FRAME_THRESHOULD_LOWER -20

struct buffer_control;
struct time_ldac_spec; /* time.h *//* log.h */
struct fm_handle; /* framebuffer_manager.h */

struct buffer_control *bc_new(uint32_t num_of_frames);

void bc_delete(struct buffer_control *bc);

/**
 * @param force reset burst mode if set to 1, or keep burst.
 */
void bc_reset(struct buffer_control *bc, bool force_reset);

void bc_update(struct buffer_control *bc, int32_t frame, struct time_ldac_spec *now);

void bc_burst(struct buffer_control *bc, int32_t life, struct time_ldac_spec *now);

int32_t bc_peek(struct buffer_control *bc, struct fm_handle *hFM, void *data, uint32_t size);

int32_t bc_pop(struct buffer_control *bc, struct fm_handle *hFM);

const char *bc_strstatus(struct buffer_control *bc);

double bc_frame_sma(struct buffer_control *bc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LDAC_BUFFER_CONTROL_H__ */
