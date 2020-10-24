/* 
Cubic interpolation
basile dot graf at a3 dot epfl dot ch
*/

#include "cubic.h"
#include <stdio.h>      
#include <stdlib.h>  
#include <math.h>

/* The colums of matrix A contain the [-1 0 1 2]' to the power 0, 1, 2 & 3
 * 
 *      1   -1    1   -1
 *      1    0    0    0
 *      1    1    1    1
 *      1    2    4    8
 * 
 * The inverse of the matrix is
 * 
 *      0       1       0       0
 *     -1/3    -1/2     1      -1/6
 *      1/2    -1       1/2     0
 *     -1/6     1/2    -1/2     1/6
 * 
 * Define x_vector = [1 x_eval x_eval^2 x_eval^3]
 * Then the interpolated value is given by
 * 
 * y_eval = x_vector' * inverse(A) * y_data
 * 
 * 
 */


t_float cubic(t_float* y_data, t_float x_eval)
{
    t_float x_eval2 = x_eval * x_eval;
    t_float x_eval3 = x_eval2 * x_eval;
    t_float y_eval = 0.0;
    y_eval += (        (-1.0/3.0) * x_eval + (1.0/2.0) * x_eval2 + (-1.0/6.0) * x_eval3) * y_data[0];
    y_eval += (1.0  +  (-1.0/2.0) * x_eval -             x_eval2 + (1.0/2.0)  * x_eval3) * y_data[1];
    y_eval += (                     x_eval + (1.0/2.0) * x_eval2 + (-1.0/2.0) * x_eval3) * y_data[2];
    y_eval += (        (-1.0/6.0) * x_eval                       + (1.0/6.0)  * x_eval3) * y_data[3];
    return y_eval;
}


