#ifndef OPUS_CONFIG_H
#define OPUS_CONFIG_H

/* general stuff */
#define OPUS_BUILD

/* alloc stuff */
//#define VAR_ARRAYS
//#define NORM_ALIASING_HACK

#define NONTHREADSAFE_PSEUDOSTACK
#define OVERRIDE_OPUS_ALLOC_SCRATCH

#define CUT 1

//#define OVERRIDE_OPUS_ALLOC
//#define OVERRIDE_OPUS_FREE
//#define OVERRIDE_OPUS_ALLOC_SCRATCH

//#define opus_alloc          _ogg_malloc
//#define opus_free           _ogg_free
//#define opus_alloc_scratch  _ogg_malloc

/* lrint */
//#define HAVE_LRINTF 0
//#define HAVE_LRINT  0

/* embedded stuff */
#define FIXED_POINT		1
#define DISABLE_FLOAT_API
//#define EMBEDDED_ARM 1

/* undefinitions */
#ifdef ABS
#undef ABS
#endif
#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

#define OPUS_ARM_ASM 1
#define OPUS_ARM_INLINE_ASM 1
#define OPUS_ARM_INLINE_EDSP 1
//#define OPUS_ARM_INLINE_EDSP_CM4 1

#endif /* OPUS_CONFIG_H */

