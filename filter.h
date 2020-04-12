
#ifndef _FILTER_H_

#include "m_pd.h"

typedef struct _filter {
    t_int order, n;
    double* b;		// nuemrator (b coeffs) , size order+1
    double* a;		// denomintaor (a coeffs), size order
    double* v; 	// state, size order+1
} t_filter;



void filter_init(t_filter* filt, t_int order);
void filter_free(t_filter* filt);
void filter_step(t_filter* filt, t_float x, t_float* y);


#endif /* _FILTER_H_ */
