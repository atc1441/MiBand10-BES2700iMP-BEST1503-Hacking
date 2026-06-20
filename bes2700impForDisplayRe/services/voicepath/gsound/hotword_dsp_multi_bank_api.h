// Copyright 2019 Google LLC.
#ifndef SPEECH_MICRO_API_HOTWORD_DSP_MULTI_BANK_API_H_
#define SPEECH_MICRO_API_HOTWORD_DSP_MULTI_BANK_API_H_

// This API manages a global singleton of data and state behind the scenes. It's
// the caller's responsibility to store the contents of the hotword_memmap model
// files into aligned memory and pass their pointers to this library. Note that
// no additional memory is allocated.

#ifdef __cplusplus
extern "C" {
#endif

// Specifies the required alignment for each memory bank.
extern const int kGoogleHotwordRequiredDataAlignment;

// Called to set up the Google hotword algorithm. Returns a void* memory handle
// if successful, and NULL otherwise. It receives an array of memory bank
// pointers, each one loaded with one of the provided hotword_memmap model files
// (in order). This function should only be called once.
void* GoogleHotwordDspMultiBankInit(void** memory_banks, int number_of_banks);

// Call with every frame of samples to process. If a hotword is detected, this
// function returns 1 otherwise 0. The required preamble length will be set to
// the number of milliseconds of buffered audio to be transferred to the
// application processor.
// It needs the memory handle returned by GoogleHotwordDspMultiBankInit.
int GoogleHotwordDspMultiBankProcess(const void* samples, int num_samples,
                                     int* preamble_length_ms,
                                     void* memory_handle);

// If there's a break in the audio stream (e.g. when Sound Activity Detection is
// enabled), call this before any subsequent calls to
// GoogleHotwordDspMultiBankProcess.
void GoogleHotwordDspMultiBankReset(void* memory_handle);

// Returns the maximum possible audio preamble length in miliseconds.
int GoogleHotwordDspMultiBankGetMaximumAudioPreambleMs(void* memory_handle);

// Returns an internal version number that this library was built at.
extern int GoogleHotwordVersion();

#ifdef __cplusplus
}
#endif

#endif  // SPEECH_MICRO_API_HOTWORD_DSP_MULTI_BANK_API_H_
