/* 
Variable delay line with linearly interpolated tap.
basile dot graf at a3 dot epfl dot ch
*/

#include "delay.h"
#include <stdio.h>      
#include <stdlib.h>  
#include <math.h>


void delay_print(t_delay* del)
{
    post("maxLength = %d", del->maxLength);
    post("n         = %d", del->n);
    post("integer   = %d", del->integer);
    post("decimal   = %f", del->decimal);
}


// Initilize a delay of max length samples long
void delay_init(t_delay* del, t_int maxLength)
{
    del->integer = 0;
    del->n = 0;
    del->decimal = 0.0f;
    if (maxLength < 0)
    {
        del->maxLength = 0;
        error("delay: max delay length should be non-negative");
    }
    del->maxLength = maxLength;
    del->v = (t_float*) malloc((unsigned int) maxLength * sizeof(t_float));
    if (del->v == NULL)
    {
        del->maxLength = 0;
        error("delay: memory alloc failed!");
    }
    delay_reset(del);
    // Plucked string
    del->plk_state = e_pluck_idle;
    del->plk_amplitude = 0.0f;
    del->plk_rising_step = 0.0f;
    del->plk_falling_step = 0.0f;
    del->plk_value = 0.0f;
}

// Free/deallocate delay memory
void delay_free(t_delay* del)
{
    del->integer = 0;
    del->n = 0;
    del->decimal = 0.0f;
    del->maxLength = 0;
    free(del->v);
}

// Reset delay state
void delay_reset(t_delay* del)
{
    for (int k=0; k<del->maxLength; k++)
    {
        del->v[k] = 0.0f;
    }
}

// Set duration dur in seconds, sample time h
void delay_set_duration(t_delay* del, t_float dur, t_float h)
{
    float intpartf;
    if (dur < 0.f) dur = 0.0f;
    if (h <= 0.0f) error("delay: sample time must be strictly positive");
    
    del->decimal =  modff(dur / h, &intpartf);
    del->integer = (t_int) intpartf;

    if (del->integer >= del->maxLength)
    {
        del->decimal = 0.0f;
        del->integer = del->maxLength - 1;
    }
}

// write at current index
void delay_write(t_delay* del, t_float x)
{
    del->v[del->n] = x;
}

// read relative to current index
void delay_read(t_delay* del, t_float* y)
{
    t_int n = del->n;
    t_int vLen = del->maxLength;
    t_float tapn, tapn1;
    tapn  = del->v[(vLen + n - del->integer) % vLen];
    tapn1 = del->v[(vLen + n - del->integer - 1) % vLen];
    *y = (1.0f - del->decimal) * tapn + del->decimal * tapn1;
}


void delay_step(t_delay* del)
{
    
    
    // plucked string   
    switch (del->plk_state)
    {
        case e_pluck_rising:
            del->plk_value += del->plk_rising_step;
            if (del->plk_value >= del->plk_amplitude)
            {
                del->plk_value = del->plk_amplitude;
                del->plk_state = e_pluck_falling;
            }
            del->v[del->n] += del->plk_value;
        break;
        case e_pluck_falling:
            del->plk_value -= del->plk_falling_step;
            if (del->plk_value <= 0.0f)
            {
                del->plk_value = 0.0f;
                del->plk_state = e_pluck_idle;
            }
            del->v[del->n] += del->plk_value;
        break;
        case e_pluck_idle:
            // nothing
        break;
    }
    
    // increment index
    del->n = (del->n + 1)  % del->maxLength;
}


// pluck string à la Karplus-Strong with amplitude ampl at position 0.0 <= pos <= 1.0
void delay_pluck_string(t_delay* del, t_float ampl, t_float pos)
{
     t_float L = del->decimal + (t_float) del->integer;
     pos = pos >= 0.0f ? pos : 0.0f;
     pos = pos <= 1.0f ? pos : 1.0f;
     pos *= L; // pluck position in samples
     if (pos > 0.0f)
     {
         del->plk_rising_step = ampl / pos;
     }
     else
     {
         del->plk_rising_step = ampl;
     }
     pos = L - pos;
     if (pos > 0.0f)
     {
         del->plk_falling_step = ampl / pos;
     }
     else
     {
         del->plk_falling_step = ampl;
     }
     del->plk_amplitude = ampl;
     del->plk_state = e_pluck_rising;
}
