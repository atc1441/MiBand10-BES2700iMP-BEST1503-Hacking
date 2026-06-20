// Copyright 2019 Google LLC.
// Libgsound version: bb3b118
#ifndef GSOUND_TARGET_HOTWORD_MODEL_COMMON_H
#define GSOUND_TARGET_HOTWORD_MODEL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file is used for all targets implementing hotword, where the hotword
 * detection is handled by GSound (internal) or by the target (external).
 */

/**
 * The Google hotword library is distributed separately from libgsound. Reach
 * out to your Google contact to get the correct hotword library for your
 * platform.
 */

#define SUPPORTED_HOTWORD_MODEL_DELIM "\n"

#define GSOUND_HOTWORD_MODEL_ID_BYTES 4

typedef enum {
  /**
   * An mmap of this type is Read-Only data
   */
  GSOUND_HOTWORD_MMAP_TEXT,
  /*
   * An mmap of this type is preinitialized Read-Write data
   */
  GSOUND_HOTWORD_MMAP_DATA,
  /*
   * An mmap of this type is uninitialized Read-Write data
   */
  GSOUND_HOTWORD_MMAP_BSS
} GSoundHotwordMmapType;

#ifdef __cplusplus
}
#endif

#endif  // GSOUND_TARGET_HOTWORD_MODEL_COMMON_H
