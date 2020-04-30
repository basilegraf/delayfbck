
#include "picont.h"
#include <stdio.h>     
#include <stdlib.h>     
#include <math.h>


/*typedef struct _picont {
    t_float kp;         // Proportional gain
    t_float ki;         // Integral gain
    t_float int_sat;    // Integrator max absolute value
    t_float int_state;  // integrator state
} t_picont;*/



void picont_init(t_picont* cont, t_float kp, t_float ki, t_float sat)
{
    cont->kp = kp;
    cont->ki = ki;
    cont->int_sat = sat;
    cont->int_state = 0.0;
    return;
}

void picont_free(t_picont* cont)
{
    return;
}


void picont_step(t_picont* cont, t_float x, t_float* y, t_float h)
{
    cont->int_state += cont->ki * h * x;
    cont->int_state = fminf(cont->int_state,  cont->int_sat);
    cont->int_state = fmaxf(cont->int_state, -cont->int_sat);
    *y = cont->kp * x + cont->int_state;
    return;
}

void picont_reset(t_picont* cont)
{
    cont->int_state = 0.0;
    return;
}
