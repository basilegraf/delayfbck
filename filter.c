#include "filter.h"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* calloc, exit, free */





// filt->b = (t_fsample*) calloc(order+1,sizeof(t_fsample));
//if ((filt->b==NULL) || (filt->a==NULL) || (filt->v==NULL))
//    {
//        post("delayfbck: memory alloc failed!");
//    }

void filter_init(t_filter* filt, t_int order)
{
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
    //free(filt->b);
    //free(filt->a);
    //free(filt->v);
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
    t_float a0i = 1.0 / (4.0 + hw * (hw + 4.0*z));
    filt->b[0] = a0i * 4.0 * (1.0 + hw * z);
    filt->b[1] = a0i * (-8.0);
    filt->b[2] = a0i * 4.0 * (1.0 - hw * z);

    filt->a[0] = a0i * (-8.0 + 2.0 * hw * hw);
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


