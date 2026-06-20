//////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API VAD definitions for PryonLite
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_VAD_H
#define PRYON_LITE_VAD_H

#ifdef __cplusplus
extern "C" {
#endif

///
/// @brief VAD State
///
typedef enum PryonLiteVadState
{
    PRYON_LITE_VAD_INACTIVE = 0,
    PRYON_LITE_VAD_ACTIVE = 1
} PryonLiteVadState;

///
/// @brief VAD Event
///
typedef struct PryonLiteVadEvent
{
    PryonLiteVadState vadState;   ///< VAD state (inactive/active)

    void* reserved;               ///< reserved for future use
    void* userData;               ///< userData passed in via PryonLiteDecoderConfig during decoder initialization

} PryonLiteVadEvent;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //PRYON_LITE_VAD_H
