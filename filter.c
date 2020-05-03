/* 
Direct Form II filters with design of common filers (low/high-pas, notch)
basile dot graf at a3 dot epfl dot ch
*/


#include "filter.h"
#include <stdio.h>     
#include <stdlib.h>     
#include <math.h>
#include <stdbool.h> // _Bool






void filter_init(t_filter* filt, t_int order)
{
    if (order > MAX_FILTER_ORDER)
    {
        error("filter: given order was %d max order is %d",(int) order,(int) MAX_FILTER_ORDER);
        order = MAX_FILTER_ORDER;
    }
    filt->order = order;
    filt->n = 0;
    for (int k=0; k<MAX_FILTER_ORDER; k++)
    {
        filt->b[k] = 0.0;
        filt->a[k] = 0.0;
        filt->v[k] = 0.0;
    }
    filt->b[0] = 1.0;
    filt->b[MAX_FILTER_ORDER] = 0.0;
    filt->v[MAX_FILTER_ORDER] = 0.0;
    
    filt->type = e_filter_num_types; // This corresponds to no actual type => Will prevent ramping at first set
    filt->h = 1.0 / 44100.0; // TODO ??
    filt->n_param_steps = 0;
    for (int k=0; k<MAX_FILTER_NUM_PARAM; k++)
    {
        filt->param[k] = 0.0;
        filt->param_step[k] = 0.0;
        filt->param_target[k] = 0.0;
        filt->param_ramptype[k] = e_ramp_lin;
    }
}



void filter_free(t_filter* filt)
{
}


// https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html
void filter_step(t_filter* filt, t_float x, t_float* y)
{
    // Ramp parameters linearly or exponentially
    if (filt->n_param_steps == 1)
    {
        for (t_int k = 0; k < MAX_FILTER_NUM_PARAM; k++)
        {
            filt->param[k] = filt->param_target[k];
        }
        filter_x(filt);
        filt->n_param_steps = 0;
    }
    else if (filt->n_param_steps > 1)
    {
        for (t_int k = 0; k < MAX_FILTER_NUM_PARAM; k++)
        {
            if (filt->param_ramptype == e_ramp_lin)
            {
                filt->param[k] += filt->param_step[k];
            }
            else
            { // e_ramp_exp
                filt->param[k] *= filt->param_step[k];
            }
            
        }
        filter_x(filt);
        filt->n_param_steps--;
    }
    
    // Filter
    t_int ord = filt->order;
    t_int vLen = filt->order + 1;
    t_int n = filt->n;
    t_int i;

    filt->v[n] = x;
    for (i=0; i<ord; i++)
    {
        filt->v[n] -= filt->a[i] * filt->v[(vLen + n - i - 1) % vLen];
    }
	
    *y = filt->b[0] * filt->v[n];
    for (i=0; i<ord; i++)
    {
        *y += filt->b[i+1] * filt->v[(vLen + n - i - 1) % vLen];
    }

    filt->n = (n + 1)  % vLen;
    
}


// Static gain
void filter_gain(t_filter* filt, t_float g)
{
    filt->type = e_filter_gain;
    filt->param[0] = g;
    filt->param[1] = 0.0;
    filt->param[2] = 0.0;
    filter_x(filt);
}


// 2nd order lowpass with freq f and damping z
void filter_lp2(t_filter* filt, t_float f, t_float z, t_float h)
{
    filt->type = e_filter_lp2;
    filt->h = h;
    filt->param[0] = f;
    filt->param[1] = z;
    filt->param[2] = 0.0;
    filter_x(filt);
}


// 2nd order highpass with freq f, damping z and sample time h
void filter_hp2(t_filter* filt, t_float f, t_float z, t_float h)
{
    filt->type = e_filter_hp2;
    filt->h = h;
    filt->param[0] = f;
    filt->param[1] = z;
    filt->param[2] = 0.0;
    filter_x(filt);   
}

// 1st order lowpass with cutoff freq f and sample time h
void filter_lp1(t_filter* filt, t_float f, t_float h)
{
    filt->type = e_filter_lp1;
    filt->h = h;
    filt->param[0] = f;
    filt->param[1] = 0.0;
    filt->param[2] = 0.0;
    filter_x(filt);
}

// 1st order highpass with cutoff freq f and sample time h
void filter_hp1(t_filter* filt, t_float f, t_float h)
{
    filt->type = e_filter_hp1;
    filt->h = h;
    filt->param[0] = f;
    filt->param[1] = 0.0;
    filt->param[2] = 0.0;
    filter_x(filt);
}


// Notch filter with frequency f, gain g, bandwidth b and sample time h
void filter_n(t_filter* filt, t_float f, t_float g, t_float bHz, t_float h)
{
    filt->type = e_filter_n;
    filt->h = h;
    filt->param[0] = f;
    filt->param[1] = g;
    filt->param[2] = bHz;
    filter_x(filt);
}




