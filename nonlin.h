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
    t_int type;	        	// type of nonlinearity
    t_float saturation;			
    t_float gain;
} t_nonlin;


// Initilize 
void nonlin_init(t_nonlin* nl);
// Free 
void nonlin_free(t_nonlin* nl);

// Set the type, gain and saturation
int nonlin_set(t_nonlin* nl, t_int type, t_float gain, t_float sat);
// 1 setp
void nonlin_step(t_nonlin* nl, t_float x, t_float* y);

void nonlin_print(t_nonlin* nl);

#endif /* _NONLIN_H_ */
