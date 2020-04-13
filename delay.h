/* 
Variable delay line with linearly interpolated tap.
basile dot graf at a3 dot epfl dot ch
*/


#ifndef _DELAY_H_

#include "m_pd.h"

#define PI 3.14159265358979324


typedef struct _delay {
    t_int maxLength, n;	// Allocated length, current index
    t_int integer;	// Integer part f the delay 0 <= integer < maxLength
    t_float decimal;	// Decimal part of the delay 0.0 <= decimal <= 1.0
    t_float* v; 	// state, dynamic alloc
} t_delay;


// Initilize a delay of max length samples long
void delay_init(t_delay* del, t_int maxLength);
// Free/deallocate delay memory
void delay_free(t_delay* del);
// Reset delay state
void delay_reset(t_delay* del);
// Set duration in dur in seconds, sample time h
void delay_set_duration(t_delay* del, t_float dur, t_float h); 

// write at current index
void delay_write(t_delay* del, t_float x);
// read relative to current index
void delay_read(t_delay* del, t_float* y);
// increment index
void delay_step(t_delay* del);

void delay_print(t_delay* del);

#endif /* _DELAY_H_ */
