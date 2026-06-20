///////////////////////////////////////////////////////////////////////////
// Copyright 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
///////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pryon_lite.h"

#define SAMPLES_PER_FRAME (160)

// global flag to stop processing, set by application
static int quit = 0;

// decoder handle
static PryonLiteDecoderHandle sDecoder = NULL;

// binary model buffer, allocated by application
// this buffer can be read-only memory as PryonLite will not modify the contents
#define ALIGN(n) __attribute__((aligned(n)))

ALIGN(4) static const char * modelBuffer = { 0 }; // should be an array large enough to hold the largest model
static char* decoderBuffer = { 0 }; // should be an array large enough to hold the largest decoder

static void loadModel(PryonLiteDecoderConfig* config)
{
    // In order to detect keywords, the decoder uses a model which defines the parameters,
    // neural network weights, classifiers, etc that are used at runtime to process the audio
    // and give detection results.

    // Each model is packaged in two formats:
    // 1. A .bin file that can be loaded from disk (via fopen, fread, etc)
    // 2. A .cpp file that can be hard-coded at compile time

    config->sizeofModel = 1; // example value, will be the size of the binary model byte array
    config->model = &modelBuffer; // pointer to model in memory
}

// client implemented function to read audio samples
static void readAudio(short* samples, int sampleCount)
{
    // todo - read samples from file, audio system, etc.
}

// keyword detection callback
static void detectionCallback(PryonLiteDecoderHandle handle, const PryonLiteResult* result)
{
    printf("Detected keyword '%s'", result->keyword);
}

// VAD event callback
static void vadCallback(PryonLiteDecoderHandle handle, const PryonLiteVadEvent* vadEvent)
{
    printf("VAD state %d\n", (int) vadEvent->vadState);
}

// main loop
int main(int argc, char **argv)
{
    PryonLiteDecoderConfig config = PryonLiteDecoderConfig_Default;

    loadModel(&config);

    // Query for the size of instance memory required by the decoder
    PryonLiteModelAttributes modelAttributes;
    PryonLiteError err = PryonLite_GetModelAttributes(config.model, config.sizeofModel, &modelAttributes);

    config.decoderMem = decoderBuffer;
    if (modelAttributes.requiredDecoderMem > sizeof(decoderBuffer))
    {
        // handle error
        return -1;
    }
    config.sizeofDecoderMem = modelAttributes.requiredDecoderMem;

    // initialize decoder
    PryonLiteSessionInfo sessionInfo;

    config.detectThreshold = 500; // default threshold
    config.resultCallback = detectionCallback; // register detection handler
    config.vadCallback = vadCallback; // register VAD handler
    config.useVad = 1;  // enable voice activity detector

    err = PryonLiteDecoder_Initialize(&config, &sessionInfo, &sDecoder);

    if (err != PRYON_LITE_ERROR_OK)
    {
        return -1;
    }

    // Set detection threshold for all keywords (this function can be called any time after decoder initialization)
    int detectionThreshold = 500;
    err = PryonLiteDecoder_SetDetectionThreshold(sDecoder, NULL, detectionThreshold);

    if (err != PRYON_LITE_ERROR_OK)
    {
        return -1;
    }

    // allocate buffer to hold audio samples
    short samples[SAMPLES_PER_FRAME];

    // run decoder
    while(1)
    {
        // read samples
        readAudio(samples, sessionInfo.samplesPerFrame);

        // pass samples to decoder
        err = PryonLiteDecoder_PushAudioSamples(sDecoder, samples, sessionInfo.samplesPerFrame);

        if (err != PRYON_LITE_ERROR_OK)
        {
            // handle error

            return -1;
        }

        if (quit)
        {
            // Calling Destroy() will flush any unprocessed audio that has been
            // pushed and allows for calling Initialize() again if desired.
            err = PryonLiteDecoder_Destroy(&sDecoder);
            if (err != PRYON_LITE_ERROR_OK)
            {
                // handle error

                return -1;
            }
            break;
        }
    }

    return 0;
}


