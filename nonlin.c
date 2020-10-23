/* 
Symmetric and asymmetric saturating nonlinearities
basile dot graf at a3 dot epfl dot ch
*/

#include "nonlin.h"
#include <stdio.h>      
#include <stdlib.h>  
#include <math.h>

int dummy = 0;


// Initialize 
void nonlin_init(t_nonlin* nl, t_float sample_time)
{
    nl->sample_time = sample_time;
    
    // Variables for gain ramping
    nl->gain = 0.0f;           // Current gain value
    nl->gain_step_size = 0.0f; // Gain ramp incement size 
    nl->gain_n_step = 0;       // Remaining number of steps to apply
    
    // Modulation 
    nl->gain_modulation = 0.0f;
    
    nl->type = e_symmetric_sat;
    nl->saturation = 10.0f;
}
 
// Free 
void nonlin_free(t_nonlin* nl)
{
    // nothing to do
}


// Set the type, gain, saturation and ramp time
// Note, negative and zero gain allowed
//       negative saturation same as positive
void nonlin_set(t_nonlin* nl, t_int type, t_float gain, t_float sat, t_float ramp_time)
{
    t_float ramp_amplitude;
    
    if (type < 0 || type >= e_nonlin_num_types)
    {
       error("nonlin : undefined type number.");
       return;
    }
    nl->type = type;
    nl->saturation = sat;
    
    ramp_amplitude = gain - nl->gain;
    if (ramp_time <= 0.0f)
    {
        nl->gain_step_size = ramp_amplitude;
        nl->gain_n_step = 1;
    }
    else
    {
        nl->gain_n_step = (t_int) roundf(ramp_time / nl->sample_time);
        nl->gain_n_step = nl->gain_n_step >= 1 ? nl->gain_n_step : 1;
        nl->gain_step_size = ramp_amplitude / ((t_float) nl->gain_n_step);
    }
}

// 1 setp
void nonlin_step(t_nonlin* nl, t_float x, t_float* y)
{
    t_float g, s;
    
    // Ramp gain value
    if (nl->gain_n_step > 0)
    {
        nl->gain += nl->gain_step_size;
        nl->gain_n_step--;
    }
    
    // Modulate gain, but prevent sign change
    if (nl->gain > 0.0f) 
    {
        g = fmaxf(nl->gain + nl->gain_modulation, 0.0f);
    }
    else
    {
        g = fminf(nl->gain + nl->gain_modulation, 0.0f);
    }
    
    s = nl->saturation;
    
    switch (nl->type)
    {
        case e_symmetric_sat:
            *y = g * x;
            *y = *y > s ? s : (*y < -s ? -s : *y);
        break;
        case e_asymmetric_sat:
            *y = g * x;
            *y = *y > s ? s : *y;
        break;
        case e_symmetric_sigmoid:
            *y = tanhf((g * x) / s);
        break;
        case e_asymmetric_sigmoid:
            *y = x < 0.0f ? g * x : tanhf((g * x) / s);
        break;
        default:
            *y = g * x; // should not happen
        break;
    }
    
    if (dummy == 0)
    {
        post("gain = %f", nl->gain);
    }
    dummy++;
    if (dummy > 10000) dummy = 0;
}


void nonlin_print(t_nonlin* nl)
{
    switch (nl->type)
    {
        case e_symmetric_sat:
            post("nonlin: Symmetric saturation gain=%f, saturation=%f", nl->gain, nl->saturation);
        break;
        case e_asymmetric_sat:
            post("nonlin: Asymmetric saturation gain=%f, saturation=%f", nl->gain, nl->saturation);
        break;
        case e_symmetric_sigmoid:
            post("nonlin: Symmetric sigmoid gain=%f, saturation=%f", nl->gain, nl->saturation);
        break;
        case e_asymmetric_sigmoid:
            post("nonlin: Asymmetric sigmoid gain=%f, saturation=%f", nl->gain, nl->saturation);
        break;
        default:
            post("nonlin: Undefined nonlinearity, using linear gain=%f", nl->gain);
        break;
    }
    //post("nonlin: gain       = %f", nl->gain);
    //post("nonlin: saturation = %f", nl->saturation);
}
 
