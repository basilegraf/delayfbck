/*
 * HOWTO write an External for Pure data
 * (c) 2001-2006 IOhannes m zmölnig zmoelnig[AT]iem.at
 *
 * this is the source-code for the fourth example in the HOWTO
 * it creates a simple dsp-object:
 * 2 input signals are mixed into 1 output signal
 * the mixing-factor can be set via the 3rd inlet
 *
 * for legal issues please see the file LICENSE.txt
 */

// https://github.com/pure-data/externals-howto

/**
 * include the interface to Pd 
 */
 
#include <math.h>
#include "m_pd.h"

#include "filter.h"
#include "delay.h"
#include "nonlin.h"
#include "picont.h"
#include "cubic.h"

#define MAX_NUM_FILTERS 5


/**
 * define a new "class" 
 */
static t_class *delayfbck_tilde_class;


/**
 * this is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
 typedef struct _delayfbck_tilde {
  t_object  x_obj;
  t_sample f;

  // Filter types symbols
  t_symbol* sym_g;
  t_symbol* sym_lp1;
  t_symbol* sym_hp1;
  t_symbol* sym_lp2;
  t_symbol* sym_hp2;
  t_symbol* sym_n;

  // Create nonlineaity type symbols
  t_symbol* sym_symmetric_sat;
  t_symbol* sym_asymmetric_sat;
  t_symbol* sym_symmetric_sigmoid;
  t_symbol* sym_asymmetric_sigmoid;
  
  // Create symbol for plucked string
  t_symbol* sym_pluck;

  // Filter array
  t_filter filters[MAX_NUM_FILTERS];
  t_int use_pitch_correction;
  t_int use_gain_correction;
  
  // Envelope detection filter
  t_filter envelopeFilt;
  
  // Amplitude PI controller (as filter)
  t_picont pic;
  t_float amplitudeRef;
  t_float picOut;

  t_float sampleTime; // elsewhere??

  t_float delDurationDesired;

  t_delay del;
  t_nonlin nl;
  t_float nl_gain_base_value;
  t_float nl_gain_correction;

  t_inlet *x_in2;
  t_outlet* x_out1;
  t_outlet* x_out2;
} t_delayfbck_tilde;

// Function declarations
void set_delay(t_delayfbck_tilde* x, t_floatarg duration, t_floatarg delRampTime);


/**
 * this is the core of the object
 * this perform-routine is called for each signal block
 * the name of this function is arbitrary and is registered to Pd in the 
 * delayfbck_tilde_dsp() function, each time the DSP is turned on
 *
 * the argument to this function is just a pointer within an array
 * we have to know for ourselves how many elements inthis array are
 * reserved for us (hint: we declare the number of used elements in the
 * delayfbck_tilde_dsp() at registration
 *
 * since all elements are of type "t_int" we have to cast them to whatever
 * we think is apropriate; "apropriate" is how we registered this function
 * in delayfbck_tilde_dsp()
 */
t_int *delayfbck_tilde_perform(t_int *w)
{
  /* the first element is a pointer to the dataspace of this object */
  t_delayfbck_tilde *x = (t_delayfbck_tilde *)(w[1]);
  /* here is a pointer to the t_sample arrays that hold the 2 input signals */
  t_sample  *in1 =    (t_sample *)(w[2]);
  t_sample  *in2 =    (t_sample *)(w[3]);
  /* here comes the signalblock that will hold the output signal */
  t_sample  *out1 =   (t_sample *)(w[4]);
  t_sample  *out2 =   (t_sample *)(w[5]);
  /* all signalblocks are of the same length */
  int          n =           (int)(w[6]);

  /* just a counter */
  int i;

  /* this is the main routine: 
   * mix the 2 input signals into the output signal
   */
  t_float yDel;
  for(i=0; i<n; i++)
    {      
      // Modulate delay duration with second input
      (x->del).delay_modulation = in2[i];

      // Delay line output
      delay_read(&x->del, &yDel);

      // Apply PI controller to gain modulation
      x->nl.gain_modulation = x->picOut;
      nonlin_step(&x->nl, in1[i] + yDel, &out1[i]);

      //Filter
      for (int k=0; k<MAX_NUM_FILTERS; k++)
      {
        filter_step(&x->filters[k], out1[i], &out1[i]);
      }

      // Delay line feedback     
      delay_write(&x->del, out1[i]);
      delay_step(&x->del);    
      
      // second output: Envelope filter
      out2[i] = fabsf(out1[i]);
      filter_step(&x->envelopeFilt, out2[i], &out2[i]);
      
      // PI amplitude controller
      picont_step(&x->pic, x->amplitudeRef - out2[i], &x->picOut, x->sampleTime);
    }

  /* return a pointer to the dataspace for the next dsp-object */
  return (w+7);
}


