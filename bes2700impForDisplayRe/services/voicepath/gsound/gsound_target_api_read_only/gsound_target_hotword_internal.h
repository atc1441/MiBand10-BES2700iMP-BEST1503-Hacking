// Copyright 2019 Google LLC.
// Libgsound version: bb3b118
#ifndef GSOUND_TARGET_HOTWORD_INTERNAL_H
#define GSOUND_TARGET_HOTWORD_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif


#include "gsound.h"
#include "gsound_target_hotword_common.h"

/*******************************************************************************
 *
 * Internal Hotword Detection
 *
 * This file is only used for targets choosing to support "internal hotword",
 * where libgsound handles all hotword detection internally, through the same
 * audio interface as PTT queries.
 * The only hotword specific behavior that the target needs to implement is
 * reading and writing the hotword model file "blobs" from the file system.
 *
 * For hotword detection on an separate DSP, where the target notifies GSound
 * upon detection of a hotword, and tranfsers the buffered audio to GSound, see
 * "external hotword".
 *
 * To support internal hotwording, call GSoundServiceInitHotwordInternal.
 * This will cause the internal hotword objects to be referenced and included at
 * link time.
 *
 * For targets without hotword support, do not call
 * GSoundServiceInitHotwordInternal, and ignore this file.
 *
 ******************************************************************************/


/**
 * Get the model blob for the given hotword model id
 * param[in] model_id The 4 character model identifier
 *
 * Returns a pointer to the model blob for model_id. If model_id is not
 * present on the device, return NULL
 */
const uint8_t *GSoundTargetHotwordGetModelFile(const char *model_id);

/**
 * Return the list of available models on the device. The format is
 * "<model_id 1>\n<model_id 2>\n...<model_id N>". N must be less than or equal
 * to GSOUND_MAX_SUPPORTED_MODELS defined in gsound_target_hotword_common.h
 *
 * param[in] buffer_len: length of buffer pointed to by models_out
 * param[out] models_out: Buffer to copy string of supported models
 *
 * Return GSOUND_STATUS_ERROR if copying fails (e.g. buffer is too small)
 *        GSOUND_STATUS_OK otherwise
 */
GSoundStatus GSoundTargetHotwordGetSupportedModels(uint8_t buffer_len,
                                                   char *const models_out);

#ifdef __cplusplus
}
#endif

#endif  // GSOUND_TARGET_HOTWORD_INTERNAL_H
