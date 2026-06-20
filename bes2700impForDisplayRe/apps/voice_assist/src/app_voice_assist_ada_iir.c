#include "app_voice_assist_ada_iir.h"

#include "app_anc_assist.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "ext_fft_f32.h"
#include "app_anc.h"
#include "app_utils.h"
#include "anc_process.h"

#include "aud_section.h"
extern struct_anc_cfg * anc_coef_list_50p7k[2];
#ifndef RESOLUTION_2048
#define HEAP_BUFF_SIZE (10 * 1024)
#else
#define HEAP_BUFF_SIZE (20 * 1024)
#endif
#include "ext_heap.h"
static ALIGNED(4) uint8_t iir_ext_heap[HEAP_BUFF_SIZE];

static assist_anc_fr_fitting_inst cur_state;
static void *fft_lookup;
static int32_t voice_assist_ada_iir_callback(void *buf, uint32_t len, void *other);
static int cpt_step = -1;
static const int cpt_steps = 150;
static int fit_or_not = 0;
static int fit_res = -1;
static int particle_pieces = 0;
static const int scale = (1 << 27);

int32_t app_voice_assist_ada_iir_init(void)
 {
    app_anc_assist_register(ANC_ASSIST_USER_ADA_IIR, voice_assist_ada_iir_callback);
    return 0;
 }

// static int32_t _ada_iir_open_mic_callback(void)
// {
//     anc_adc_data_select(ANC_ADC_ADC_ADD_MC); //fb DAM data
//     return 0;
// }

int32_t app_voice_assist_ada_iir_open(void)
{
    TRACE(0, "[%s] adaptive iir anc start stream", __func__);

    // strip music
    // app_anc_assist_open_mic_register(ANC_ASSIST_USER_ADA_IIR, _ada_iir_open_mic_callback);
    anc_adc_data_select(ANC_ADC_ADC_ADD_MC);
    TRACE(0, "[adaptive iir] anc_adc_data_select open");
    // allocate static space
    ext_heap_init(iir_ext_heap);
    fft_lookup = ext_f32_fft_init(ADA_IIR_NFFT, 1, ext_allocator.malloc);
    // algo init
    // assist_anc_fr_fitting_create(&cur_state, ADA_IIR_SR, ADA_IIR_PARTICLES, ADA_IIR_NFFT, &ext_allocator);
    assist_anc_fr_fitting_create_dynamic(&cur_state, ADA_IIR_SR, ADA_IIR_PARTICLES, ADA_IIR_NFFT);
    // speed up
    app_sysfreq_req(APP_SYSFREQ_USER_ADA_IIR, APP_SYSFREQ_208M);
    // open algo
    app_anc_assist_open(ANC_ASSIST_USER_ADA_IIR);
    return 0;
}

int32_t app_voice_assist_ada_iir_close(void)
{
    TRACE(0, "[%s] adaptive iir anc close stream", __func__);
    // recover sysfreq
    app_sysfreq_req(APP_SYSFREQ_USER_ADA_IIR, APP_SYSFREQ_26M);
    // switch to pnc
    app_anc_switch(0);
    // close algo
    app_anc_assist_close(ANC_ASSIST_USER_ADA_IIR);
    // algo destory
    // assist_anc_fr_fitting_destory(&cur_state, &ext_allocator);
    assist_anc_fr_fitting_destory_dynamic(&cur_state);
    // free static space
    ext_f32_fft_destroy(fft_lookup, ext_allocator.free);
    ext_heap_deinit();
    return 0;
}

int32_t app_voice_assist_ada_iir_reset(void) {
    TRACE(0, "[%s] adaptive iir anc reset stream", __func__);
    assist_anc_fr_fitting_reset(&cur_state);
    return 0;
}

extern uint8_t is_a2dp_mode(void);
void set_bypass_ff(int idx);
void set_bypass_fb(int idx);
void set_bypass_tt(int idx);
void set_param_ff(int idx, float* ba, float factor);
void set_param_fb(int idx, float* ba, float factor);
void set_param_tt(int idx, float* ba, float factor);

