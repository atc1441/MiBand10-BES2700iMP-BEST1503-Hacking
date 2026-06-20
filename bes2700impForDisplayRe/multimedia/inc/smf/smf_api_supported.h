#pragma once
#ifndef __SMF_API_SUPPORTED_H__
#define __SMF_API_SUPPORTED_H__
#include "smf_common.h"

/** get the supported decoders list.
 * @return :the decoder keywords list, end with NULL.
 */
EXTERNC int smf_supported_decoder(const char* keys[], int max);

/** get the supported encoders list.
 * @return :the encoder keywords list, end with NULL.
 */
EXTERNC int smf_supported_encoder(const char* keys[], int max);

/** get the supported filter list.
 * @return :the filter keywords list, end with NULL.
 */
EXTERNC int smf_supported_processer(const char* keys[], int max);

EXTERNC int smf_supported_demuxer(const char* keys[], int max);
EXTERNC int smf_supported_muxer(const char* keys[],int max);

/** get the supported decoders list.
 * @return :is supported .
 */
EXTERNC int smf_is_supported_decoder(const char* keys);

/** get the supported encoders list.
 * @return :the encoder keywords list, end with NULL.
 */
EXTERNC int smf_is_supported_encoder(const char* keys);

/** get the supported filter list.
 * @return :the filter keywords list, end with NULL.
 */
EXTERNC int smf_is_supported_processer(const char* keys);

EXTERNC int smf_is_supported_demuxer(const char* keys);
EXTERNC int smf_is_supported_muxer(const char* keys);

#endif
