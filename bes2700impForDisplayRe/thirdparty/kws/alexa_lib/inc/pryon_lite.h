//////////////////////////////////////////////////////////////////////////
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API for PryonLite Keyword Spotter
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_H
#define PRYON_LITE_H

/// Defines for compatibility purposes
#define PryonLiteDecoderConfig_Default PryonLiteWakewordConfig_Default
#define PryonLiteDecoder_SetDetectionThreshold PryonLiteWakeword_SetDetectionThreshold
#define PryonLiteDecoderHandle PryonLiteWakewordHandle
#define PryonLiteDecoderConfig PryonLiteWakewordConfig
#define PryonLiteResult PryonLiteWakewordResult

#include <stddef.h>

#include "pryon_lite_error.h"
#include "pryon_lite_ww.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(PRYONLITE_EXPORTS)
#define DLLDECL __declspec(dllexport)
#elif defined(PRYONLITE_IMPORTS)
#define DLLDECL __declspec(dllimport)
#else
#define DLLDECL
#endif

///
/// @brief PryonLite Engine Attributes
///
typedef struct PryonLiteEngineAttributes
{
    const char* engineVersion;     ///< PryonLite engine version.
    int maxMetadataBlobSize;    ///< Maximum size of metadata blob returned as part of a PryonLiteResult
    void* reserved;                ///< reserved, set to NULL

} PryonLiteEngineAttributes;

///
/// @brief PryonLite Model Attributes
///
typedef struct PryonLiteModelAttributes
{
    const char* modelVersion;   ///< PryonLite model version.
    size_t requiredDecoderMem;  ///< Memory in bytes required by a wakeword instance using this model
    void* reserved;             ///< reserved, set to NULL

} PryonLiteModelAttributes;

///
/// @brief Initialization Configuration
///
typedef struct PryonLiteSessionInfo
{
    int samplesPerFrame;   ///< input frame size required for calls to PryonLiteDecoder_PushAudioSamples()
    PryonLiteEngineAttributes engineAttributes; ///< Attributes pertaining to the engine, independent of loaded model.
    PryonLiteModelAttributes  modelAttributes;  ///< Attributes pertaining to the loaded model.
    void* reserved;            ///< reserved for future use
} PryonLiteSessionInfo;

///
/// @brief Initializes the decoder
/// @param config [in]   Decoder configuration
/// @param result [out]  Address of a struct that is filled with session/engine/model information
/// @param pHandle [in/out] Pointer to decoder handle for use in subsequent references to decoder object; the value
/// must be initialized to NULL when passed to this function.  Existing decoder instances must not be reinitialized,
/// without first calling PryonLiteDecoder_Destroy().
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
DLLDECL PryonLiteError PryonLiteDecoder_Initialize(const PryonLiteDecoderConfig* config, PryonLiteSessionInfo* result, PryonLiteDecoderHandle *pHandle);

///
/// @brief Submits audio to be decoded.
///
/// @param handle [in] Handle to decoder instance
/// @param samples [in] Buffer of audio to be processed. Samples should be 16-bit (short) right-justified integers.
///                     Audio format is single-channel / 16-bit / 16kHz / Linear PCM
/// @param sampleCount [in]  Number of samples. This must be equal to the frame size returned in the
///                          "samplesPerFrame" member of the result structure in the call to
///                          PryonLiteDecoder_Initialize().
///
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
DLLDECL PryonLiteError PryonLiteDecoder_PushAudioSamples(PryonLiteDecoderHandle handle, const short *samples,
                                                         int sampleCount);

///
/// @brief Destroys the decoder. All internal state and buffers are flushed and reset. Any pending
///        wakeword detection events internally queued will invoke the detection callback. After this call,
///        the decoder must be reinitialized before resuming processing.
///
/// @param pHandle [in/out] Handle to decoder instance, reset to NULL if reset is successful
///
/// @return PRYON_LITE_ERROR_OK if successful, otherwise a non-zero error code
///
DLLDECL PryonLiteError PryonLiteDecoder_Destroy(PryonLiteDecoderHandle *pHandle);

///
/// @brief Checks if the decoder has been initialized
///
/// @param handle [in] Handle to decoder instance
///
/// @return 1 if initialized, 0 if not initialized
///
DLLDECL int PryonLiteDecoder_IsDecoderInitialized(PryonLiteDecoderHandle handle);

///
/// @brief Fetches the Pryon Lite engine attributes
/// @param engineAttributes Target attributes need to be filled
/// @return PRYON_LITE_ERROR_OK if the attributes are fetched successfully.
///
DLLDECL PryonLiteError PryonLite_GetEngineAttributes(PryonLiteEngineAttributes* engineAttributes);

///
/// @brief Get model attributes
/// @param model [in] Wakeword model data (loaded from disk or statically compiled in)
/// @param sizeofModel [in] The total size of model binary (in bytes).
/// @param modelAttributes [out] Model attributes
/// @return PRYON_LITE_ERROR_OK for success, otherwise a non-zero error code
///
DLLDECL PryonLiteError PryonLite_GetModelAttributes(const void *model, size_t sizeofModel, PryonLiteModelAttributes* modelAttributes);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // PRYON_LITE_H