void filter_x(t_filter* filt)
{
    t_fsample h = filt->h;
    t_float f, z, w, hw, hw2, a0i, g, b, bHz, bh, sqg;
    switch(filt->type)
    {
        case e_filter_gain:
        // Static gain g
        // param = {g}
            filt->order = 0;
            filt->b[0] = filt->param[0]; // gain g
            break;

        case e_filter_lp2:
        // 2nd order lowpass with freq f and damping z
        // param = {f, z}
            f = filt->param[0];
            z = filt->param[1];
            filt->order = 2;
            w = 2.0 * PI * f;
            hw = h * w;
            hw2 = hw * hw;
            a0i = 1.0 / (4.0 + hw * (hw + 4.0*z));
            filt->b[0] = a0i * hw2;
            filt->b[1] = 2.0 * filt->b[0];
            filt->b[2] = filt->b[0];

            filt->a[0] = a0i * (-8.0 + 2.0 * hw2);
            filt->a[1] = a0i * (4.0 + hw * (hw - 4.0*z));
            break;

        case e_filter_hp2:
        // 2nd order highpass with freq f, damping z
        // param = {f, z}
            f = filt->param[0];
            z = filt->param[1];
            filt->order = 2;
            w = 2.0 * PI * f;
            hw = h * w;
            hw2 = hw * hw;
            a0i = 1.0 / (4.0 + hw * (hw + 4.0*z));
            filt->b[0] = a0i * 4.0 ;
            filt->b[1] = a0i * (-8.0);
            filt->b[2] = a0i * 4.0 ;

            filt->a[0] = a0i * (-8.0 + 2.0 * hw2);
            filt->a[1] = a0i * (4.0 + hw * (hw - 4.0*z));
            break;

        case e_filter_lp1:
        // 1st order lowpass with cutoff freq f 
        // param = {f}
            f = filt->param[0];
            filt->order = 1;
            w = 2.0 * PI * f;
            hw = h * w;
            a0i = 1.0 / (2.0 + hw);
            filt->b[0] = a0i * hw;
            filt->b[1] = filt->b[0];

            filt->a[0] = a0i * (-2.0 + hw);
            break;

        case e_filter_hp1:
        // 1st order highpass with cutoff freq f 
        // param = {f}
            f = filt->param[0];
            filt->order = 1;
            w = 2.0 * PI * f;
            hw = h * w;
            a0i = 1.0 / (2.0 + hw);
            filt->b[0] = a0i * 2.0;
            filt->b[1] = -filt->b[0];

            filt->a[0] = a0i * (-2.0 + hw);
                break;

        case e_filter_n:
        // Notch filter with frequency f, gain g, bandwidth b
        // param =  {f, g, bHz}
            f = filt->param[0];
            g = filt->param[1];
            bHz = filt->param[2];
            filt->order = 2;
            w = 2.0 * PI * f;
            b = 2.0 * PI * bHz;
            hw = h * w;
            hw2 = hw * hw;
            bh = b * h;
            sqg = sqrt(g);
            
            a0i = 1.0 / (2.0*bh + sqg*(4+hw2));

            filt->b[0] = a0i * sqg * (4.0 + 2.0*b*sqg*h + hw2);
            filt->b[1] = a0i * 2.0 * sqg *(-4.0 + hw2);
            filt->b[2] = a0i * sqg * (4.0 - 2.0*b*sqg*h + hw2);

            filt->a[0] = a0i * (2.0*sqg*(-4.0 + hw2));
            filt->a[1] = a0i * ((-2.0*bh + sqg*(4 + hw2)));
            break;

        default:
          error("filter: Unknown filter type.");
    }
}


// Bode of a polynomial c transfer function of degree n, evaluated at normalized frequency f = freq/fsampling in [0, 1]
// c order is descending powers
// If hasLead is flase, n+1 values are taken from c[]
// If hasLead is true, the polynomial is prepended with 1.0 and only n values are taken from c[]
void polynom_bode(t_fsample* c, t_int n,  t_float f, bool hasLead, t_float* mag, t_float* phase)
{
    t_float w = TWOPI * f;
    t_float real = 0.0;
    t_float imag = 0.0;
    if (hasLead)
    {
        real = cosf(((t_float)(n)) * w);
        imag = sinf(((t_float)(n)) * w);
        for (t_int k=0; k<n; k++)
        {
            real += c[k] * cosf(((t_float)(n-k-1)) * w);
            imag += c[k] * sinf(((t_float)(n-k-1)) * w);
        }
    }
    else
    {
        for (t_int k=0; k<=n; k++)
        {
            real += c[k] * cosf(((t_float)(n-k)) * w);
            imag += c[k] * sinf(((t_float)(n-k)) * w);
        }
    }    
    *mag = sqrtf(real*real + imag*imag);
    *phase = atan2f(imag, real);
}


// Bode of a filter filt transfer function, evaluated at normalized frequency f = freq/fsampling in [0, 1]
void filter_bode(t_filter* filt,  t_float f, t_float* mag, t_float* phase)
{
    t_float magNum, magDen, phaseNum, phaseDen;

    polynom_bode(filt->b, filt->order, f, false, &magNum, &phaseNum);
    polynom_bode(filt->a, filt->order, f, true, &magDen, &phaseDen);
    *mag = magNum / magDen;
    *phase = phaseNum - phaseDen;
}