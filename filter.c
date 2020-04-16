/* 
Direct Form II filters with design of common filers (low/high-pas, notch)
basile dot graf at a3 dot epfl dot ch
*/


#include "filter.h"
#include <stdio.h>     
#include <stdlib.h>     
#include <math.h>







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
}



void filter_free(t_filter* filt)
{
}


// https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html
void filter_step(t_filter* filt, t_float x, t_float* y)
{
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
    filt->order = 0;
    filt->b[0] = g;
}


// 2nd order lowpass with freq f and damping z
void filter_lp2(t_filter* filt, t_float f, t_float z, t_float h)
{
    filt->order = 2;
    t_float w = 2.0 * PI * f;
    t_float hw = h * w;
    t_float hw2 = hw * hw;
    t_float a0i = 1.0 / (4.0 + hw * (hw + 4.0*z));
    filt->b[0] = a0i * hw2;
    filt->b[1] = 2.0 * filt->b[0];
    filt->b[2] = filt->b[0];

    filt->a[0] = a0i * (-8.0 + 2.0 * hw2);
    filt->a[1] = a0i * (4.0 + hw * (hw - 4.0*z));
}


// 2nd order highpass with freq f, damping z and sample time h
void filter_hp2(t_filter* filt, t_float f, t_float z, t_float h)
{
    filt->order = 2;
    t_float w = 2.0 * PI * f;
    t_float hw = h * w;
    t_float hw2 = hw * hw;
    t_float a0i = 1.0 / (4.0 + hw * (hw + 4.0*z));
    filt->b[0] = a0i * 4.0 ;
    filt->b[1] = a0i * (-8.0);
    filt->b[2] = a0i * 4.0 ;

    filt->a[0] = a0i * (-8.0 + 2.0 * hw2);
    filt->a[1] = a0i * (4.0 + hw * (hw - 4.0*z));    
}

// 1st order lowpass with cutoff freq f and sample time h
void filter_lp1(t_filter* filt, t_float f, t_float h)
{
    filt->order = 1;
    t_float w = 2.0 * PI * f;
    t_float hw = h * w;
    t_float a0i = 1.0 / (2.0 + hw);
    filt->b[0] = a0i * hw;
    filt->b[1] = filt->b[0];

    filt->a[0] = a0i * (-2.0 + hw);
}

// 1st order highpass with cutoff freq f and sample time h
void filter_hp1(t_filter* filt, t_float f, t_float h)
{
    filt->order = 1;
    t_float w = 2.0 * PI * f;
    t_float hw = h * w;
    t_float a0i = 1.0 / (2.0 + hw);
    filt->b[0] = a0i * 2.0;
    filt->b[1] = -filt->b[0];

    filt->a[0] = a0i * (-2.0 + hw);
}


// Notch filter with frequency f, gain g, bandwidth b and sample time h
void filter_n(t_filter* filt, t_float f, t_float g, t_float bHz, t_float h)
{
    filt->order = 2;
    t_float w = 2.0 * PI * f;
    t_float b = 2.0 * PI * bHz;
    t_float hw = h * w;
    t_float hw2 = hw * hw;
    t_float bh = b * h;
    t_float sqg = sqrt(g);
	
    t_float a0i = 1.0 / (2.0*bh + sqg*(4+hw2));

    filt->b[0] = a0i * sqg * (4.0 + 2.0*b*sqg*h + hw2);
    filt->b[1] = a0i * 2.0 * sqg *(-4.0 + hw2);
    filt->b[2] = a0i * sqg * (4.0 - 2.0*b*sqg*h + hw2);

    filt->a[0] = a0i * (2.0*sqg*(-4.0 + hw2));
    filt->a[1] = a0i * ((-2.0*bh + sqg*(4 + hw2)));
}

