/* 
Direct Form II filters with design of common filers (low/high-pas, notch)
basile dot graf at a3 dot epfl dot ch
*/

#ifndef _FILTER_H_

#include "m_pd.h"

#define MAX_FILTER_ORDER 5  // Should be minimum 2
#define MAX_FILTER_NUM_PARAM 3  
#define PI 3.14159265358979324
#define TWOPI 6.28318530718

typedef double t_fsample;

enum e_filter_type {
    e_filter_gain = 0,
    e_filter_lp2,
    e_filter_hp2,
    e_filter_lp1,
    e_filter_hp1,
    e_filter_n,
    e_filter_num_types
};

enum e_ramp_type {
    e_ramp_lin,
    e_ramp_exp
};

enum e_set_filter {
    e_set_filter_coeffs,
    e_set_filter_coeffs_target
};

typedef struct _filter {
    t_int order, n;
    t_fsample b[MAX_FILTER_ORDER+1];	// nuemrator (b coeffs) , size order+1
    t_fsample a[MAX_FILTER_ORDER];	// denomintaor (a coeffs), size order
    t_fsample v[MAX_FILTER_ORDER+1]; 	// state, size order+1
    // Target filter coefficients (reached after a ramp). Used for delay length compensation:
    t_fsample b_target[MAX_FILTER_ORDER+1];	// nuemrator (b coeffs) , size order+1
    t_fsample a_target[MAX_FILTER_ORDER];	// denomintaor (a coeffs), size order
    // Variables for smooth parameter change:
    enum e_filter_type type;
    t_fsample h;
    t_int n_param_steps;
    t_fsample param[MAX_FILTER_NUM_PARAM];
    t_fsample param_step[MAX_FILTER_NUM_PARAM];
    t_fsample param_target[MAX_FILTER_NUM_PARAM]; // Used to relpace last step in case of inaccurate ramping
    enum e_ramp_type param_ramptype[MAX_FILTER_NUM_PARAM];
} t_filter;



void filter_init(t_filter* filt);
void filter_free(t_filter* filt);
void filter_step(t_filter* filt, t_float x, t_float* y);

// filter designs

// Static gain
void filter_gain(t_filter* filt, t_float g);
// 2nd order lowpass with cutoff freq f, damping z and sample time h
void filter_lp2(t_filter* filt, t_float f, t_float z, t_float h);
// 2nd order highpass with cutoff freq f, damping z and sample time h
void filter_hp2(t_filter* filt, t_float f, t_float z, t_float h);
// 1st order lowpass with cutoff freq f and sample time h
void filter_lp1(t_filter* filt, t_float f, t_float h);
// 1st order highpass with cutoff freq f and sample time h
void filter_hp1(t_filter* filt, t_float f, t_float h);
// Notch filter with frequency f, gain g, bandwidth b and sample time h
void filter_n(t_filter* filt, t_float f, t_float g, t_float b, t_float h);

void filter_x(t_filter* filt, enum e_set_filter e_set_to);

// Bode transfer function of filter filt evaluated at normalized frequency f = freq/fsampling in [0, 1]
// e_set: choose between current filter coeffs and and target filter coeffs (after ramp)
void filter_bode(t_filter* filt,  t_float f, enum e_set_filter e_set, t_float* mag, t_float* phase);


#endif /* _FILTER_H_ */
