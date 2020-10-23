/* 
Symmetric and asymmetric saturating nonlinearities
basile dot graf at a3 dot epfl dot ch
*/


#ifndef _NONLIN_H_

#include "m_pd.h"


enum e_nonlin_type {
    e_symmetric_sat = 0,
    e_asymmetric_sat,
    e_symmetric_sigmoid,
    e_asymmetric_sigmoid,
    e_nonlin_num_types
};

typedef struct _nonlin {
    
    t_float sample_time;
    
    // Variables for gain ramping
    t_float gain;           // Current gain value
    t_float gain_step_size; // Gain ramp incement size 
    t_int gain_n_step;      // Remaining number of steps to apply
    
    // Additive gain modulation, i.e. gain => gain + gain_modulation 
    t_float gain_modulation;
    
    t_int type;	        	// type of nonlinearity
    t_float saturation;			
    
} t_nonlin;


// Initilize 
void nonlin_init(t_nonlin* nl, t_float sample_time);
// Free 
void nonlin_free(t_nonlin* nl);

// Set the type, gain, saturation and ramp time
void nonlin_set(t_nonlin* nl, t_int type, t_float gain, t_float sat, t_float ramp_time);
// 1 setp
void nonlin_step(t_nonlin* nl, t_float x, t_float* y);

void nonlin_print(t_nonlin* nl);

#endif /* _NONLIN_H_ */
