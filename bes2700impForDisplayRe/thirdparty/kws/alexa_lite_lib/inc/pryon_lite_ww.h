//////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API wakeword component for PryonLite
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_WW_H
#define PRYON_LITE_WW_H

#include "pryon_lite_error.h"
#include "pryon_lite_vad.h"
#include "pryon_lite_metadata.h"

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

/// @brief Pryon lite API version
#define PRYON_LITE_API_VERSION "2.7"


///
/// @brief Handle to a wakeword instance
///
typedef void* PryonLiteWakewordHandle;

///
/// @brief Detection Result
///
typedef struct PryonLiteWakewordResult
{
    long long beginSampleIndex;      ///< Identifies the sample index in the client's source of audio at which the
                                    /// speech for the enumResult begins.

    long long endSampleIndex;        ///< Identifies the sample index in the client's source of audio at which the
                                    /// wakeword ends.  The number of samples in the audio for the
                                    /// wakeword is given by endSampleIndex - beginSampleIndex.

    const char* keyword;            ///< The keyword that was detected
    long int confidence;              ///< The confidence of the detection, from 0 (lowest) to 1000 (highest)

    PryonLiteMetadataBlob metadataBlob;  ///< Auxiliary information


    void* reserved;             ///< reserved for future use
    void* userData;             ///< userData passed in via PryonLiteWakewordConfig during wakeword initialization

} PryonLiteWakewordResult;

///
/// @brief Callback function for detections.
/// @param handle [in] Handle to wakeword instance
/// @param result [in] Keyword spotter detection event
/// @note Elements within the result struct are valid only for the scope of the callback function.
///
typedef void (*PryonLiteResult_Callback)(PryonLiteWakewordHandle handle, const PryonLiteWakewordResult* result);

///
/// @brief Callback function for VAD Event.
///        This function is called whenever the VAD state changes
/// @param handle [in] Handle to wakeword instance
/// @param event [in] Voice activity detector event
/// @note Data supplied by the callback is not guaranteed to be valid outside the scope of the callback.
///
typedef void (*PryonLiteVad_Callback)(PryonLiteWakewordHandle handle, const PryonLiteVadEvent* event);

///
/// @brief Configuration parameters to be passed in during initialization
///
typedef struct PryonLiteWakewordConfig
{
    PryonLiteResult_Callback resultCallback;    ///< callback function for handling detection notifications
    PryonLiteVad_Callback vadCallback;          ///< callback function for handling VAD state change notifications
                                                ///  optional, only when useVad is true.
    long int detectThreshold;                     ///< integer from 1-1000. Default is 500.
                                                ///< 1 = lowest threshold, most detections.
                                                ///< 1000 = highest threshold, fewest detections.
    long int useVad;                              ///< Controls use of voice-activity-detector pre-stage
    long int lowLatency;                          ///< Only valid for type 'U' models. Results in ~200ms lower detection
                                                ///< latency, at the cost of less accurate ww end indexes reported in
                                                ///< the detection callback
    const void* model;                          ///< Wakeword model data (loaded from disk or statically compiled in)
                                                ///< *** Note this memory must persist while the library is in use ***
                                                ///< *** Note this model must be 4 byte aligned ***
    size_t sizeofModel;                         ///< The total size of model binary (in bytes).
    char *decoderMem;                           ///< Instance memory supplied by client for the wakeword instance, as per model attributes
    size_t sizeofDecoderMem;                    ///< Size of memory supplied for the wakeword instance (in bytes).
    struct PryonLiteDnnAccelConfig* dnnAccel;   ///< Pointer to configuration structure for external DNN acceleration
    void* reserved;                             ///< reserved, set to NULL
    const char* apiVersion;                     ///< For header / library version consistency verification.
                                                ///  Must pass in PRYON_LITE_API_VERSION as defined in this file.
    void* userData;                             ///< User-specified data pointer, to be returned when invoking detection and VAD callbacks
} PryonLiteWakewordConfig;


///
/// @brief Default Configuration to be used to initialize PryonLiteWakewordConfig struct
///
#define PryonLiteWakewordConfig_Default \
{ \
    NULL, /* resultCallback */ \
    NULL, /* vadCallback */ \
    500, /* detectThreshold */ \
    0, /* useVad */ \
    0, /* lowLatency */ \
    NULL, /* model */ \
    0, /* sizeofModel */ \
    NULL, /* decoderMem */ \
    0, /* sizeofDecoderMem */ \
    NULL, /* dnnAccel */ \
    NULL, /* reserved */ \
    PRYON_LITE_API_VERSION, /* apiVersion */ \
    NULL, /* userData */ \
}

///
/// @brief Wakeword specific attributes
///
typedef struct PryonLiteWakewordConfigAttributes {
    const char *wwApiVersion;      //< PryonLite wakeword API version
    const char *wwConfigVersion;   //< PryonLite wakeword config version
} PryonLiteWakewordConfigAttributes;

#define PRYON_LITE_WAKEWORD_DETECT_THRESHOLD_MIN (1)     ///< Most permissive detection threshold
#define PRYON_LITE_WAKEWORD_DETECT_THRESHOLD_MAX (1000)  ///< Least permissive detection threshold

///
/// @brief Sets the detection threshold parameter
///
/// @param handle [in] Handle to wakeword instance
/// @param keyword [in] Keyword for which to set the detection threshold; ex. "ALEXA".  Pass a NULL pointer to set for all keywords.
/// @param detectThreshold [in] Integer in range [1, 1000] with 1 being most permissive, and 1000 being least permissive.
///
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
/// @note The keyword string must be in upper case.
///
DLLDECL PryonLiteStatus PryonLiteWakeword_SetDetectionThreshold(PryonLiteWakewordHandle handle, const char* keyword, int detectThreshold);


#ifdef __cplusplus
} // extern "C"
#endif

#endif //PRYON_LITE_WW_H
