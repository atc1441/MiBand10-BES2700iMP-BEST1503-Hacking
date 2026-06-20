//////////////////////////////////////////////////////////////////////////
//
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////
//
// Public API metadata definitions for PryonLite
//
///////////////////////////////////////////////////////////////////////////

#ifndef PRYON_LITE_METADATA_H
#define PRYON_LITE_METADATA_H

#ifdef __cplusplus
extern "C" {
#endif

///
/// @brief Metadata blob accompanying PryonLite detection events
///
typedef struct PryonLiteMetadataBlob
{
    long int blobSize; // in bytes
    const char *blob;
} PryonLiteMetadataBlob;

#ifdef __cplusplus
} // extern "C"
#endif

#endif //PRYON_LITE_METADATA_H
