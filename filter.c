#include "filter.h"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* calloc, exit, free */


// https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html


void filter_init(t_filter* filt, t_int order)
{
    filt->order = order;
    filt->n = 0;
    filt->b = (double*) calloc(order+1,sizeof(double));
    filt->a = (double*) calloc(order,sizeof(double));
    filt->v = (double*) calloc(order+1,sizeof(double));
    
    if ((filt->b==NULL) || (filt->a==NULL) || (filt->v==NULL))
    {
        post("delayfbck: memory alloc failed!");
    }
}



void filter_free(t_filter* filt)
{
    free(filt->b);
    free(filt->a);
    free(filt->v);
}


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
