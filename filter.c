#include "filter.h"
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* calloc, exit, free */


// https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html


void filter_init(t_filter* filt, t_int order)
{
    filt->order = order;
    filt->n = 0;
    filt->b = (t_float*) calloc(order+1,sizeof(t_float));
    filt->a = (t_float*) calloc(order,sizeof(t_float));
    filt->v = (t_float*) calloc(order+1,sizeof(t_float));
    
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
    t_int n = filt->n;

    filt->v[n] = x;
    for (t_int i=0; i<ord; i++)
    {
        filt->v[n] -= filt->a[i] * filt->v[(ord + n - i - 1) % ord];
    }
	
    *y = filt->b[0] * filt->v[n];
    for (t_int i=0; i<ord; i++)
    {
        *y += filt->b[i+1] * filt->v[(ord + n - i - 1) % ord];
    }

    filt->n = (n + 1)  % ord;
    
}
