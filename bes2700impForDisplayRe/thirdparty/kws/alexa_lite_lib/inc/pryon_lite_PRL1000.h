//////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API for PryonLite V2
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_PRL1000_H
#define PRYON_LITE_PRL1000_H

#include <stddef.h>

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
/// @brief Handle to V2 engine state
///
typedef void* PryonLiteEngineStateHandle;

///
/// @brief Config structure with one member per sub-config.
///        Populate a sub-config pointer to enable it.
///        Set a sub-config pointer to NULL to disable it.
///
typedef struct PryonLiteV2ConfigPRL1000_s {
    PryonLiteWakewordConfig *ww; ///< Configuration parameters to be used for wakeword
} PryonLiteV2Config;

///
/// @brief Handle structure holding all instances created
///
typedef struct PryonLiteV2HandlePRL1000_s {
    PryonLiteWakewordHandle ww; ///< Handle to be used for wakeword
    PryonLiteEngineStateHandle engineState; ///< Handle to be used for internal engine state
} PryonLiteV2Handle;

///
/// @brief PryonLite attributes
///
typedef struct PryonLiteV2ConfigAttributesPRL1000_s{
    const char* engineVersion;  ///< PryonLite engine version.
    size_t maxMetadataBlobSize; ///< Maximum size of metadata blob returned as part of a PryonLiteResult
    size_t requiredMem;         ///< Memory in bytes required by an engine instance using this config
    PryonLiteWakewordConfigAttributes wwConfigAttributes; ///< Attributes for the wakeword config
} PryonLiteV2ConfigAttributes;

///
/// @brief PryonLite external event
///
typedef struct PryonLiteV2EventPRL1000_s {
    PryonLiteVadEvent *vadEvent;
    PryonLiteWakewordResult *wwEvent;
} PryonLiteV2Event;

///
/// @brief PryonLite external event configuration
///
typedef struct PryonLiteV2EventConfigPRL1000_s {
    int enableVadEvent;
    int enableWwEvent;
} PryonLiteV2EventConfig;

///
/// @brief Callback function for event handling
/// @param handle [in] Handle to pryon lite instance
/// @param result [in] Event structure
/// @note Elements within the result struct are valid only for the scope of the callback function.
///
typedef void (*PryonLiteEvent_Callback)(PryonLiteV2Handle *handle, const PryonLiteV2Event* event);

/// @brief Get configuration attributes
/// @param config [in] Engine configuration
/// @param eventConfig [in] External event configuration
/// @param configAttributes [out] Configuration attributes
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
DLLDECL PryonLiteStatus PryonLite_GetConfigAttributes(const PryonLiteV2Config* config, PryonLiteV2EventConfig* eventConfig, PryonLiteV2ConfigAttributes* configAttributes);

/// @brief Initializes the engine
/// @param config [in]   Engine configuration
/// @param pHandle [in/out] Pointer to engine handle for use in subsequent references to engine object; the value
/// must be initialized to NULL when passed to this function. Existing engine instances must not be reinitialized,
/// without first calling PryonLite_Destroy().
/// @param publicEventCallback [in] Callback function for handling external events
/// @param eventConfig [in] External event configuration
/// @param engineMem [in] Instance memory supplied by client for the engine instance, as per config attributes.
/// @param sizeofEngineMem [in]   Engine configuration Size of memory supplied for the engine instance (in bytes).
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
DLLDECL PryonLiteStatus PryonLite_Initialize(const PryonLiteV2Config* config, PryonLiteV2Handle* handle, PryonLiteEvent_Callback publicEventCallback, PryonLiteV2EventConfig* eventConfig, char* engineMem, size_t sizeofEngineMem);

///
/// @brief Destroys the engine. All internal state and buffers are flushed and reset. Any pending
///        wakeword detection events internally queued will invoke the detection callback. After this call,
///        the engine must be reinitialized before resuming processing.
///
/// @param pHandle [in/out] Handle to engine instance, reset to NULL if reset is successful
///
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
DLLDECL PryonLiteStatus PryonLite_Destroy(PryonLiteV2Handle *handle);

///
/// @brief Submits audio to be decoded.
///
/// @param handle [in] Handle to engine instance
/// @param samples [in] Buffer of audio to be processed. Samples should be 16-bit (short) right-justified integers.
///                     Audio format is single-channel / 16-bit / 16kHz / Linear PCM
/// @param sampleCount [in]  Number of samples. This must be equal to the frame size returned in the
///                          "samplesPerFrame" member of the result structure in the call to
///                          PryonLite_Initialize().
///
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
DLLDECL PryonLiteStatus PryonLite_PushAudioSamples(PryonLiteV2Handle *handle, const short *samples, int sampleCount);

/// @brief Checks if the engine has been initialized
///
/// @param handle [in] Handle to engine instance
///
/// @return 1 if initialized, 0 if not initialized
///
DLLDECL int PryonLite_IsInitialized(PryonLiteV2Handle *handle);

///
/// @brief Set a client property.
///
/// @param handle [in] Handle to engine instance
/// @param group [in] Client property group to set.
/// @param property [in] Client property to set.
/// @param data [in] Data to set
///
/// @return a PryonLiteStatus structure, nominally { PRYON_LITE_ERROR_OK, 0 }
///
DLLDECL PryonLiteStatus PryonLite_SetClientProperty(PryonLiteV2Handle *handle, int group, int property, long int data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //PRYON_LITE_PRL1000_H
