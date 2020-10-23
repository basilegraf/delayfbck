/* 
Variable delay line with linearly interpolated tap.
basile dot graf at a3 dot epfl dot ch
*/


#ifndef _DELAY_H_

#include "m_pd.h"

#define PI 3.14159265358979324

enum e_pluck_state {
    e_pluck_idle,
    e_pluck_rising,
    e_pluck_falling
};

typedef struct _delay {
    
    t_float sample_time;
    
    // Variables for delay length ramping
    t_float delay_value;    // Current delay length
    t_float delay_step_size;// Delay ramp incement size 
    t_int delay_n_step;     // Remaining number of steps to apply
    
    // Modulation (additive on delay length)
    t_float delay_modulation;
    
    // Variables for "plucked string"
    enum e_pluck_state plk_state;
    t_float plk_amplitude;
    t_float plk_rising_step;
    t_float plk_falling_step;
    t_float plk_value;
    
    // Variables for delay implementation
    t_int maxLength, n;	    // Allocated length, current index
    t_int integer;	        // Integer part f the delay 0 <= integer < maxLength
    t_float decimal;	    // Decimal part of the delay 0.0 <= decimal <= 1.0
    t_float* v; 	        // state, dynamic alloc
    
} t_delay;


// Initilize a delay of max length samples long
void delay_init(t_delay* del, t_float sample_time, t_int maxLength);
// Free/deallocate delay memory
void delay_free(t_delay* del);
// Reset delay state
void delay_reset(t_delay* del);

// Set the delay line length
void delay_set_duration(t_delay* del, t_float dur, t_float ramp_time);


// write at current index
void delay_write(t_delay* del, t_float x);
// read relative to current index
void delay_read(t_delay* del, t_float* y);
// increment index
void delay_step(t_delay* del);

void delay_print(t_delay* del);

// pluck string à la Karplus-Strong with amplitude ampl at position 0.0 <= pos <= 1.0
void delay_pluck_string(t_delay* del, t_float ampl, t_float pos);


// Private methods

// Set duration in dur in seconds, sample time h
void delay_set_delayline_duration_(t_delay* del, t_float dur); 

#endif /* _DELAY_H_ */
