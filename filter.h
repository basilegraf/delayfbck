
#ifndef _FILTER_H_

#include "m_pd.h"

#define MAX_FILTER_ORDER 5  // Should be minimum 2
#define PI 3.14159265358979324

typedef double t_fsample;

typedef struct _filter {
    t_int order, n;
    t_fsample b[MAX_FILTER_ORDER+1];	// nuemrator (b coeffs) , size order+1
    t_fsample a[MAX_FILTER_ORDER];	// denomintaor (a coeffs), size order
    t_fsample v[MAX_FILTER_ORDER+1]; 	// state, size order+1
} t_filter;



void filter_init(t_filter* filt, t_int order);
void filter_free(t_filter* filt);
void filter_step(t_filter* filt, t_float x, t_float* y);

// filter designs

// 2nd order lowpass with cutoff freq f, damping z and sample time h
void filter_lp2(t_filter* filt, t_float f, t_float z, t_float h);
// 2nd order highpass with cutoff freq f, damping z and sample time h
void filter_hp2(t_filter* filt, t_float f, t_float z, t_float h);
// 1st order lowpass with cutoff freq f and sample time h
void filter_lp1(t_filter* filt, t_float f, t_float h);
// 1st order highpass with cutoff freq f and sample time h
void filter_hp1(t_filter* filt, t_float f, t_float h);


#endif /* _FILTER_H_ */
