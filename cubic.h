/* 
Cubic interpolation
basile dot graf at a3 dot epfl dot ch
*/


#ifndef _CUBIC_H_

#include "m_pd.h"

/* 4-point cubic interpolation of data defined by
 * 
 *  x data :       -1.0        0.0        1.0         2.0
 *  y data :    {y_data[0], y_data[1], y_data[2], y_data[3]}
 * 
 * Returns evaluation at x = xEval of the 3rd order polynomial passing through data points.
 * It is recommended to evaluate the polynomial at values between 0.0 and 1.0
 * 
 */
t_float cubic(t_float* y_data, t_float x_eval);


#endif /* _CUBIC_H_ */
