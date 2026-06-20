#ifndef DIRAC_H_
#define DIRAC_H_

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

typedef enum {
    DIRAC_CHANNEL_CONFIG_RESERVED = 0, // invalid channel configuration
    // Stereo channel configuration. Expects 2 channels input, and produces 2 channels output.
    DIRAC_CHANNEL_CONFIG_STEREO = 1,
    // TWS channel configurations. Produces mono output in the first channel.
    // For Opteo, and single input channel is used. For Virtuo, stereo input is expected.
    DIRAC_CHANNEL_CONFIG_TWS_LEFT  = 2,
    DIRAC_CHANNEL_CONFIG_TWS_RIGHT = 3,
} dirac_channel_config_t;

typedef enum {
    // `int32_t` with 8 msb ignored
    DIRAC_SAMPLE_FORMAT_R24,
    // `int16_t`
    DIRAC_SAMPLE_FORMAT_I16,
} dirac_sample_format_t;

typedef struct {
    int samplerate;
    dirac_channel_config_t channel_config;
    dirac_sample_format_t sample_format;
} dirac_config_t;

// set the callback to be used to print dirac log messages (optional)
void dirac_set_log_cb(void (*log_cb)(const char *));
// compute the necessary memory size to allocate
size_t dirac_get_size(const dirac_config_t *config);
// initialize the Dirac processor in the designated memory location
void dirac_create(const dirac_config_t *config, void *memory);
// uninitialize the Dirac processor
void dirac_destroy();
// apply the Dirac processor in-place on the input. `data` contains
// samples according to the format specified by `sample_format` during
// creation. Channels are stored interleaved. `len` is the total
// number of samples for all channels (maximum 1024*`n_channels`).
void dirac_process(uint8_t *data, int len);
// set the Dirac processor to enabled or disabled (bypass)
void dirac_set_enable(bool enable);

#endif // DIRAC_H_
