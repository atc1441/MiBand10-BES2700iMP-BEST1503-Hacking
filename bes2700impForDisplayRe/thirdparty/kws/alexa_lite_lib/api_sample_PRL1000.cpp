///////////////////////////////////////////////////////////////////////////
// Copyright 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
///////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pryon_lite_common_client_properties.h"
#include "pryon_lite_PRL1000.h"

#define SAMPLES_PER_FRAME (160)
#define true 1

// global flag to stop processing, set by application
static int quit = 0;

// engine handle
static PryonLiteV2Handle sHandle = {0};

#define ALIGN(n) __attribute__((aligned(n)))

static char* engineBuffer = { 0 }; // should be an array large enough to hold the largest engine

// binary model buffer, allocated by application
// this buffer can be read-only memory as PryonLite will not modify the contents
ALIGN(4) static const char * wakewordModelBuffer = { 0 }; // should be an array large enough to hold the largest wakeword model

static void loadWakewordModel(PryonLiteWakewordConfig* config)
{
    // In order to detect keywords, the decoder uses a model which defines the parameters,
    // neural network weights, classifiers, etc that are used at runtime to process the audio
    // and give detection results.

    // Each model is packaged in two formats:
    // 1. A .bin file that can be loaded from disk (via fopen, fread, etc)
    // 2. A .cpp file that can be hard-coded at compile time

    config->sizeofModel = 1; // example value, will be the size of the binary model byte array
    config->model = &wakewordModelBuffer; // pointer to model in memory
}

// client implemented function to read audio samples
static void readAudio(short* samples, int sampleCount)
{
    // todo - read samples from file, audio system, etc.
}

// VAD event handler
static void vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent)
{
    printf("VAD state %d\n", (int) vadEvent->vadState);
}
// Wakeword event handler
static void wakewordEventHandler(PryonLiteV2Handle *handle, const PryonLiteWakewordResult* wwEvent)
{
    printf("Detected wakeword '%s'\n", wwEvent->keyword);
}

static void handleEvent(PryonLiteV2Handle *handle, const PryonLiteV2Event* event)
{
    if (event->vadEvent != NULL)
    {
        vadEventHandler(handle, event->vadEvent);
    }
    if (event->wwEvent != NULL)
    {
        wakewordEventHandler(handle, event->wwEvent);
    }
}
// main loop
int main(int argc, char **argv)
{
    PryonLiteV2Config engineConfig = {0};
    PryonLiteV2EventConfig engineEventConfig = {0};
    PryonLiteV2ConfigAttributes configAttributes = {0};

    // Wakeword detector configuration
    PryonLiteWakewordConfig wakewordConfig = PryonLiteWakewordConfig_Default;
    loadWakewordModel(&wakewordConfig);
    wakewordConfig.detectThreshold = 500; // default threshold
    wakewordConfig.useVad = 1;  // enable voice activity detector
    engineConfig.ww = &wakewordConfig;

    engineEventConfig.enableVadEvent = true;
    engineEventConfig.enableWwEvent = true;

    PryonLiteStatus status = PryonLite_GetConfigAttributes(&engineConfig, &engineEventConfig, &configAttributes);

    if (status.publicCode != PRYON_LITE_ERROR_OK)
    {
        // handle error
        return -1;
    }

    if (configAttributes.requiredMem > sizeof(engineBuffer))
    {
        // handle error
        return -1;
    }

    status = PryonLite_Initialize(&engineConfig, &sHandle, handleEvent, &engineEventConfig, engineBuffer, sizeof(engineBuffer));

    if (status.publicCode != PRYON_LITE_ERROR_OK)
    {
        return -1;
    }

    // Set detection threshold for all keywords (this function can be called any time after decoder initialization)
    int detectionThreshold = 500;
    status = PryonLiteWakeword_SetDetectionThreshold(sHandle.ww, NULL, detectionThreshold);
    if (status.publicCode != PRYON_LITE_ERROR_OK)
    {
        return -1;
    }

    // Call the set client property API to inform the engine of client state changes
    status = PryonLite_SetClientProperty(&sHandle, CLIENT_PROP_GROUP_COMMON, CLIENT_PROP_COMMON_AUDIO_PLAYBACK, 1);

    if (status.publicCode != PRYON_LITE_ERROR_OK)
    {
        return -1;
    }

    // allocate buffer to hold audio samples
    short samples[SAMPLES_PER_FRAME];

    // run decoder
    while (1)
    {
        // read samples
        readAudio(samples, SAMPLES_PER_FRAME);

        // pass samples to decoder
        status = PryonLite_PushAudioSamples(&sHandle, samples, SAMPLES_PER_FRAME);

        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            // handle error

            return -1;
        }



        if (quit)
        {


            // Calling Destroy() will flush any unprocessed audio that has been
            // pushed and allows for calling Initialize() again if desired.
            status = PryonLite_Destroy(&sHandle);
            if (status.publicCode != PRYON_LITE_ERROR_OK)
            {
                // handle error

                return -1;
            }
            break;
        }
    }

    return 0;
}
