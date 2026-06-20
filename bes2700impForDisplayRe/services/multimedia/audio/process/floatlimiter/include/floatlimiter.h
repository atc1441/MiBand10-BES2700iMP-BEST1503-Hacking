#ifndef FLOATLIMITER_H
#define FLOATLIMITER_H

#include <stdint.h>

#define FL_ATTACK_DEFAULT_MS      (15)              /* default attack  time in ms */
#define FL_RELEASE_DEFAULT_MS     (50)              /* default release time in ms */

typedef enum {
	FLOATLIMIT_OK = 0,
	FLOATLIMIT_INVALID_HANDLE,
	FLOATLIMIT_INVALID_PARAMETER,
} FLOATLIMITER_ERROR;

struct FloatLimiter;
typedef struct FloatLimiter* FloatLimiterPtr;

#define FL_ATTACK_DEFAULT_MS      (15)              /* default attack  time in ms */
#define FL_RELEASE_DEFAULT_MS     (50)              /* default release time in ms */

#define FL_THRESHOLD                     (1.0f)              /* default release time in ms */

#define MAX_CHANEL (2)
#define MAX_SAMPLERATE  (384000)
#define MAX_SAMPLEBIT (32)

#define ATTACK_SAMPLES (48*FL_ATTACK_DEFAULT_MS)//48000Hz*0.015s
#define MAX_BUF_SAMPLES (ATTACK_SAMPLES+1) 
#define DELAY_BUF_SAMPLES (ATTACK_SAMPLES*MAX_CHANEL) //*ch

#define FL_STATE_SIZE (8744)

#ifdef __cplusplus
extern "C" {
#endif

const uint32_t GetFloatLimiterSize(uint32_t maxSampleRate, uint32_t maxChannels, uint32_t maxAttackMs);

/******************************************************************************
* createLimiter                                                               *
* maxAttackMs:   maximum and initial attack/lookahead time in milliseconds    *
* releaseMs:     release time in milliseconds (90% time constant)             *
* threshold:     limiting threshold                                           *
* maxChannels:   maximum and initial number of channels                       *
* maxSampleRate: maximum and initial sampling rate in Hz                      *
* returns:       limiter handle                                               *
******************************************************************************/
FloatLimiterPtr CreateFloatLimiter(uint8_t *mem,
	unsigned int  maxAttackMs,
	unsigned int  releaseMs,
	float       threshold,
	unsigned int  maxChannels,
	unsigned int  maxSampleRate,
	unsigned int  maxSampleBit);

FLOATLIMITER_ERROR DestroyFloatLimiter(FloatLimiterPtr limiter);

/******************************************************************************
* resetLimiter                                                                *
* limiter: limiter handle                                                     *
* returns: error code                                                         *
******************************************************************************/
FLOATLIMITER_ERROR ResetFloatLimiter(FloatLimiterPtr limiter);

/******************************************************************************
* applyLimiter                                                                *
* limiter:    limiter handle                                                  *
* pGain :     pointer to gains to be applied to the signal before limiting,   *
*             which are downscaled by TDL_GAIN_SCALING bit.                   *
*             These gains are delayed by gain_delay, and smoothed.            *
*             Smoothing is done by a butterworth lowpass filter with a cutoff *
*             frequency which is fixed with respect to the sampling rate.     *
*             It is a substitute for the smoothing due to windowing and       *
*             overlap/add, if a gain is applied in frequency domain.          *
* gain_scale: pointer to scaling exponents to be applied to the signal before *
*             limiting, without delay and without smoothing                   *
* gain_size:  number of elements in pGain, currently restricted to 1          *
* gain_delay: delay [samples] with which the gains in pGain shall be applied  *
*             gain_delay <= nSamples                                          *
* samples:    input/output buffer containing interleaved samples              *
*             precision of output will be DFRACT_BITS-TDL_GAIN_SCALING bits   *
* nSamples:   number of samples per channel                                   *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR ApplyFloatLimiter(FloatLimiterPtr limiter,
	unsigned char*    input,
	float     pGain,
	unsigned int nSamples);

/******************************************************************************
* getLimiterDelay                                                             *
* limiter: limiter handle                                                     *
* returns: exact delay caused by the limiter in samples                       *
******************************************************************************/
unsigned int GetFloatLimiterDelay(FloatLimiterPtr limiter);

/******************************************************************************
* setLimiterNChannels                                                         *
* limiter:   limiter handle                                                   *
* nChannels: number of channels ( <= maxChannels specified on create)         *
* returns:   error code                                                       *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterNChannels(FloatLimiterPtr limiter, unsigned int nChannels);

/******************************************************************************
* setLimiterSampleRate                                                        *
* limiter:    limiter handle                                                  *
* sampleRate: sampling rate in Hz ( <= maxSampleRate specified on create)     *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterSampleRate(FloatLimiterPtr limiter, unsigned int sampleRate);

/******************************************************************************
* setLimiterSampleBit                                                        *
* limiter:    limiter handle                                                   *
* sampleBit: sampling bit.(16 24 32)                                   *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterSampleBit(FloatLimiterPtr limiter, unsigned int sampleBit);

/******************************************************************************
* setLimiterAttack                                                            *
* limiter:    limiter handle                                                  *
* attackMs:   attack time in ms ( <= maxAttackMs specified on create)         *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterAttack(FloatLimiterPtr limiter, unsigned int attackMs);

/******************************************************************************
* setLimiterRelease                                                           *
* limiter:    limiter handle                                                  *
* releaseMs:  release time in ms                                              *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterRelease(FloatLimiterPtr limiter, unsigned int releaseMs);

/******************************************************************************
* setLimiterThreshold                                                         *
* limiter:    limiter handle                                                  *
* threshold:  limiter threshold                                               *
* returns:    error code                                                      *
******************************************************************************/
FLOATLIMITER_ERROR SetFloatLimiterThreshold(FloatLimiterPtr limiter, float threshold);

#ifdef __cplusplus
}
#endif

#endif