/* Copyright (C) Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SSP_AEC_H__
#define __SSP_AEC_H__
/** @defgroup ssp_aec_st_t ssp_aec_st_t: Acoustic echo canceller
 *  This is the acoustic echo canceller module.
 *  @{
 */
// #include "speexdsp_types.h"

#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef short pcm_t;

/** Obtain frame size used by the AEC */
#define SSP_AEC_GET_FRAME_SIZE 3

#define SSP_AEC_GET_STAGE 4

/** Set sampling rate */
#define SSP_AEC_SET_SAMPLING_RATE 24
/** Get sampling rate */
#define SSP_AEC_GET_SAMPLING_RATE 25

/* Can't set window sizes */
/** Get size of impulse response (int32) */
#define SSP_AEC_GET_IMPULSE_RESPONSE_SIZE 27

/* Can't set window content */
/** Get impulse response (int32[]) */
#define SSP_AEC_GET_IMPULSE_RESPONSE 29

#define SSP_AEC_GET_FREQ_RESPONSE 31

/** Internal echo canceller state. Should never be accessed directly. */
struct ssp_aec_st_t_;
typedef struct ssp_aec_st_t_ ssp_aec_st_t;

/** Creates a new multi-channel echo canceller state
 * @param sample_rate The sample rate of the mic and speak signal
 * @param frame_len Number of samples to process at one time (should correspond to 10-20 ms)
 * @param filter_len Number of samples of echo to cancel (should generally correspond to 100-500 ms)
 * @param nb_mic Number of microphone channels
 * @param nb_speakers Number of speaker channels
 * @return Newly-created echo canceller state
 */
ssp_aec_st_t *ssp_aec_create(int sample_rate, int frame_len, int filter_len, int delay, int nb_mic, int nb_speakers);

ssp_aec_st_t *ssp_aec_create_with_custom_allocator(int sample_rate, int frame_len, int filter_len, int delay, int nb_mic, int nb_speakers, custom_allocator *allocator);

/** Destroys an echo canceller state
 * @param st Echo canceller state
*/
void ssp_aec_destroy(ssp_aec_st_t *st);

/** Performs echo cancellation a frame, based on the audio sent to the speaker (no delay is added
 * to playback in this form)
 *
 * @param st Echo canceller state
 * @param rec Signal from the microphone (near end + far end echo)
 * @param play Signal played to the speaker (received from far end)
 * @param out Returns near-end signal with echo removed
 */
void ssp_aec_process(ssp_aec_st_t *st, const pcm_t *rec, const pcm_t *play, pcm_t *out);

/** Perform echo cancellation using internal playback buffer, which is delayed by two frames
 * to account for the delay introduced by most soundcards (but it could be off!)
 * @param st Echo canceller state
 * @param rec Signal from the microphone (near end + far end echo)
 * @param out Returns near-end signal with echo removed
*/
void ssp_aec_capture(ssp_aec_st_t *st, const pcm_t *rec, pcm_t *out);

/** Let the echo canceller know that a frame was just queued to the soundcard
 * @param st Echo canceller state
 * @param play Signal played to the speaker (received from far end)
*/
void ssp_aec_playback(ssp_aec_st_t *st, const pcm_t *play);

/** Reset the echo canceller to its original state
 * @param st Echo canceller state
 */
void ssp_aec_reset(ssp_aec_st_t *st);

/** Used like the ioctl function to control the echo canceller parameters
 *
 * @param st Echo canceller state
 * @param request ioctl-type request (one of the SSP_AEC_* macros)
 * @param ptr Data exchanged to-from function
 * @return 0 if no error, -1 if request in unknown
 */
int ssp_aec_ctl(ssp_aec_st_t *st, int request, void *ptr);



struct ssp_decorr_st_t_;

typedef struct ssp_decorr_st_t_ ssp_decorr_st;


/** Create a state for the channel decorrelation algorithm
    This is useful for multi-channel echo cancellation only
 * @param rate Sampling rate
 * @param channels Number of channels (it's a bit pointless if you don't have at least 2)
 * @param frame_size Size of the frame to process at ones (counting samples *per* channel)
*/
ssp_decorr_st *ssp_decorr_create(int rate, int channels, int frame_size);

/** Remove correlation between the channels by modifying the phase and possibly
    adding noise in a way that is not (or little) perceptible.
 * @param st Decorrelator state
 * @param in Input audio in interleaved format
 * @param out Result of the decorrelation (out *may* alias in)
 * @param strength How much alteration of the audio to apply from 0 to 100.
*/
void ssp_decorr_process(ssp_decorr_st *st, const pcm_t *in, pcm_t *out, int strength);

/** Destroy a Decorrelation state
 * @param st State to destroy
*/
void ssp_decorr_destroy(ssp_decorr_st *st);


#ifdef __cplusplus
}
#endif


/** @}*/
#endif
