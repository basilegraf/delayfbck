/* 
Symmetric and asymmetric saturating nonlinearities
basile dot graf at a3 dot epfl dot ch
*/

#include "nonlin.h"
#include <stdio.h>      
#include <stdlib.h>  
#include <math.h>



// Initialize 
void nonlin_init(t_nonlin* nl)
{
    nl->type = e_symmetric_sat;
    nl->gain = 1.0f;
    nl->saturation = 10.0f;
}
 
// Free 
void nonlin_free(t_nonlin* nl)
{
    // nothing to do
}


// Set the type, gain and saturation
// Note, negative and zero gain allowed
//       negative saturation same as positive
int nonlin_set(t_nonlin* nl, t_int type, t_float gain, t_float sat)
{
    if (type < 0 || type >= e_nonlin_num_types)
    {
       error("nonlin : undefined type number.");
       return -1;
    }
    nl->type = type;
    nl->gain = gain;
    nl->saturation = sat;
    return 0;
}

// 1 setp
void nonlin_step(t_nonlin* nl, t_float x, t_float* y)
{
    t_float g = nl->gain;
    t_float s = nl->saturation;
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
 