/**
 * register a special perform-routine at the dsp-engine
 * this function gets called whenever the DSP is turned ON
 * the name of this function is registered in delayfbck_tilde_setup()
 */
void delayfbck_tilde_dsp(t_delayfbck_tilde *x, t_signal **sp)
{
  /* add delayfbck_tilde_perform() to the DSP-tree;
   * the delayfbck_tilde_perform() will expect "4" arguments (packed into an
   * t_int-array), which are:
   * the objects data-space, 3 signal vectors (which happen to be
   * 2 input signal and 2 output signals) and the length of the
   * signal vectors (all vectors are of the same length)
   */
  dsp_add(delayfbck_tilde_perform, 6, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

/**
 * this is the "destructor" of the class;
 * it allows us to free dynamically allocated ressources
 */
void delayfbck_tilde_free(t_delayfbck_tilde *x)
{
  /* free any ressources associated with the given inlet */
  inlet_free(x->x_in2);

  /* free any ressources associated with the given outlet */
  outlet_free(x->x_out1);
  outlet_free(x->x_out2);

  // Free the filter
  for (int k=0; k<MAX_NUM_FILTERS; k++)
  {
     filter_free(&x->filters[k]);
  }

  // Free the delay
  delay_free(&x->del);

  // Free the nonlinearity
  nonlin_free(&x->nl);
}

/**
 * this is the "constructor" of the class
 * the argument is the initial mixing-factor
 */
void *delayfbck_tilde_new(t_floatarg f)
{
  t_delayfbck_tilde *x = (t_delayfbck_tilde *)pd_new(delayfbck_tilde_class);

  x->f = f; // TODO remove
  
  /* create a new signal-inlet */
  x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  /* create a new signal-outlet */
  x->x_out1 = outlet_new(&x->x_obj, &s_signal);
  x->x_out2 = outlet_new(&x->x_obj, &s_signal);

  // Create filter type symbols
  x->sym_g   = gensym("g");
  x->sym_lp1 = gensym("lp1");
  x->sym_hp1 = gensym("hp1");
  x->sym_lp2 = gensym("lp2");
  x->sym_hp2 = gensym("hp2");
  x->sym_n = gensym("n");

  // Create nonlineaity type symbols
  x->sym_symmetric_sat = gensym("symmetric_sat");
  x->sym_asymmetric_sat = gensym("asymmetric_sat");
  x->sym_symmetric_sigmoid = gensym("symmetric_sigmoid");
  x->sym_asymmetric_sigmoid = gensym("asymmetric_sigmoid");
  
  x->sampleTime = 1.0 / 44100.0; // TODO

  // Initialise filters to unit gain
  for (int k=0; k<MAX_NUM_FILTERS; k++) 
  {
    filter_init(&x->filters[k]);
    filter_gain(&x->filters[k], 1.0);
  }
  
  // Init envelope filter
  filter_lp1(&x->envelopeFilt, 10.0, x->sampleTime);
  
  x->use_pitch_correction = 1;
  x->use_gain_correction = 0;
  
  // Init PI amplitude controller with zero gains and saturation 1.0
  picont_init(&x->pic, 0.0, 0.0, 1.0);
  x->amplitudeRef = 0.0;
  x->picOut = 0.0;

  // Init delay line
  delay_init(&x->del, x->sampleTime, 44100);
  x->delDurationDesired = 1.0/100.0;
  delay_set_duration(&x->del, x->delDurationDesired,  x->sampleTime);


  // Init the nonlinearity
  nonlin_init(&x->nl, x->sampleTime);
  x->nl_gain_base_value = 0.0;
  x->nl_gain_correction = 1.0;
  nonlin_set(&x->nl, e_symmetric_sat, x->nl_gain_base_value, 1.0f, 0.0f);
  nonlin_print(&x->nl);
  
  return (void *)x;
}

// Set filter
// Message format:
// "filter filtNum filtType floatArg1 , ..., floatArgn"
//         A_FLOAT A_SYMBOL A_FLOAT          A_FLOAT
//         int     symbol   float            float
// Example:
// "filter 0       lp2      150.0    0.7  0.1"
// Last optional argument is the ramp time
void set_filter(t_delayfbck_tilde* x, t_symbol *s, int argc, t_atom *argv)
{
  //post("delayfbck: filter with %d arguments", argc);
  //for (int k=0; k<argc; k++)
  //{
  //    post("delayfbck filter: argc %d type = %d", k, argv[k].a_type);
  //}
  if (argc < 3)
  {
    error("delayfbck filter: Too few arguments.");
    return;
  }
  else if (argv[0].a_type != A_FLOAT)
  {
    error("delayfbck filter: Arguments 1 should be an integer");
    return;
  }
  else if (argv[1].a_type != A_SYMBOL)
  {
    error("delayfbck filter: Arguments 2 should be a symbol");
    return;
  }
  else
  {
    int allFloats = 1;
    for (int k=2; k<argc; k++) allFloats &= (argv[k].a_type == A_FLOAT);
    if (!allFloats)
    {
      error("delayfbck filter: Arguments types should be: integer, symbol, floats, <float>, ... , <float>.");
      return;
    }
  }

  t_int filtNum = atom_getintarg(0, argc, argv);
  t_symbol* filtTypeSym = atom_getsymbolarg(1, argc, argv);
  //t_float filtarg1 = atom_getfloatarg(2, argc, argv);
  //t_float filtarg2 = atom_getfloatarg(3, argc, argv);
  enum e_filter_type prevType = x->filters[filtNum].type;
  
  
  t_float filtRampTime = 0.05; // TODO: make as argument somehow?
  t_float filtargs[MAX_FILTER_NUM_PARAM + 1];  // last arg : ramp time
  for (t_int k=0; k<MAX_FILTER_NUM_PARAM + 1; k++)
  {
      filtargs[k] = atom_getfloatarg(k + 2, argc, argv);
  }

  if ((filtNum < 0) || (filtNum >= MAX_NUM_FILTERS))
  {
    error("delayfbck filter: Filter number out of range 0...%d.", MAX_NUM_FILTERS-1);
  }
  
  // Block any filter ramping currently in progress (will freeze b,a to current value)
  t_int n_param_steps = 0;
  x->filters[filtNum].n_param_steps = 0;
  
  if (filtTypeSym == x->sym_g)
  {
    //post("delayfbck: g");
    x->filters[filtNum].type = e_filter_gain;
    x->filters[filtNum].param_ramptype[0] = e_ramp_lin;
    filtRampTime = filtargs[1];
  }
  else if (filtTypeSym == x->sym_lp1)
  {
    //post("delayfbck: lp1");
    x->filters[filtNum].type = e_filter_lp1;
    x->filters[filtNum].param_ramptype[0] = e_ramp_exp;
    filtRampTime = filtargs[1];
  }
  else if (filtTypeSym == x->sym_hp1)
  {
    //post("delayfbck: hp1");
    x->filters[filtNum].type = e_filter_hp1;
    x->filters[filtNum].param_ramptype[0] = e_ramp_exp; // gain
    filtRampTime = filtargs[1];
  }
  else if (filtTypeSym == x->sym_lp2)
  {
    //post("delayfbck: lp2");
    x->filters[filtNum].type = e_filter_lp2;
    x->filters[filtNum].param_ramptype[0] = e_ramp_exp; // freqency
    x->filters[filtNum].param_ramptype[1] = e_ramp_lin; // damping zeta
    filtRampTime = filtargs[2];
  }
  else if (filtTypeSym == x->sym_hp2)
  {
    //post("delayfbck: hp2");
    x->filters[filtNum].type = e_filter_hp2;
    x->filters[filtNum].param_ramptype[0] = e_ramp_exp; // freqency
    x->filters[filtNum].param_ramptype[1] = e_ramp_lin; // damping zeta
    filtRampTime = filtargs[2];
  }
  else if (filtTypeSym == x->sym_n)
  {
    //post("delayfbck: n");
    x->filters[filtNum].type = e_filter_n;
    x->filters[filtNum].param_ramptype[0] = e_ramp_exp; // freqency
    x->filters[filtNum].param_ramptype[1] = e_ramp_lin; // gain
    x->filters[filtNum].param_ramptype[2] = e_ramp_lin; // bandwidth
    filtRampTime = filtargs[3];
  }
  
  // Ramping steps
  if (prevType != x->filters[filtNum].type)
  {
      // Filter type change, we cannot ramp parameters. Set in one step
      n_param_steps = 1;  
  }
  else
  {
      n_param_steps = (t_int) roundf(filtRampTime / x->sampleTime);
      n_param_steps = n_param_steps >= 1 ? n_param_steps : 1;
  }

  
  for (t_int k=0; k<MAX_FILTER_NUM_PARAM; k++)
  {
      x->filters[filtNum].param_target[k] = filtargs[k];
      switch (x->filters[filtNum].param_ramptype[k])
      {
          case e_ramp_lin:
            x->filters[filtNum].param_step[k] =  (filtargs[k] - x->filters[filtNum].param[k]) / ((t_float) n_param_steps);
            break;
          
          case e_ramp_exp:
            x->filters[filtNum].param_step[k] =  powf(filtargs[k] / x->filters[filtNum].param[k] , 1.0 / ((t_float) n_param_steps));
            break;
          
          default:
            error("Undefined ramp type.");
      }
      
  }
  
  // Set target coefficients (i.e. filter coeffs after ramping. To be used for delay length compensation)
  filter_x(&x->filters[filtNum],  e_set_filter_coeffs_target);
  
  x->filters[filtNum].h = x->sampleTime; // TODO ??
  
  // Finally, allow filter ramping again by setting x->filters[filtNum].n_param_steps
  x->filters[filtNum].n_param_steps = n_param_steps;
  
  // Set the delay duration to the desired one ==> apply duration correction based on new filters
  set_delay(x, x->delDurationDesired, filtRampTime);
}



void set_nonlinearity(t_delayfbck_tilde* x, t_symbol *s, int argc, t_atom *argv)
{
  post("delayfbck: nonlin with %f arguments", (t_float) argc);
  for (int k=0; k<argc; k++)

  if ((argc != 3) && (argc != 4))
  {
    error("delayfbck nonlin: 3 or 4 arguments expected");
    return;
  }
  else if (argv[0].a_type != A_SYMBOL)
  {
    error("delayfbck nonlin: Arguments 1 should be a symbol");
    return;
  }
  else if (argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT)
  {
    error("delayfbck nonlin: Arguments 2 and 3 should be floats");
    return;
  }
  if (argc == 4 && argv[3].a_type != A_FLOAT)
  {
    error("delayfbck nonlin: Arguments 4 should be a float");
    return;
  }

  t_symbol* nonlinType = atom_getsymbolarg(0, argc, argv);
  t_float gain = atom_getfloatarg(1, argc, argv);
  t_float sat  = atom_getfloatarg(2, argc, argv);
  t_float ramp_time  = atom_getfloatarg(3, argc, argv);
  
  x->nl_gain_base_value = gain;
  gain *= x->nl_gain_correction;
  
  if      (nonlinType == x->sym_symmetric_sat)      {nonlin_set(&x->nl, e_symmetric_sat,      gain, sat, ramp_time);}
  else if (nonlinType == x->sym_asymmetric_sat)     {nonlin_set(&x->nl, e_asymmetric_sat,     gain, sat, ramp_time);}
  else if (nonlinType == x->sym_symmetric_sigmoid)  {nonlin_set(&x->nl, e_symmetric_sigmoid,  gain, sat, ramp_time);}
  else if (nonlinType == x->sym_asymmetric_sigmoid) {nonlin_set(&x->nl, e_asymmetric_sigmoid, gain, sat, ramp_time);}
  else {error("Unknown nonlin type");}
 
  nonlin_print(&x->nl);
}


void set_delay(t_delayfbck_tilde* x, t_floatarg duration, t_floatarg delRampTime)
{
    // Store the uncompensated desired delay
    x->delDurationDesired = duration;
    
    // No amplitude corraection by default
    x->nl_gain_correction = 1.0f;
    
    post("PitchCorrect = %f, amlpCorrect = %f", (t_float) x->use_pitch_correction, (t_float) x->use_gain_correction);
    
    // If desired, take phase of filters and nonlin gain at frequency 1/duration into account 
    if ((x->use_pitch_correction > 0) || (x->use_gain_correction > 0))
    {
        // Compute phase of all filters
        t_float mag = 1.0;
        t_float phase =0.0;
        t_float fNorm = x->sampleTime / duration;
        t_float magk, phasek;
        for (t_int k=0; k<MAX_NUM_FILTERS; k++)
        {
            filter_bode(&x->filters[k],  fNorm, e_set_filter_coeffs_target, &magk, &phasek); // Use phase of target filter (after any currently ongoing filter ramp)
            mag *= magk;
            phase += phasek;
        }
        post("-");
        // Add phase of non-linearity
        if (x->nl.gain < 0.0)
        {
            phase += PI;
        }
        if (phase > PI)
        {
            phase -= PI;
        } 
        post("Filters phase = %g, mag=%g", 180.0 * phase / PI, mag);
        
        if (x->use_pitch_correction > 0) 
        {
            duration += phase * duration / TWOPI;
        }
        
        if ((x->use_gain_correction > 0) && (mag != 0.0f))
        {
            x->nl_gain_correction = fminf(1.0f / mag, 10.0f);
        }
    }

    // Update target gain
    t_float gain = x->nl_gain_base_value * x->nl_gain_correction;
    nonlin_set(&x->nl, x->nl.type, gain, x->nl.saturation, delRampTime);
    // Update target delay line duration
    delay_set_duration(&x->del, duration, delRampTime);
}


void set_amplitude_control(t_delayfbck_tilde* x, t_floatarg lpfreq, t_floatarg amplRef, t_floatarg Pgain, t_floatarg Igain, t_floatarg sat)
{
    post("delayfbck: set amplitude controller, lpFreq %2.1f, amplitude %1.2f, P %f, I %g, Sat %1.2f", lpfreq, amplRef, Pgain, Igain, sat);
    filter_lp1(&x->envelopeFilt, lpfreq, x->sampleTime);
    picont_init(&x->pic, Pgain, Igain, sat);
    x->amplitudeRef = amplRef;
}


void set_pluck_string(t_delayfbck_tilde* x, t_floatarg ampl, t_floatarg pos)
{
    post("delayfbck: pluck string, amplitude %2.1f, position %2.1f", ampl, pos);
    delay_pluck_string(&x->del, ampl, pos);
}


void set_pitch_correction(t_delayfbck_tilde* x, t_floatarg pitch_correct)
{
    t_float ramp_time = 0.01f; // Should be fine
    x->use_pitch_correction = pitch_correct > 0.5f ? 1 : 0;
    // Set the delay duration to the desired one ==> apply correction based on new choice
    set_delay(x, x->delDurationDesired, ramp_time);
    post("Use pitch correction: %f", (t_float) pitch_correct);
}

void set_gain_correction(t_delayfbck_tilde* x, t_floatarg gain_correct)
{
    t_float ramp_time = 0.01f; // Should be fine
    x->use_gain_correction = gain_correct > 0.5f ? 1 : 0;
    // Set the delay duration to the desired one ==> apply correction based on new choice
    set_delay(x, x->delDurationDesired, ramp_time);
    post("Use gain correction: %f", (t_float) gain_correct);
}

void set_linear_interp(t_delayfbck_tilde* x)
{
    x->del.interp_type = e_interp_linear;
    post("Use linear interpolation");
}

void set_cubic_interp(t_delayfbck_tilde* x)
{
    x->del.interp_type = e_interp_cubic;
    post("Use cubic interpolation");
}


/**
 * define the function-space of the class
 * within a single-object external the name of this function is very special
 */
void delayfbck_tilde_setup(void) {

  delayfbck_tilde_class = class_new(gensym("delayfbck~"),
        (t_newmethod)delayfbck_tilde_new,
        (t_method)delayfbck_tilde_free,
	sizeof(t_delayfbck_tilde),
        CLASS_DEFAULT, 
        A_DEFFLOAT, 0);

  class_addmethod(delayfbck_tilde_class,
      (t_method)set_filter, gensym("filter"),
      A_GIMME, 0);

  class_addmethod(delayfbck_tilde_class,
      (t_method)set_nonlinearity, gensym("nonlin"),
      A_GIMME, 0);

  class_addmethod(delayfbck_tilde_class,
        (t_method)set_delay, gensym("delay"),
        A_DEFFLOAT, A_DEFFLOAT, 0);
        
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_amplitude_control, gensym("ampctrl"),
        A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
        
    
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_pluck_string, gensym("pluck"),
        A_DEFFLOAT, A_DEFFLOAT, 0);
        
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_pitch_correction, gensym("pitchcorr"),
        A_DEFFLOAT, 0);
        
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_gain_correction, gensym("gaincorr"),
        A_DEFFLOAT, 0);
        
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_linear_interp, gensym("linear"),
        0);
        
  class_addmethod(delayfbck_tilde_class,
        (t_method)set_cubic_interp, gensym("cubic"),
        0);  
        


  /* whenever the audio-engine is turned on, the "delayfbck_tilde_dsp()" 
   * function will get called
   */
  class_addmethod(delayfbck_tilde_class,
        (t_method)delayfbck_tilde_dsp, gensym("dsp"), 0);
  /* if no signal is connected to the first inlet, we can as well 
   * connect a number box to it and use it as "signal"
   */
  CLASS_MAINSIGNALIN(delayfbck_tilde_class, t_delayfbck_tilde, f);
}
