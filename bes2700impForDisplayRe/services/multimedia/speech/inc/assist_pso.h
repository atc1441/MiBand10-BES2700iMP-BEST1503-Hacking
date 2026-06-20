/***************************************************************************
 *
 * Copyright 2015-2024 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __PSO_H__
#define __PSO_H__

// === CONSTANTS ===
#define PSO_MAX_SIZE 100 // max swarm size
#define PSO_INERTIA 0.9 //0.7298 // default value of w (see clerc02)


// === INERTIA WEIGHT UPDATE METHODS ===
#define PSO_W_CONST 0
#define PSO_W_LIN_DEC 1
#define PSO_W_EXP_DEC 2

// === PSO SOLUTION -- Initialized by the user ===
typedef struct {

    float error;
    float *gbest; // should contain DIM elements!!

} pso_result_t;

typedef struct
{
    float** pos;
    float** vel;
    float** pos_b;
    float* fit;
    float* fit_b;
} assist_pso;


// === PSO SETTINGS ===
typedef struct {

    int dim; // problem dimensionality
    float *range_lo; // lower range limit (array of length DIM)
    float *range_hi; // higher range limit (array of length DIM)
    float *range;
    float tol; // optimization goal (error threshold)

    int size; // swarm size (number of particles)
    int print_every; // ... N steps (set to 0 for no output)
    int steps; // maximum number of iterations
    int plateau; // allowed maximum number of iters without error decreasing
    int consecutive_failed;
    int step; // current PSO step
    float c1; // cognitive coefficient
    float c2; // social coefficient
    float w_max; // max inertia weight value
    float w_min; // min inertia weight value

    int clamp_pos; // whether to keep particle position within defined bounds (TRUE)
    int w_strategy; // inertia weight strategy (see PSO_W_*)
    float delta_w; // used when exp_decay works

    float* init_v; // initial value
    int has_init;
    int improved;
    int best_idx;

    // specific membors for ==========eq_adjustment============ hereafter;
    int num_points; // the number of freq points
    int sr; // sample rate
    float left_bound; // freq range
    float right_bound;
    int idx_low_freq;
    // space for time
    float* ba;
    float* cur_adjust_fr;
    float* tp_vec_cost;
    float* sinw;
    float* sin2w;
    float* cosw;
    float* cos2w;

    // float* xminusY;
    float* freq;
    float* target;
    float* origin_freq;
    float* origin_target;

    assist_pso* pso_buffer;
} pso_settings_t;

pso_settings_t *pso_settings_new(int dim, float *range_lo, float *range_hi, pso_settings_t* settings);
void pso_settings_free(pso_settings_t *settings);


// === OBJECTIVE FUNCTION TYPE ===
typedef float (*pso_obj_fun_t)(const float *, int, void *, pso_settings_t*);

// === PSO_SOLVER ===
// minimize the provided obj_fun using PSO with the specified settings
// and store the result in *solution
void pso_solve(pso_obj_fun_t obj_fun, void *obj_fun_params,
	       pso_result_t *solution, pso_settings_t *settings, int start_step, int particle_pieces);


// utils
// float **pso_matrix_new(int size, int dim);
// void pso_matrix_free(float **m, int size);
void print_float(float v);

#endif // PSO_H_