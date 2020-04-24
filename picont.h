/* 
Proportional-Integral (PI) controller with integrator state saturation
basile dot graf at a3 dot epfl dot ch
*/

#ifndef _PI_CONT_H_

#include "m_pd.h"

#define PI 3.14159265358979324

typedef struct _picont {
    t_float kp;         // Proportional gain
    t_float ki;         // Integral gain
    t_float int_sat;    // Integrator max absolute value
    t_float int_state;  // integrator state
} t_picont;



void picont_init(t_picont* cont, t_float kp, t_float ki, t_float sat);
void picont_free(t_picont* cont);
void picont_step(t_picont* cont, t_float x, t_float* y, t_float h);
void picont_reset(t_picont* cont);

#endif /* _PI_CONT_H_ */