static int32_t voice_assist_ada_iir_callback(void *buf, uint32_t len, void *other) {
    // uint32_t start_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
    //
    AncAssistRes* res = other;
    if (cur_state.delay == cur_state.t4 && !cur_state.close_strip_music)
    {
        anc_adc_data_select(ANC_ADC_ONLY_ADC); // fb mic includes music
        cur_state.close_strip_music = 1;
        TRACE(0, "[adaptive iir] anc_adc_data_select recover");
    }

    // avoid howling and music disturbance
    if (cur_state.delay >= cur_state.t4) {
        // TRACE(0, "[adaptive anc]: sample over!!!");
    } else if (res->ff_howling_status[0] == HOWLING_STATUS_HOWLING
        || res->fb_howling_status[0] == HOWLING_STATUS_HOWLING) {
        TRACE(0, "Howling happens and delay adaptive iir");
        return 0;
    // } else if (is_a2dp_mode()) { // TODO: api detects 6 secs after stopping music!!!
        // TRACE(0, "Be music state and delay adaptive iir");
        // return 0;
    }

    cur_state.cur_anc_mode = app_anc_get_curr_mode();
    int bf_len = FRAME_LEN * (cur_state.nfft / FRAME_LEN + 1);

    float** input_data = buf;
    float *ff_mic = input_data[0];
    float *fb_mic = input_data[1];

    int frame_shift = (int) (cur_state.nfft / (2*FRAME_LEN)) * FRAME_LEN;
    if (cur_state.cur_anc_mode == 0) {
        if (cur_state.delay < cur_state.t1 && cur_state.work_free) {
            cur_state.work_free = 0;
            cur_state.delay++;
            cur_state.work_free = 1;
        } else if (cur_state.cnt < bf_len && cur_state.work_free){
            cur_state.work_free = 0;
            memmove(cur_state.fb_frame + cur_state.cnt, fb_mic, sizeof(float)*FRAME_LEN);
            memmove(cur_state.ff_frame + cur_state.cnt, ff_mic, sizeof(float)*FRAME_LEN);
            arm_scale_f32(cur_state.fb_frame + cur_state.cnt, 1.f/(1<<23), cur_state.fb_frame + cur_state.cnt, FRAME_LEN);
            arm_scale_f32(cur_state.ff_frame + cur_state.cnt, 1.f/(1<<23), cur_state.ff_frame + cur_state.cnt, FRAME_LEN);

            cur_state.cnt += FRAME_LEN;
            // TRACE(0, "off sample %d", cur_state.cnt);
            cur_state.work_free = 1;
        } else if (cur_state.delay < cur_state.t2 && cur_state.work_free) {
            cur_state.work_free = 0;
            ext_f32_fft(fft_lookup, cur_state.fb_frame, cur_state.fb_fft);
            ext_f32_fft(fft_lookup, cur_state.ff_frame, cur_state.ff_fft);
            my_complex_div(cur_state.fb_fft, cur_state.ff_fft, cur_state.fb_fft, cur_state.nfft);
            arm_add_f32(cur_state.fb_fft, cur_state.x, cur_state.x, cur_state.nfft);

            memmove(cur_state.fb_frame, cur_state.fb_frame + frame_shift, sizeof(float)*(bf_len-frame_shift));
            memmove(cur_state.ff_frame, cur_state.ff_frame + frame_shift, sizeof(float)*(bf_len-frame_shift));
            cur_state.cnt = bf_len - frame_shift;
            cur_state.avg_cnt++;
            cur_state.delay++;
            cur_state.work_free = 1;
        } else if (cur_state.anc_on == 0 && cur_state.work_free) {
            cur_state.work_free = 0;
            TRACE(0, "[adaptive iir] off sample over with %d times", cur_state.avg_cnt);
            arm_scale_f32(cur_state.x, 1.f/cur_state.avg_cnt, cur_state.x, cur_state.nfft);
            cur_state.avg_cnt = 0;
            cur_state.anc_on = 1;
            cur_state.last_anc_mode = cur_state.cur_anc_mode;
            app_anc_switch(1); // to sample
            cur_state.work_free = 1;
        }
    } else if ((cur_state.cur_anc_mode == 1 || cur_state.cur_anc_mode == 2) && cur_state.last_anc_mode == 0) {
        if (cur_state.delay < cur_state.t3 && cur_state.work_free) { // wait ff+tt.
            cur_state.work_free = 0;
            cur_state.delay++;
            cur_state.cnt = 0;
            cpt_step = -1;
            fit_or_not = 0;
            fit_res = -1;
            particle_pieces = 0;
            cur_state.work_free = 1;
        } else if (cur_state.cnt < bf_len && cur_state.work_free){
            cur_state.work_free = 0;
            memmove(cur_state.fb_frame + cur_state.cnt, fb_mic, sizeof(float)*FRAME_LEN);
            memmove(cur_state.ff_frame + cur_state.cnt, ff_mic, sizeof(float)*FRAME_LEN);

            arm_scale_f32(cur_state.fb_frame + cur_state.cnt, 1.f/(1<<23), cur_state.fb_frame + cur_state.cnt, FRAME_LEN);
            arm_scale_f32(cur_state.ff_frame + cur_state.cnt, 1.f/(1<<23), cur_state.ff_frame + cur_state.cnt, FRAME_LEN);
            cur_state.cnt += FRAME_LEN;
            // TRACE(0, "on sample %d", cur_state.cnt);
            cur_state.work_free = 1;
        } else if (cur_state.delay < cur_state.t4 && cur_state.work_free) {
            cur_state.work_free = 0;
            ext_f32_fft(fft_lookup, cur_state.fb_frame, cur_state.fb_fft);
            ext_f32_fft(fft_lookup, cur_state.ff_frame, cur_state.ff_fft);
            my_complex_div(cur_state.fb_fft, cur_state.ff_fft, cur_state.fb_fft, cur_state.nfft);
            arm_add_f32(cur_state.fb_fft, cur_state.y, cur_state.y, cur_state.nfft);

            memmove(cur_state.fb_frame, cur_state.fb_frame + frame_shift, sizeof(float)*(bf_len-frame_shift));
            memmove(cur_state.ff_frame, cur_state.ff_frame + frame_shift, sizeof(float)*(bf_len-frame_shift));
            cur_state.cnt = bf_len - frame_shift;
            cur_state.avg_cnt++;
            cur_state.delay++;
            cur_state.work_free = 1;
        #ifdef DEBUG_TEST_ADA_IIR // open fb first
        } else if (cur_state.anc_fb_on == 0 && cur_state.work_free) {
            cur_state.work_free = 0;
            cur_state.anc_fb_on = 1;
            app_anc_switch(2);
            cur_state.work_free = 1;
        } else if (cur_state.delay < cur_state.t4 + (int) (2.0f * 1000 / 7.5f) && cur_state.work_free) {
            cur_state.work_free = 0;
            // TRACE(1, "\n____fb delay___\n");
            cur_state.delay++;
            cur_state.work_free = 1;
        #endif
        } else if(cur_state.avg_cnt && cur_state.work_free) {
            cur_state.work_free = 0;
            TRACE(0, "[adaptive iir] on sample over with %d times", cur_state.avg_cnt);
            arm_scale_f32(cur_state.y, 1.f/cur_state.avg_cnt, cur_state.y, cur_state.nfft);
            cur_state.avg_cnt = 0;
            cur_state.work_free = 1;
        } else if (cur_state.work_free){
            cur_state.work_free = 0;

            if(cpt_step==-1 && !particle_pieces) { // initial process
                fit_or_not = assist_anc_fr_fitting_preprocess(&cur_state, cpt_steps/2);
                cur_state.fr_fitting_res->error = 1e15f;
                if (!fit_or_not) {
                    cpt_step = cpt_steps;
                }
            }

            if (cpt_step < cpt_steps) { // cpt
                fit_res = assist_anc_fr_fitting_cpt(&cur_state, cpt_step, cpt_steps, particle_pieces);
                cpt_step = (particle_pieces == 3)? cpt_step+1:cpt_step;
                particle_pieces = (particle_pieces==3)? 0:particle_pieces+1;
                cpt_step = cur_state.settings->consecutive_failed > cur_state.settings->plateau ?
                            cpt_steps : cpt_step;
            } else { // fitting over
                cur_state.last_anc_mode = cur_state.cur_anc_mode;

                cur_state.cnt = 0;
                cur_state.delay = 0;
                cur_state.anc_on = 0;
                cur_state.anc_fb_on = 0;
                cur_state.fitting_done = 0;
                cur_state.close_strip_music = 0;

                TRACE(0, "[adaptive iir] optimization with error %d.%.6d",
                    (int)cur_state.fr_fitting_res->error,
                    (int)((cur_state.fr_fitting_res->error - (int)cur_state.fr_fitting_res->error) * 1000000)
                );

                if (!fit_or_not || fit_res == -1) { // no need to add filter
                    // TRACE(0, "\n____[adaptive anc]: wearing tightly or beyond capacity!_____\n");
                    TRACE(1, "\n____iir adaptive anc algo ends___\n");
                    #ifdef ONLY_FF_IN_IIR
                    #ifdef LITTLE_FF
                    for (int i=0;i<3;i++) { // bypass ff 5-7
                        set_bypass_ff(5+i);
                    }
                    #else
                    for (int i=0;i<2;i++) { // bypass ff 6-7/ fb 0-2
                        set_bypass_ff(6+i);
                    }
                    for (int i=0;i<3;i++) {
                        set_bypass_fb(i);
                    }
                    #endif
                    #else
                    for (int i=0;i<5;i++) { // bypass tt 1-5
                        set_bypass_tt(1+i);
                    }
                    #endif

                    #ifndef DEBUG_TEST_ADA_IIR
                    cur_state.fitting_done++;
                    #endif
                }
                else {
                    #ifdef SHOW_FILTER_PARAM
                    TRACE(0, "[adaptive iir] param:");
                    TRACE(0,
                            #ifdef LITTLE_FF
                                "[adaptive iir] gain: %d.%.6d, %d.%.6d, %d.%.6d",
                            #else
                                "[adaptive iir] gain: %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d",
                            #endif
                            (int)cur_state.fr_fitting_res->gbest[0],
                            (int)((cur_state.fr_fitting_res->gbest[0] - (int)cur_state.fr_fitting_res->gbest[0])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[3],
                            (int)((cur_state.fr_fitting_res->gbest[3] - (int)cur_state.fr_fitting_res->gbest[3])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[6],
                            (int)((cur_state.fr_fitting_res->gbest[6] - (int)cur_state.fr_fitting_res->gbest[6])*1e6)
                            #ifndef LITTLE_FF
                            ,
                            (int)cur_state.fr_fitting_res->gbest[9],
                            (int)((cur_state.fr_fitting_res->gbest[9] - (int)cur_state.fr_fitting_res->gbest[9])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[12],
                            (int)((cur_state.fr_fitting_res->gbest[12] - (int)cur_state.fr_fitting_res->gbest[12])*1e6)
                            #endif
                    );

                    TRACE(0,
                            #ifdef LITTLE_FF
                            "[adaptive iir] fc: %d.%.6d, %d.%.6d, %d.%.6d",
                            #else
                            "[adaptive iir] fc: %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d",
                            #endif
                            (int)cur_state.fr_fitting_res->gbest[1],
                            (int)((cur_state.fr_fitting_res->gbest[1] - (int)cur_state.fr_fitting_res->gbest[1])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[4],
                            (int)((cur_state.fr_fitting_res->gbest[4] - (int)cur_state.fr_fitting_res->gbest[4])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[7],
                            (int)((cur_state.fr_fitting_res->gbest[7] - (int)cur_state.fr_fitting_res->gbest[7])*1e6)
                            #ifndef LITTLE_FF
                            ,
                            (int)cur_state.fr_fitting_res->gbest[10],
                            (int)((cur_state.fr_fitting_res->gbest[10] - (int)cur_state.fr_fitting_res->gbest[10])*1e6),
                            (int)cur_state.fr_fitting_res->gbest[13],
                            (int)((cur_state.fr_fitting_res->gbest[13] - (int)cur_state.fr_fitting_res->gbest[13])*1e6)
                            #endif
                    );

                        TRACE(0,
                                #ifdef LITTLE_FF
                                "[adaptive iir] q: %d.%.6d, %d.%.6d, %d.%.6d",
                                #else
                                "[adaptive iir] q: %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d, %d.%.6d",
                                #endif
                                (int)cur_state.fr_fitting_res->gbest[2],
                                (int)((cur_state.fr_fitting_res->gbest[2] - (int)cur_state.fr_fitting_res->gbest[2])*1e6),
                                (int)cur_state.fr_fitting_res->gbest[5],
                                (int)((cur_state.fr_fitting_res->gbest[5] - (int)cur_state.fr_fitting_res->gbest[5])*1e6),
                                (int)cur_state.fr_fitting_res->gbest[8],
                                (int)((cur_state.fr_fitting_res->gbest[8] - (int)cur_state.fr_fitting_res->gbest[8])*1e6)
                                #ifndef LITTLE_FF
                                ,
                                (int)cur_state.fr_fitting_res->gbest[11],
                                (int)((cur_state.fr_fitting_res->gbest[11] - (int)cur_state.fr_fitting_res->gbest[11])*1e6),
                                (int)cur_state.fr_fitting_res->gbest[14],
                                (int)((cur_state.fr_fitting_res->gbest[14] - (int)cur_state.fr_fitting_res->gbest[14])*1e6)
                                #endif
                        );

                        TRACE(0,"[adaptive iir] ref_gain: %d.%.6d",
                        #ifdef LITTLE_FF
                                (int)cur_state.fr_fitting_res->gbest[9],
                                (int)((cur_state.fr_fitting_res->gbest[9] - (int)cur_state.fr_fitting_res->gbest[9])*1e6)
                        #else
                                (int)cur_state.fr_fitting_res->gbest[15],
                                (int)((cur_state.fr_fitting_res->gbest[15] - (int)cur_state.fr_fitting_res->gbest[15])*1e6)
                        #endif
                        );
                    #endif

                    float ba[6];
                    float* solu = cur_state.fr_fitting_res->gbest;
                    #ifndef ONLY_FF_IN_IIR
                    float factor = powf(10.f, solu[15]/ 20.f);
                    for (int i=0;i<5;i++) {
                        getba(solu+3*i, 375e3, ba);
                        if (i==0) { // considering ref_gain
                            set_param_tt(1+i, ba, factor);
                        } else {
                            set_param_tt(1+i, ba, 1);
                        }
                    }
                    #else
                    #ifdef LITTLE_FF
                    float factor = powf(10.f, solu[9]/ 20.f);
                    for (int i=0;i<3;i++) {
                        getba(solu+3*i, 375e3, ba);
                        if (i==0) { // considering ref_gain
                            set_param_ff(5+i, ba, factor);
                        } else {
                            set_param_ff(5+i, ba, 1);
                        }
                    }
                    #else
                    float factor = powf(10.f, solu[15]/ 20.f);
                    for (int i=0;i<2;i++) {
                        getba(solu+3*i, 375e3, ba);
                        if (i==0) { // considering ref_gain
                            set_param_ff(6+i, ba, factor);
                        } else {
                            set_param_ff(6+i, ba, 1);
                        }
                    }
                    for (int i=2;i<5;i++) {
                        getba(solu+3*i, 375e3, ba);
                        set_param_fb(i-2, ba, 1);
                    }
                    #endif
                    #endif

                    TRACE(1, "\n____iir adaptive anc algo ends___\n");
                    cur_state.fitting_done++;
                    }
            }
            cur_state.work_free = 1;
        }
    } else if (cur_state.fitting_done) { // just for comparing ff_new with ff_new+fb in debug stage.
            #ifdef DEBUG_TEST_ADA_IIR
                app_anc_switch(3);
                cur_state.fitting_done = 0;
            #else
                app_anc_switch(2);
                cur_state.fitting_done = 0;
            #endif
            // }
        }
    // TRACE(2, "adaptive iir callback time = %d us", FAST_TICKS_TO_US(hal_fast_sys_timer_get())-start_time);
    return 0;
}

void set_bypass_ff(int idx) {
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_b[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_a[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_b[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_a[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_b[2] = 0x07dbd999;
    anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_a[2] = 0x07dbd999;
}

void set_bypass_fb(int idx) {
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_b[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_a[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_b[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_a[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_b[2] = 0x07dbd999;
    anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_a[2] = 0x07dbd999;
}

void set_bypass_tt(int idx) {
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_b[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_a[0] = 0x08000000;
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_b[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_a[1] = 0xf024cb39;
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_b[2] = 0x07dbd999;
    anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_a[2] = 0x07dbd999;
}

void set_param_ff(int idx, float* ba, float factor) {
    for (int j = 0; j < 3; j++) {
        anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_b[j] = (int) (scale * factor * ba[j]);
        anc_coef_list_50p7k[1]->anc_cfg_ff_l.iir_coef[idx].coef_a[j] = (int) (scale * ba[3+j]);
    }
}

void set_param_fb(int idx, float* ba, float factor) {
    for (int j = 0; j < 3; j++) {
        anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_b[j] = (int) (scale * factor * ba[j]);
        anc_coef_list_50p7k[1]->anc_cfg_fb_l.iir_coef[idx].coef_a[j] = (int) (scale * ba[3+j]);
    }
}

void set_param_tt(int idx, float* ba, float factor) {
    for (int j = 0; j < 3; j++) {
        anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_b[j] = (int) (scale * factor * ba[j]);
        anc_coef_list_50p7k[1]->anc_cfg_tt_l.iir_coef[idx].coef_a[j] = (int) (scale * ba[3+j]);
    }
}
