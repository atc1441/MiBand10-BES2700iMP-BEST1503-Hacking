//////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API error definitions for PryonLite
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_ERROR_H
#define PRYON_LITE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

///
/// @brief Public Error codes
///
typedef enum PryonLiteError
{
    PRYON_LITE_ERROR_OK = 0,            ///< Ok
    PRYON_LITE_ERROR,                   ///< General Error
    PRYON_LITE_ERROR_INVALID_CONFIG,    ///< Invalid Configuration
    PRYON_LITE_ERROR_NOT_INIT,          ///< Not Initialized
    PRYON_LITE_ERROR_ALREADY_INIT,      ///< Already Initialized
    PRYON_LITE_ERROR_INVALID_PARAM,     ///< Invalid Parameter
    PRYON_LITE_ERROR_INSUFFICIENT_MEM,  ///< Insufficient memory for given model
    PRYON_LITE_ERROR_INTERNAL,          ///< Internal error
    PRYON_LITE_ERROR_MODEL_INCOMPATIBLE,///< Model type or version is not compatible with this build of the engine
    PRYON_LITE_ERROR_MODEL_CORRUPT,     ///< Model malformed
    PRYON_LITE_ERROR_MODEL_ALIGNMENT,   ///< Model alignment error
    PRYON_LITE_ERROR_API_INCOMPATIBLE,  ///< Header version incompatible with library version
    PRYON_LITE_ERROR_NOT_SUPPORTED,     ///< Action not supported for given model
} PryonLiteError;

///
/// @brief Structure provides status of API calls.
///
typedef struct PryonLiteStatus
{
    int publicCode;           ///< Public Error code for error checking.
    int internalCode;         ///< Internal Error code for diagnostic purposes.
}PryonLiteStatus;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //PRYON_LITE_ERROR_H
