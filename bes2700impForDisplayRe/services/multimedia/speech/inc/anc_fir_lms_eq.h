#ifndef __ANC_FIR_LMS_EQ_H__
#define __ANC_FIR_LMS_EQ_H__

#ifdef __cplusplus
extern "C" {
#endif

void anc_fir_lms_eq_init(int sample_rate, int frame_size);

void anc_fir_lms_eq_destroy(void);

void anc_fir_lms_eq_process_f32(float **data, int frame_size);
#ifdef __cplusplus
}
#endif

#endif