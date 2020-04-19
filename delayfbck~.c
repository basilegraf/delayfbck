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

#define MAX_NUM_FILTERS 5

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

  // Filter array
  t_filter filters[MAX_NUM_FILTERS];
  
  // Envelope detection filter
  t_filter envelopeFilt;

  t_float sampleTime; // elsewhere??

  t_float delDuration;

  t_delay del;
  t_nonlin nl;

  t_inlet *x_in2;
  t_outlet* x_out1;
  t_outlet* x_out2;
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
  t_sample  *in2 =    (t_sample *)(w[3]);
  /* here comes the signalblock that will hold the output signal */
  t_sample  *out1 =   (t_sample *)(w[4]);
  t_sample  *out2 =   (t_sample *)(w[5]);
  /* all signalblocks are of the same length */
  int          n =           (int)(w[6]);
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
      // Modulate delay duration with second input
      delay_set_duration(&x->del, x->delDuration * (1.0 + in2[i]), x->sampleTime); 

      // delay line output
      delay_read(&x->del, &yDel);

      // Nonlinearity
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

  /* save the mixing factor in our dataspace */
  x->f_delayfbck = f;
  
  /* create a new signal-inlet */
  x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  /* create a new passive inlet for the mixing-factor */
  //x->x_in2 = floatinlet_new (&x->x_obj, &x->f_delayfbck);

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
    filter_gain(&x->filters[k], 1.0);
  }
  
  // Init envelope filter
  filter_lp1(&x->envelopeFilt, 10.0, 1.0/44100.0);

  // Init delay line
  delay_init(&x->del, 44100);
  x->delDuration = 1.0/100.0;
  delay_set_duration(&x->del, 1.0/100.0,  1.0/44100.0);

  // Init the nonlinearity
  nonlin_init(&x->nl);
  nonlin_set(&x->nl, e_symmetric_sat, 1.0, 1.0);
  nonlin_print(&x->nl);
  return (void *)x;
}

// Set filter
// Message format:
// "filter filtNum filtType floatArg1 , ..., floatArgn"
//         A_FLOAT A_SYMBOL A_SYMBOL         A_SYMBOL
//         int     symbol   float            float
// Example:
// "filter 0       lp2      150.0    0.7"
void set_filter(t_delayfbck_tilde* x, t_symbol *s, int argc, t_atom *argv)
{
// TODO ! But the method is called properly now !
  post("delayfbck: filter with %d arguments", argc);
  for (int k=0; k<argc; k++)
  {
      post("delayfbck filter: argc %d type = %d", k, argv[k].a_type);
  }
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
  t_symbol* filtType = atom_getsymbolarg(1, argc, argv);
  t_float filtarg1 = atom_getfloatarg(2, argc, argv);
  t_float filtarg2 = atom_getfloatarg(3, argc, argv);

  if (filtNum < 0 || filtNum >= MAX_NUM_FILTERS)
  {
    error("delayfbck filter: Filter number out of range 0...%d.", MAX_NUM_FILTERS-1);
  }
  
  if (filtType == x->sym_g)
  {
    post("delayfbck: g");
    filter_gain(&x->filters[filtNum], filtarg1);
  }
  else if (filtType == x->sym_lp1)
  {
    post("delayfbck: lp1");
    filter_lp1(&x->filters[filtNum], filtarg1, x->sampleTime);
  }
  else if (filtType == x->sym_hp1)
  {
    post("delayfbck: hp1");
    filter_hp1(&x->filters[filtNum], filtarg1, x->sampleTime);
  }
  else if (filtType == x->sym_lp2)
  {
    post("delayfbck: lp2");
    filter_lp2(&x->filters[filtNum], filtarg1, filtarg2, x->sampleTime);
  }
  else if (filtType == x->sym_hp2)
  {
    post("delayfbck: hp2");
    filter_hp2(&x->filters[filtNum], filtarg1, filtarg2, x->sampleTime);
  }
  else if (filtType == x->sym_n)
  {
    post("delayfbck: n");
    t_float filtarg3 = atom_getfloatarg(4, argc, argv);
    filter_n(&x->filters[filtNum], filtarg1, filtarg2, filtarg3, x->sampleTime);
  }
}



void set_nonlinearity(t_delayfbck_tilde* x, t_symbol *s, int argc, t_atom *argv)
{
  post("delayfbck: nonlin with %d arguments", argc);
  for (int k=0; k<argc; k++)

  if (argc != 3)
  {
    error("delayfbck nonlin: 3 arguments expected");
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

  t_symbol* nonlinType = atom_getsymbolarg(0, argc, argv);
  t_float gain = atom_getfloatarg(1, argc, argv);
  t_float sat  = atom_getfloatarg(2, argc, argv);

  if      (nonlinType == x->sym_symmetric_sat)      {if (nonlin_set(&x->nl, e_symmetric_sat,      gain, sat)) return;}
  else if (nonlinType == x->sym_asymmetric_sat)     {if (nonlin_set(&x->nl, e_asymmetric_sat,     gain, sat)) return;}
  else if (nonlinType == x->sym_symmetric_sigmoid)  {if (nonlin_set(&x->nl, e_symmetric_sigmoid,  gain, sat)) return;}
  else if (nonlinType == x->sym_asymmetric_sigmoid) {if (nonlin_set(&x->nl, e_asymmetric_sigmoid, gain, sat)) return;}
  else {error("Unknown nonlin type");}
 
  nonlin_print(&x->nl);
}


void set_delay(t_delayfbck_tilde* x, t_floatarg duration)
{
  post("delayfbck: set delay to %fs", duration);
  x->delDuration = duration;
  delay_set_duration(&x->del, duration, x->sampleTime); 
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
