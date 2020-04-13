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
#include "m_pd.h"

#include "filter.h"
#include "delay.h"
#include "nonlin.h"

/**
 * define a new "class" 
 */
static t_class *delayfbck_tilde_class;


/**
 * this is the dataspace of our new object
 * the first element is the mandatory "t_object"
 * f_delayfbck denotes the mixing-factor
 * "f" is a dummy and is used to be able to send floats AS signals.
 */
 typedef struct _delayfbck_tilde {
  t_object  x_obj;
  t_sample f_delayfbck;
  t_sample f;

  t_filter filt;
  t_filter filthp;
  t_delay del;
  t_nonlin nl;

  t_inlet *x_in2;
  t_outlet*x_out;
} t_delayfbck_tilde;


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
  /* here comes the signalblock that will hold the output signal */
  t_sample  *out =    (t_sample *)(w[3]);
  /* all signalblocks are of the same length */
  int          n =           (int)(w[4]);
  /* get (and clip) the mixing-factor */
  t_sample f_delayfbck = (x->f_delayfbck<0)?0.0:(x->f_delayfbck>1)?1.0:x->f_delayfbck;
  /* just a counter */
  int i;

  /* this is the main routine: 
   * mix the 2 input signals into the output signal
   */
  t_float yDel;
  for(i=0; i<n; i++)
    {
      // delay line output
      delay_read(&x->del, &yDel);

      // Nonlinearity
      nonlin_step(&x->nl, in1[i] + yDel, &out[i]);

      //Filter
      filter_step(&x->filt, out[i], &out[i]);
      filter_step(&x->filthp, out[i], &out[i]);

      // Delay line feedback     
      delay_write(&x->del, out[i]);
      delay_step(&x->del);    
    }

  /* return a pointer to the dataspace for the next dsp-object */
  return (w+5);
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
   * 1 input signal and 1 output signal) and the length of the
   * signal vectors (all vectors are of the same length)
   */
  dsp_add(delayfbck_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
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
  outlet_free(x->x_out);

  // Free the filter
  filter_free(&x->filt);
  filter_free(&x->filthp);

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

  /* save the mixing factor in our dataspace */
  x->f_delayfbck = f;
  
  /* create a new signal-inlet */
  //x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  /* create a new passive inlet for the mixing-factor */
  x->x_in2 = floatinlet_new (&x->x_obj, &x->f_delayfbck);

  /* create a new signal-outlet */
  x->x_out = outlet_new(&x->x_obj, &s_signal);

  // Create filters
  filter_lp1(&x->filt, 500.0, 1.0/44100.0);
  filter_hp1(&x->filthp, 10.0, 1.0/44100.0);

  // Init delay line
  delay_init(&x->del, 44100);
  delay_set_duration(&x->del, 500.0/44100.0,  1.0/44100.0);

  // Init the nonlinearity
  nonlin_init(&x->nl);
  nonlin_set(&x->nl, e_asymmetric_sat, 1.11, 1.0);
  nonlin_print(&x->nl);
  return (void *)x;
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
